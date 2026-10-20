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
import star.listener;
import std;
import star.string;
import star.lua;
import star.maybe;
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
  addCallbacks("sb", LuaBindings::makeUtilityCallbacks(root.random(), root.logger(), root.logMap()));
  addCallbacks("root", LuaBindings::makeRootCallbacks(root));
  setAutoReInit(true);
}

LuaBaseComponent::~LuaBaseComponent() {
  m_threads.clear();
}

Root& LuaBaseComponent::root() const {
  return m_root;
}

StringList const& LuaBaseComponent::scripts() const {
  return m_scripts;
}

void LuaBaseComponent::setScript(String script) {
  setScripts({std::move(script)});
}

void LuaBaseComponent::setScripts(StringList scripts) {
  if (initialized())
    throw LuaComponentException("Cannot call LuaWorldComponent::setScripts when LuaWorldComponent is initialized");

  m_scripts = std::move(scripts);
}

void LuaBaseComponent::addCallbacks(String groupName, LuaCallbacks callbacks) {
  if (!m_callbacks.insert(groupName, callbacks).second)
    throw LuaComponentException::format("Duplicate callbacks named '{}' in LuaBaseComponent", groupName);

  if (m_context)
    m_context->setCallbacks(groupName, callbacks);
}

bool LuaBaseComponent::removeCallbacks(String const& groupName) {
  if (m_callbacks.remove(groupName)) {
    if (m_context)
      m_context->remove(groupName);
    return true;
  }
  return false;
}

void LuaBaseComponent::addThreadCallbacks(String groupName, LuaCallbacks callbacks) {
  if (!m_threadCallbacks.insert(groupName, callbacks).second)
    throw LuaComponentException::format("Duplicate thread callbacks named '{}' in LuaBaseComponent", groupName);

  for (auto const& p : m_threads) {
    p.second->addCallbacks(groupName,callbacks);
  }
}

bool LuaBaseComponent::removeThreadCallbacks(String const& groupName) {
  if (m_threadCallbacks.remove(groupName)) {
    for (auto const& p : m_threads) {
      p.second->removeCallbacks(groupName);
    }
    return true;
  }
  return false;
}

bool LuaBaseComponent::autoReInit() const {
  return (bool)m_reloadTracker;
}

void LuaBaseComponent::setAutoReInit(bool autoReInit) {
  m_reloadSubscription.disconnect();
  if (autoReInit) {
    m_reloadTracker = std::make_shared<TrackerListener>();
    m_reloadSubscription = m_root.registerReloadListener(m_reloadTracker);
  } else {
    m_reloadTracker.reset();
  }
}

void LuaBaseComponent::setLuaRoot(std::shared_ptr<LuaRoot> luaRoot) {
  if (luaRoot && &luaRoot->root() != &m_root)
    throw LuaComponentException("Lua component and LuaRoot must belong to the same Root");
  m_luaRoot = std::move(luaRoot);
}

std::shared_ptr<LuaRoot> const& LuaBaseComponent::luaRoot() {
  return m_luaRoot;
}

bool LuaBaseComponent::init() {
  uninit();

  if (!m_luaRoot)
    return false;

  m_error.reset();
  try {
    m_context = m_luaRoot->createContext(m_scripts);
  } catch (LuaException const& e) {
    m_root.logger().error("Exception while creating lua context for scripts '{}': {}", m_scripts, outputException(e, true));
    m_error = String(printException(e, false));
    m_context.reset();
    return false;
  }
  contextSetup();

  if (m_context->containsPath("init")) {
    try {
      m_context->invokePath("init");
    } catch (LuaException const& e) {
      m_root.logger().error("Exception while calling script init: {}", outputException(e, true));
      m_error = String(printException(e, false));
      m_context.reset();
      return false;
    }
  }

  return true;
}

void LuaBaseComponent::uninit() {
  if (m_context) {
    if (m_context->containsPath("uninit")) {
      try {
        m_context->invokePath("uninit");
      } catch (LuaException const& e) {
        m_root.logger().error("Exception while calling script uninit: {}", outputException(e, true));
        m_error = String(printException(e, false));
      }
    }
    contextShutdown();
    m_context.reset();
  }
  for (auto p : m_threads) {
    p.second->stop();
  }

  m_error.reset();
}

bool LuaBaseComponent::initialized() const {
  return m_context.isValid();
}

Maybe<String> const& LuaBaseComponent::error() const {
  return m_error;
}

Maybe<LuaContext> const& LuaBaseComponent::context() const {
  return m_context;
}

Maybe<LuaContext>& LuaBaseComponent::context() {
  return m_context;
}

void LuaBaseComponent::contextSetup() {
  m_context->setPath("self", m_context->createTable());

  // set up here instead
  if (!m_callbacks.contains("threads"))
    addCallbacks("threads", makeThreadsCallbacks());
  
  for (auto const& p : m_callbacks)
    m_context->setCallbacks(p.first, p.second);
}

void LuaBaseComponent::contextShutdown() {}

void LuaBaseComponent::setError(String error) {
  m_context.reset();
  m_error = std::move(error);
}

bool LuaBaseComponent::checkInitialization() {
  // We should re-initialize if we are either already initialized or in an
  // error state (which means we WERE initialized until we had an error)
  bool shouldBeInitialized = initialized() || error();
  if (shouldBeInitialized && m_reloadTracker && m_reloadTracker->pullTriggered())
    init();
  return initialized();
}

void LuaBaseComponent::cleanThreads() {
  for (auto const& p : m_threads) {
    if (p.second->shouldExpire()) {
      m_threads.remove(p.first);
    }
  }
}

LuaCallbacks LuaBaseComponent::makeThreadsCallbacks() {
  LuaCallbacks callbacks;
  
  callbacks.registerCallback("create", [this](Json parameters) {
    cleanThreads();
    auto name = parameters.getString("name",Uuid(m_root.random()).hex());
    if (m_threads.contains(name)) {
      m_threads.get(name)->stop();
      m_threads.remove(name);
    }
    auto thread = std::make_shared<ScriptableThread>(parameters.set("name",name), this);
    
    for (auto const& callbackPair : m_threadCallbacks)
      thread->addCallbacks(callbackPair.first, callbackPair.second);
    
    thread->setPause(false);
    thread->start();
    m_threads.set(name,thread);
    return name;
  });
  callbacks.registerCallback("setPause", [this](String const& threadName, bool paused) {
    if (m_threads.contains(threadName))
      m_threads.get(threadName)->setPause(paused);
  });
  callbacks.registerCallback("stop", [this](String const& threadName) {
    if (m_threads.contains(threadName)) {
      m_threads.get(threadName)->stop();
      m_threads.remove(threadName);
    }
    cleanThreads();
  });
  callbacks.registerCallback("sendMessage", [this](String const& threadName, String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    if (!m_threads.contains(threadName))
      return RpcPromise<Json>::createFailed("Thread does not exist");
    
    auto thread = m_threads.get(threadName);
    if (thread->shouldExpire())
      return RpcPromise<Json>::createFailed("Thread is stopped");
    
    auto pair = RpcPromise<Json>::createPair();
    thread->passMessage({ message, JsonArray::from(std::move(args)), pair.second });
    return pair.first;
  });
  
  return callbacks;
}

RpcPromise<Json> LuaBaseComponent::threadPassMessage(String const& thread, String const& message, JsonArray const& args) {
  ((void)(thread));
  ((void)(message));
  ((void)(args));
  return RpcPromise<Json>::createFailed("Lua component cannot handle messages");
}

}
