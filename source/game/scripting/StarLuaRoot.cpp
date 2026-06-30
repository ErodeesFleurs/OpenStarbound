#include "StarLuaRoot.hpp"
#include "StarAlgorithm.hpp"
#include "StarException.hpp"
#include "StarFile.hpp"
#include "StarLogging.hpp"
#include "StarImageLuaBindings.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarTime.hpp"

namespace Star {

LuaRootServices requireLuaRootServices(LuaRootServices services, char const* context) {
  services.assets = requireServiceValueAs<StarException>(std::move(services.assets), context, "assets");
  services.configuration = requireServiceValueAs<StarException>(std::move(services.configuration), context, "configuration");
  services.root = requireServiceValueAs<StarException>(services.root, context, "root");
  services.storageDirectory = requireNonEmptyServiceValue(std::move(services.storageDirectory), context, "storage directory");
  return services;
}

LuaRoot::LuaRoot(LuaRootServices services) {
  m_services = requireLuaRootServices(std::move(services), "LuaRoot");
  m_assets = m_services.assets;
  m_scriptCache = make_shared<ScriptCache>(m_assets);
  addCallbacks("root", LuaBindings::makeRootCallbacks(*m_services.root));

  restart();

  m_rootReloadListener = make_shared<CallbackListener>([this, cache = m_scriptCache]() {
    cache->clear();
    m_reloadListeners.trigger();
  });
  if (m_services.registerReloadListener)
    m_services.registerReloadListener(m_rootReloadListener);

  m_storageDirectory = m_services.storageDirectory;
}

LuaRoot::~LuaRoot() {
  shutdown();
}

void LuaRoot::loadScript(String const& assetPath) {
  m_scriptCache->loadScript(*m_luaEngine, assetPath);
}

bool LuaRoot::scriptLoaded(String const& assetPath) const {
  return m_scriptCache->scriptLoaded(assetPath);
}

void LuaRoot::unloadScript(String const& assetPath) {
  m_scriptCache->unloadScript(assetPath);
}

void LuaRoot::restart() {
  shutdown();

  auto const& configuration = m_services.configuration;
  m_luaEngine = LuaEngine::create(configuration->get("safeScripts").toBool());
  LuaBindings::registerImageLuaAssets(*m_luaEngine, m_services.assets);

  m_luaEngine->setRecursionLimit(configuration->get("scriptRecursionLimit").toUInt());
  m_luaEngine->setInstructionLimit(configuration->get("scriptInstructionLimit").toUInt());
  m_luaEngine->setProfilingEnabled(configuration->get("scriptProfilingEnabled").toBool());
  m_luaEngine->setInstructionMeasureInterval(configuration->get("scriptInstructionMeasureInterval").toUInt());
}

void LuaRoot::shutdown() {
  clearScriptCache();

  if (!m_luaEngine)
    return;

  auto profile = m_luaEngine->getProfile();
  if (!profile.empty()) {
    profile.sort([](auto const& a, auto const& b) {
        return a.totalTime > b.totalTime;
      });

    std::function<Json (LuaProfileEntry const&)> jsonFromProfileEntry = [&](LuaProfileEntry const& entry) -> Json {
        JsonObject profile;
        profile.set("function", entry.name.value("<function>"));
        profile.set("scope", entry.nameScope.value("?"));
        profile.set("source", strf("{}:{}", entry.source, entry.sourceLine));
        profile.set("self", entry.selfTime);
        profile.set("total", entry.totalTime);
        List<LuaProfileEntry> calls;
        for (auto p : entry.calls)
          calls.append(*p.second);
        profile.set("calls", calls.sorted([](auto const& a, auto const& b) { return a.totalTime > b.totalTime; }).transformed(jsonFromProfileEntry));
        return profile;
      };

    String profileSummary = Json(profile.transformed(jsonFromProfileEntry)).repr(1);

    if (!File::isDirectory(m_storageDirectory)) {
      Logger::info("Creating lua storage directory");
      File::makeDirectory(m_storageDirectory);
    }

    String filename = strf("{}.luaprofile", Time::printCurrentDateAndTime("<year>-<month>-<day>-<hours>-<minutes>-<seconds>-<millis>"));
    String path = File::relativeTo(m_storageDirectory, filename);
    Logger::info("Writing lua profile {}", filename);
    File::writeFile(profileSummary, path);
  }

  m_luaEngine.reset();
}

LuaContext LuaRoot::createContext(String const& script) {
  return createContext(StringList{script});
}

LuaContext LuaRoot::createContext(StringList const& scriptPaths) {
  auto newContext = m_luaEngine->createContext();

  auto cache = m_scriptCache;
  newContext.setRequireFunction([cache](LuaContext& context, LuaString const& module) {
    if (!context.get("_SBLOADED").is<LuaTable>())
      context.set("_SBLOADED", context.createTable());
    auto t = context.get<LuaTable>("_SBLOADED");
    if (!t.contains(module)) {
      t.set(module, true);
      cache->loadContextScript(context, module.toString());
    }
  });

  for (auto const& scriptPath : scriptPaths) {
    if (m_assets->assetExists(scriptPath))
      cache->loadContextScript(newContext, scriptPath);
    else
      Logger::error("Script '{}' does not exist", scriptPath);
  }

  for (auto const& callbackPair : m_luaCallbacks)
    newContext.setCallbacks(callbackPair.first, callbackPair.second);

  return newContext;
}

void LuaRoot::collectGarbage(Maybe<unsigned> steps) {
  if (m_luaEngine)
    m_luaEngine->collectGarbage(steps);
}

void LuaRoot::setAutoGarbageCollection(bool autoGarbageColleciton) {
  if (m_luaEngine)
    m_luaEngine->setAutoGarbageCollection(autoGarbageColleciton);
}

void LuaRoot::tuneAutoGarbageCollection(float pause, float stepMultiplier) {
  if (m_luaEngine)
    m_luaEngine->tuneAutoGarbageCollection(pause, stepMultiplier);
}

size_t LuaRoot::luaMemoryUsage() const {
  return m_luaEngine ? m_luaEngine->memoryUsage() : 0;
}

size_t LuaRoot::scriptCacheMemoryUsage() const {
  return m_luaEngine ? m_scriptCache->memoryUsage() : 0;
}

void LuaRoot::clearScriptCache() const {
  return m_scriptCache->clear();
}

void LuaRoot::addCallbacks(String const& groupName, LuaCallbacks const& callbacks) {
  m_luaCallbacks[groupName] = callbacks;
}

void LuaRoot::registerReloadListener(ListenerWeakPtr reloadListener) {
  m_reloadListeners.addListener(std::move(reloadListener));
}

LuaRootServices const& LuaRoot::services() const {
  return m_services;
}

LuaEngine& LuaRoot::luaEngine() const {
  return *m_luaEngine;
}

LuaRoot::ScriptCache::ScriptCache(AssetsConstPtr assets)
  : m_assets(requireServiceValueAs<StarException>(std::move(assets), "LuaRoot::ScriptCache", "assets")) {}

void LuaRoot::ScriptCache::loadScript(LuaEngine& engine, String const& assetPath) {
  RecursiveMutexLocker locker(mutex);
  scripts[assetPath] = engine.compile(*m_assets->bytes(assetPath), assetPath);
}

bool LuaRoot::ScriptCache::scriptLoaded(String const& assetPath) const {
  RecursiveMutexLocker locker(mutex);
  return scripts.contains(assetPath);
}

void LuaRoot::ScriptCache::unloadScript(String const& assetPath) {
  RecursiveMutexLocker locker(mutex);
  scripts.remove(assetPath);
}

void LuaRoot::ScriptCache::clear() {
  RecursiveMutexLocker locker(mutex);
  scripts.clear();
}

void LuaRoot::ScriptCache::loadContextScript(LuaContext& context, String const& assetPath) {
  RecursiveMutexLocker locker(mutex);
  if (!scriptLoaded(assetPath))
    loadScript(context.engine(), assetPath);
  context.load(scripts.get(assetPath));
}

size_t LuaRoot::ScriptCache::memoryUsage() const {
  RecursiveMutexLocker locker(mutex);
  size_t total = 0;
  for (auto const& p : scripts)
    total += p.second.size();
  return total;
}

}
