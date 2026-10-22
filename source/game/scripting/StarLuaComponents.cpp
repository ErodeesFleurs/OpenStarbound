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
import star.string;
import star.lua;
import star.format;
import star.version;
import star.rpc_promise;

import star.lua_components;
import star.lua_root;
import star.lua_converters;
import star.game_types;
import star.uuid;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.utility_lua_bindings;
import star.scriptable_thread;
import star.root_lua_bindings;

namespace Star {

LuaBaseComponent::LuaBaseComponent(Root& root) : m_root(root) {
  setAutoReInit(true);
}

LuaBaseComponent::~LuaBaseComponent() {
  if (auto stopped = stopThreads(); !stopped)
    EmergencyLog::error("Error stopping Lua component threads: {}", outputException(stopped.error(), true));
  m_threads.clear();
}

Root& LuaBaseComponent::root() const { return m_root; }
StringList const& LuaBaseComponent::scripts() const { return m_scripts; }

std::expected<void, StarException> LuaBaseComponent::setScript(String script) {
  return setScripts({std::move(script)});
}

std::expected<void, StarException> LuaBaseComponent::setScripts(StringList scripts) {
  if (initialized())
    return std::unexpected(LuaComponentException("Cannot call LuaWorldComponent::setScripts when LuaWorldComponent is initialized"));
  m_scripts = std::move(scripts);
  return {};
}

std::expected<void, StarException> LuaBaseComponent::addCallbacks(String groupName, LuaCallbacks callbacks) {
  if (m_callbacks.contains(groupName))
    return std::unexpected(LuaComponentException::format("Duplicate callbacks named '{}' in LuaBaseComponent", groupName));
  if (m_context) {
    if (auto result = m_context->setCallbacks(groupName, callbacks); !result)
      return std::unexpected(std::move(result.error()));
  }
  m_callbacks.insert(std::move(groupName), std::move(callbacks));
  return {};
}

std::expected<bool, StarException> LuaBaseComponent::removeCallbacks(String const& groupName) {
  if (!m_callbacks.contains(groupName))
    return false;
  if (m_context) {
    if (auto result = m_context->remove(groupName); !result)
      return std::unexpected(std::move(result.error()));
  }
  m_callbacks.remove(groupName);
  return true;
}

bool LuaBaseComponent::hasCallbacks(String const& groupName) const {
  return m_callbacks.contains(groupName);
}

std::expected<void, StarException> LuaBaseComponent::addThreadCallbacks(String groupName, LuaCallbacks callbacks) {
  if (m_threadCallbacks.contains(groupName))
    return std::unexpected(LuaComponentException::format("Duplicate thread callbacks named '{}' in LuaBaseComponent", groupName));
  List<std::shared_ptr<ScriptableThread>> changed;
  for (auto const& p : m_threads) {
    if (auto result = p.second->addCallbacks(groupName, callbacks); !result) {
      for (auto const& thread : changed) {
        if (auto rollback = thread->removeCallbacks(groupName); !rollback)
          result.error() = result.error().withContext(strf("Error rolling back thread callbacks: {}", outputException(rollback.error(), true)));
      }
      return std::unexpected(std::move(result.error()));
    }
    changed.append(p.second);
  }
  m_threadCallbacks.insert(std::move(groupName), std::move(callbacks));
  return {};
}

std::expected<bool, StarException> LuaBaseComponent::removeThreadCallbacks(String const& groupName) {
  auto callbacks = m_threadCallbacks.ptr(groupName);
  if (!callbacks)
    return false;
  List<std::shared_ptr<ScriptableThread>> changed;
  for (auto const& p : m_threads) {
    if (auto removed = p.second->removeCallbacks(groupName); !removed) {
      for (auto const& thread : changed) {
        if (auto restored = thread->addCallbacks(groupName, *callbacks); !restored)
          removed.error() = restored.error().withContext(strf("Original thread callback removal failure: {}", outputException(removed.error(), true)));
      }
      return std::unexpected(std::move(removed.error()));
    }
    changed.append(p.second);
  }
  m_threadCallbacks.remove(groupName);
  return true;
}

bool LuaBaseComponent::autoReInit() const { return (bool)m_reloadTracker; }

void LuaBaseComponent::setAutoReInit(bool enabled) {
  m_reloadSubscription.disconnect();
  if (enabled) {
    m_reloadTracker = std::make_shared<TrackerListener>();
    m_reloadSubscription = m_root.registerReloadListener(m_reloadTracker);
  } else {
    m_reloadTracker.reset();
  }
}

std::expected<void, StarException> LuaBaseComponent::setLuaRoot(std::shared_ptr<LuaRoot> luaRoot) {
  if (luaRoot && &luaRoot->root() != &m_root)
    return std::unexpected(LuaComponentException("Lua component and LuaRoot must belong to the same Root"));
  m_luaRoot = std::move(luaRoot);
  return {};
}

std::shared_ptr<LuaRoot> const& LuaBaseComponent::luaRoot() { return m_luaRoot; }

std::expected<bool, StarException> LuaBaseComponent::init() {
  if (auto result = uninit(); !result)
    return std::unexpected(std::move(result.error()));
  if (!m_luaRoot)
    return false;
  m_error.reset();
  auto context = m_luaRoot->createContext(m_scripts);
  if (!context) {
    if (auto cleanup = setError(printException(context.error(), false)); !cleanup)
      context.error() = cleanup.error().withContext(strf("Original Lua context creation failure: {}", outputException(context.error(), true)));
    if (auto logged = m_root.logger().error("Exception while creating lua context for scripts '{}': {}", m_scripts, outputException(context.error(), true)); !logged)
      return std::unexpected(logged.error().withContext(strf("Could not log Lua context creation failure: {}", outputException(context.error(), true))));
    return std::unexpected(std::move(context.error()));
  }
  m_context = std::move(*context);
  auto result = [&]() -> std::expected<void, StarException> {
    if (auto setup = contextSetup(); !setup)
      return std::unexpected(std::move(setup.error()));
    auto hasInit = m_context->containsPath("init");
    if (!hasInit)
      return std::unexpected(std::move(hasInit.error()));
    if (*hasInit) {
      if (auto invoked = m_context->invokePath("init"); !invoked)
        return std::unexpected(std::move(invoked.error()));
    }
    return {};
  }();
  if (!result) {
    if (auto logged = m_root.logger().error("Exception while calling script init: {}", outputException(result.error(), true)); !logged)
      result.error() = logged.error().withContext(strf("Could not log Lua initialization failure: {}", outputException(result.error(), true)));
    if (auto cleanup = setError(printException(result.error(), false)); !cleanup)
      result.error() = cleanup.error().withContext(strf("Original Lua initialization failure: {}", outputException(result.error(), true)));
    return std::unexpected(std::move(result.error()));
  }
  return true;
}

std::expected<void, StarException> LuaBaseComponent::uninit() {
  std::optional<StarException> error;
  auto remember = [&](auto result, char const* operation) {
    if (!result) {
      if (auto logged = m_root.logger().error("{}: {}", operation, outputException(result.error(), true)); !logged)
        result.error() = logged.error().withContext(strf("Could not log Lua cleanup failure: {}", outputException(result.error(), true)));
      if (!error)
        error = std::move(result.error());
      else
        error = result.error().withContext(strf("Earlier Lua cleanup failure: {}", outputException(*error, true)));
    }
  };
  if (m_context) {
    auto hasUninit = m_context->containsPath("uninit");
    if (!hasUninit)
      remember(std::move(hasUninit), "Exception while finding script uninit");
    else if (*hasUninit)
      remember(m_context->invokePath("uninit"), "Exception while calling script uninit");
    remember(contextShutdown(), "Exception while shutting down script context");
    m_context.reset();
  }
  remember(stopThreads(), "Error stopping script threads");
  m_error.reset();
  if (error)
    return std::unexpected(std::move(*error));
  return {};
}

bool LuaBaseComponent::initialized() const { return m_context.has_value(); }
std::optional<String> const& LuaBaseComponent::error() const { return m_error; }
std::optional<LuaContext> const& LuaBaseComponent::context() const { return m_context; }
std::optional<LuaContext>& LuaBaseComponent::context() { return m_context; }

std::expected<void, StarException> LuaBaseComponent::contextSetup() {
  auto self = m_context->createTable();
  if (!self)
    return std::unexpected(std::move(self.error()));
  if (auto result = m_context->setPath("self", std::move(*self)); !result)
    return std::unexpected(std::move(result.error()));
  if (!hasCallbacks("sb")) {
    auto callbacks = LuaBindings::makeUtilityCallbacks(m_root.random(), m_root.logger(), m_root.logMap());
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = addCallbacks("sb", std::move(*callbacks)); !result)
      return std::unexpected(std::move(result.error()));
  }
  if (!hasCallbacks("root")) {
    auto callbacks = LuaBindings::makeRootCallbacks(m_root);
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = addCallbacks("root", std::move(*callbacks)); !result)
      return std::unexpected(std::move(result.error()));
  }
  if (!hasCallbacks("threads")) {
    auto callbacks = makeThreadsCallbacks();
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = addCallbacks("threads", std::move(*callbacks)); !result)
      return std::unexpected(std::move(result.error()));
  }
  for (auto const& p : m_callbacks) {
    if (auto result = m_context->setCallbacks(p.first, p.second); !result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> LuaBaseComponent::contextShutdown() { return {}; }

std::expected<void, StarException> LuaBaseComponent::setError(String error) {
  std::expected<void, StarException> shutdown;
  if (m_context)
    shutdown = contextShutdown();
  m_context.reset();
  m_error = std::move(error);
  return shutdown;
}

std::expected<bool, StarException> LuaBaseComponent::checkInitialization() {
  bool shouldBeInitialized = initialized() || error();
  if (shouldBeInitialized && m_reloadTracker && m_reloadTracker->pullTriggered())
    return init();
  return initialized();
}

void LuaBaseComponent::cleanThreads() {
  StringList expired;
  for (auto const& p : m_threads) {
    if (p.second->shouldExpire())
      expired.append(p.first);
  }
  for (auto const& name : expired)
    m_threads.remove(name);
}

std::expected<void, StarException> LuaBaseComponent::stopThreads() {
  std::optional<StarException> error;
  for (auto const& p : m_threads) {
    if (auto stopped = p.second->stop(); !stopped) {
      if (!error)
        error = std::move(stopped.error());
      else
        error = stopped.error().withContext(strf("Earlier Lua thread shutdown failure: {}", outputException(*error, true)));
    }
  }
  if (error)
    return std::unexpected(std::move(*error));
  return {};
}

std::expected<LuaCallbacks, StarException> LuaBaseComponent::makeThreadsCallbacks() {
  LuaCallbacks callbacks;
  if (auto result = callbacks.registerCallback("create", [this](Json parameters) -> std::expected<String, StarException> {
    cleanThreads();
    auto name = parameters.getString("name", Uuid(m_root.random()).hex());
    if (!name)
      return std::unexpected(std::move(name.error()));
    if (auto old = m_threads.ptr(*name)) {
      if (auto stopped = (*old)->stop(); !stopped)
        return std::unexpected(std::move(stopped.error()));
      m_threads.remove(*name);
    }
    auto named = parameters.set("name", *name);
    if (!named)
      return std::unexpected(std::move(named.error()));
    auto thread = ScriptableThread::create(std::move(*named), this);
    if (!thread)
      return std::unexpected(std::move(thread.error()));
    for (auto const& p : m_threadCallbacks) {
      if (auto added = (*thread)->addCallbacks(p.first, p.second); !added)
        return std::unexpected(std::move(added.error()));
    }
    (*thread)->setPause(false);
    if (auto started = (*thread)->start(); !started)
      return std::unexpected(std::move(started.error()));
    m_threads.set(*name, std::move(*thread));
    return std::move(*name);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setPause", [this](String const& name, bool paused) {
    if (auto thread = m_threads.ptr(name))
      (*thread)->setPause(paused);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("stop", [this](String const& name) -> std::expected<void, StarException> {
    if (auto thread = m_threads.ptr(name)) {
      if (auto stopped = (*thread)->stop(); !stopped)
        return std::unexpected(std::move(stopped.error()));
      m_threads.remove(name);
    }
    cleanThreads();
    return {};
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("sendMessage", [this](String const& name, String const& message, LuaVariadic<Json> args) -> std::expected<RpcPromise<Json>, StarException> {
    auto thread = m_threads.ptr(name);
    if (!thread)
      return RpcPromise<Json>::createFailed("Thread does not exist");
    if ((*thread)->shouldExpire())
      return RpcPromise<Json>::createFailed("Thread is stopped");
    auto pair = RpcPromise<Json>::createPair();
    if (auto passed = (*thread)->passMessage({message, JsonArray::from(std::move(args)), pair.second}); !passed)
      return std::unexpected(std::move(passed.error()));
    return pair.first;
  }); !result)
    return std::unexpected(std::move(result.error()));
  return callbacks;
}

RpcPromise<Json> LuaBaseComponent::threadPassMessage(String const&, String const&, JsonArray const&) {
  return RpcPromise<Json>::createFailed("Lua component cannot handle messages");
}

}
