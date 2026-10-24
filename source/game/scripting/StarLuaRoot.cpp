import star.json_extra;
import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.output_proxy;
import star.listener;
import std;
import star.time;
import star.string;
import star.file;
import star.lua;
import star.format;
import star.version;
import star.casting;

import star.lua_root;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

namespace Star {

std::expected<Root*, StarException> luaExecutionRoot(LuaEngine& engine) {
  if (auto scope = as<LuaExecutionScope>(engine.executionHost()))
    return &scope->root();
  return std::unexpected(LuaException("Game Lua conversion requires an explicit game execution scope"));
}

LuaRoot::LuaRoot(Root& root)
  : m_root(root), m_scriptCache(std::make_shared<ScriptCache>(root)),
    m_storageDirectory(root.toStoragePath("lua")) {}

std::expected<std::shared_ptr<LuaRoot>, StarException> LuaRoot::create(Root& root) {
  auto luaRoot = std::shared_ptr<LuaRoot>(new LuaRoot(root));
  if (auto result = luaRoot->restart(); !result)
    return std::unexpected(std::move(result.error()));
  luaRoot->m_rootReloadListener = std::make_shared<CallbackListener>([cache = luaRoot->m_scriptCache]() -> std::expected<void, StarException> {
    cache->clear();
    return {};
  });
  luaRoot->m_rootReloadSubscription = root.registerReloadListener(luaRoot->m_rootReloadListener);
  return luaRoot;
}

LuaRoot::~LuaRoot() {
  if (auto result = shutdown(); !result) {
    if (auto logged = m_root.logger().error("Error while shutting down Lua root: {}", outputException(result.error(), true)); !logged)
      EmergencyLog::error("Could not log Lua root shutdown failure: {}; original failure: {}", outputException(logged.error(), true), outputException(result.error(), true));
  }
}

Root& LuaRoot::root() const {
  return m_root;
}

std::expected<void, StarException> LuaRoot::loadScript(String const& assetPath) {
  return m_scriptCache->loadScript(*m_luaEngine, assetPath);
}

bool LuaRoot::scriptLoaded(String const& assetPath) const {
  return m_scriptCache->scriptLoaded(assetPath);
}

void LuaRoot::unloadScript(String const& assetPath) {
  m_scriptCache->unloadScript(assetPath);
}

std::expected<void, StarException> LuaRoot::restart() {
  if (auto result = shutdown(); !result)
    return std::unexpected(std::move(result.error()));

  auto configuration = m_root.configuration();
  if (!configuration)
    return std::unexpected(std::move(configuration.error()));
  auto safeScripts = (*configuration)->get("safeScripts");
  if (!safeScripts)
    return std::unexpected(std::move(safeScripts.error()));
  auto safe = safeScripts->toBool();
  if (!safe)
    return std::unexpected(std::move(safe.error()));
  auto engine = LuaEngine::create(*safe);
  if (!engine)
    return std::unexpected(std::move(engine.error()));
  (*engine)->setExecutionHost(std::make_unique<LuaExecutionScope>(m_root));
  (*engine)->setImageResolver([root = &m_root](String const& path) -> std::expected<std::shared_ptr<Image const>, StarException> {
    auto assets = root->assets();
    if (!assets)
      return std::unexpected(std::move(assets.error()));
    return (*assets)->image(path);
  });

  auto recursion = (*configuration)->get("scriptRecursionLimit");
  if (!recursion)
    return std::unexpected(std::move(recursion.error()));
  auto recursionLimit = recursion->toUInt();
  if (!recursionLimit)
    return std::unexpected(std::move(recursionLimit.error()));
  auto instruction = (*configuration)->get("scriptInstructionLimit");
  if (!instruction)
    return std::unexpected(std::move(instruction.error()));
  auto instructionLimit = instruction->toUInt();
  if (!instructionLimit)
    return std::unexpected(std::move(instructionLimit.error()));
  auto profiling = (*configuration)->get("scriptProfilingEnabled");
  if (!profiling)
    return std::unexpected(std::move(profiling.error()));
  auto profilingEnabled = profiling->toBool();
  if (!profilingEnabled)
    return std::unexpected(std::move(profilingEnabled.error()));
  auto interval = (*configuration)->get("scriptInstructionMeasureInterval");
  if (!interval)
    return std::unexpected(std::move(interval.error()));
  auto measureInterval = interval->toUInt();
  if (!measureInterval)
    return std::unexpected(std::move(measureInterval.error()));
  (*engine)->setRecursionLimit(*recursionLimit);
  (*engine)->setInstructionLimit(*instructionLimit);
  (*engine)->setProfilingEnabled(*profilingEnabled);
  (*engine)->setInstructionMeasureInterval(*measureInterval);
  m_luaEngine = std::move(*engine);
  return {};
}

std::expected<void, StarException> LuaRoot::shutdown() {
  clearScriptCache();
  if (!m_luaEngine)
    return {};

  // Detach even when writing the profile fails; no half-shutdown engine remains.
  auto engine = std::move(m_luaEngine);
  auto profile = engine->getProfile();
  if (profile.empty())
    return {};
  profile.sort([](auto const& a, auto const& b) { return a.totalTime > b.totalTime; });
  std::function<Json(LuaProfileEntry const&)> jsonFromProfileEntry = [&](LuaProfileEntry const& entry) -> Json {
    JsonObject value;
    value.set("function", entry.name.value_or("<function>"));
    value.set("scope", entry.nameScope.value_or("?"));
    value.set("source", strf("{}:{}", entry.source, entry.sourceLine));
    value.set("self", entry.selfTime);
    value.set("total", entry.totalTime);
    List<LuaProfileEntry> calls;
    for (auto const& p : entry.calls)
      calls.append(*p.second);
    value.set("calls", calls.sorted([](auto const& a, auto const& b) { return a.totalTime > b.totalTime; }).transformed(jsonFromProfileEntry));
    return value;
  };
  auto summary = Json(profile.transformed(jsonFromProfileEntry)).repr(1);
  if (!summary)
    return std::unexpected(std::move(summary.error()));
  auto directory = File::isDirectory(m_storageDirectory);
  if (!directory)
    return std::unexpected(std::move(directory.error()));
  if (!*directory) {
    if (auto logged = m_root.logger().info("Creating lua storage directory"); !logged)
      return std::unexpected(std::move(logged.error()));
    if (auto result = File::makeDirectory(m_storageDirectory); !result)
      return std::unexpected(std::move(result.error()));
  }
  auto timestamp = Time::printCurrentDateAndTime("<year>-<month>-<day>-<hours>-<minutes>-<seconds>-<millis>");
  String filename = strf("{}.luaprofile", timestamp);
  String path = File::relativeTo(m_storageDirectory, filename);
  if (auto logged = m_root.logger().info("Writing lua profile {}", filename); !logged)
    return std::unexpected(std::move(logged.error()));
  return File::writeFile(*summary, path);
}

std::expected<LuaContext, StarException> LuaRoot::createContext(String const& script) {
  return createContext(StringList{script});
}

std::expected<LuaContext, StarException> LuaRoot::createContext(StringList const& scriptPaths) {
  if (!m_luaEngine)
    return std::unexpected(LuaException("Cannot create a context from a shut down Lua root"));
  auto created = m_luaEngine->createContext();
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto context = std::move(*created);
  auto cache = m_scriptCache;
  if (auto required = context.setRequireFunction([cache](LuaContext& context, LuaString const& module) -> std::expected<void, StarException> {
    auto loaded = context.get("_SBLOADED");
    if (!loaded)
      return std::unexpected(std::move(loaded.error()));
    if (!std::holds_alternative<LuaTable>(*loaded)) {
      auto loadedTable = context.createTable();
      if (!loadedTable)
        return std::unexpected(std::move(loadedTable.error()));
      if (auto result = context.set("_SBLOADED", std::move(*loadedTable)); !result)
        return std::unexpected(std::move(result.error()));
    }
    auto table = context.get<LuaTable>("_SBLOADED");
    if (!table)
      return std::unexpected(std::move(table.error()));
    auto contains = table->contains(module);
    if (!contains)
      return std::unexpected(std::move(contains.error()));
    if (!*contains) {
      if (auto result = table->set(module, true); !result)
        return std::unexpected(std::move(result.error()));
      if (auto result = cache->loadContextScript(context, module.toString()); !result) {
        auto error = std::move(result.error());
        if (auto rollback = table->remove(module); !rollback)
          return std::unexpected(rollback.error().withContext(strf("Could not roll back failed Lua module load; original load failure: {}", outputException(error, true))));
        return std::unexpected(std::move(error));
      }
    }
    return {};
  }); !required)
    return std::unexpected(std::move(required.error()));
  auto assets = m_root.assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  for (auto const& path : scriptPaths) {
    auto exists = (*assets)->assetExists(path);
    if (!exists)
      return std::unexpected(std::move(exists.error()));
    if (*exists) {
      if (auto result = cache->loadContextScript(context, path); !result)
        return std::unexpected(std::move(result.error()));
    } else {
      if (auto logged = m_root.logger().error("Script '{}' does not exist", path); !logged)
        return std::unexpected(std::move(logged.error()));
    }
  }
  for (auto const& callbacks : m_luaCallbacks) {
    if (auto result = context.setCallbacks(callbacks.first, callbacks.second); !result)
      return std::unexpected(std::move(result.error()));
  }
  return context;
}

std::expected<void, StarException> LuaRoot::collectGarbage(std::optional<unsigned> steps) {
  if (m_luaEngine)
    return m_luaEngine->collectGarbage(steps);
  return {};
}

void LuaRoot::setAutoGarbageCollection(bool enabled) {
  if (m_luaEngine)
    m_luaEngine->setAutoGarbageCollection(enabled);
}

void LuaRoot::tuneAutoGarbageCollection(float pause, float multiplier) {
  if (m_luaEngine)
    m_luaEngine->tuneAutoGarbageCollection(pause, multiplier);
}

std::size_t LuaRoot::luaMemoryUsage() const {
  return m_luaEngine ? m_luaEngine->memoryUsage() : 0;
}

std::size_t LuaRoot::scriptCacheMemoryUsage() const {
  return m_luaEngine ? m_scriptCache->memoryUsage() : 0;
}

void LuaRoot::clearScriptCache() const {
  m_scriptCache->clear();
}

void LuaRoot::addCallbacks(String const& groupName, LuaCallbacks const& callbacks) {
  m_luaCallbacks[groupName] = callbacks;
}

LuaEngine& LuaRoot::luaEngine() const {
  return *m_luaEngine;
}

std::expected<void, StarException> LuaRoot::ScriptCache::loadScript(LuaEngine& engine, String const& path) {
  auto assets = m_root.assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  std::unique_lock locker(mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  auto bytes = (*assets)->bytes(path);
  if (!bytes)
    return std::unexpected(std::move(bytes.error()));
  auto compiled = engine.compile(**bytes, path);
  if (!compiled)
    return std::unexpected(std::move(compiled.error()));
  scripts[path] = std::move(*compiled);
  return {};
}

bool LuaRoot::ScriptCache::scriptLoaded(String const& path) const {
  std::unique_lock locker(mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  return scripts.contains(path);
}

void LuaRoot::ScriptCache::unloadScript(String const& path) {
  std::unique_lock locker(mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  scripts.remove(path);
}

void LuaRoot::ScriptCache::clear() {
  std::unique_lock locker(mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  scripts.clear();
}

std::expected<void, StarException> LuaRoot::ScriptCache::loadContextScript(LuaContext& context, String const& path) {
  std::unique_lock locker(mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  if (!scriptLoaded(path)) {
    if (auto result = loadScript(context.engine(), path); !result)
      return std::unexpected(std::move(result.error()));
  }
  return context.load(*scripts.ptr(path));
}

std::size_t LuaRoot::ScriptCache::memoryUsage() const {
  std::unique_lock locker(mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  std::size_t total = 0;
  for (auto const& p : scripts)
    total += p.second.size();
  return total;
}

}
