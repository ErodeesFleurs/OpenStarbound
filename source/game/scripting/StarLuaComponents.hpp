#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarListener.hpp"
#include "StarLogging.hpp"
#include "StarPeriodic.hpp"
#include "StarWorld.hpp"
#include "StarWorldLuaBindings.hpp"

import std;

namespace Star {

using LuaComponentException = ExceptionDerived<"LuaComponentException", LuaException>;

class ScriptableThread;

// Basic lua component that can be initialized (takes and then owns a script
// context, calls the script context's init function) and uninitialized
// (releases the context, calls the context 'uninit' function).
//
// Callbacks can be added and removed whether or not the context is initialized
// or not, they will be added back during a call to init.  'root' callbacks are
// available by default as well as an ephemeral 'self' table.
//
// All script function calls (init / uninit / invoke) guard against missing
// functions.  If the function is missing, it will do nothing and return
// nothing.  If the function exists but throws an error, the error will be
// logged and the component will go into the error state.
//
// Whenever an error is set, all function calls or eval will fail until the
// error is cleared by re-initializing.
//
// If 'autoReInit' is set, Monitors Root for reloads, and if a root reload
// occurs, will automatically (on the next call to invoke) uninit and then
// re-init the script before calling invoke.  'autoReInit' defaults to true.
class LuaBaseComponent {
public:
  LuaBaseComponent();
  // The LuaBaseComponent destructor does NOT call the 'unint' entry point in
  // the script.  In order to do so, uninit() must be called manually before
  // destruction.  This is because during destruction, it is highly likely that
  // callbacks may not be valid, and highly likely that exceptions could be
  // thrown.
  virtual ~LuaBaseComponent();

  LuaBaseComponent(LuaBaseComponent const& component) = delete;
  auto operator=(LuaBaseComponent const& component) -> LuaBaseComponent& = delete;

  auto scripts() const -> StringList const&;
  void setScript(String script);
  void setScripts(StringList scripts);

  void addCallbacks(String groupName, LuaCallbacks callbacks);
  auto removeCallbacks(String const& groupName) -> bool;

  // If true, component will automatically uninit and re-init when root is
  // reloaded.
  auto autoReInit() const -> bool;
  void setAutoReInit(bool autoReInit);

  // Lua components require access to a LuaRoot object to initialize /
  // uninitialize.
  void setLuaRoot(Ptr<LuaRoot> luaRoot);
  auto luaRoot() -> Ptr<LuaRoot> const&;

  // init returns true on success, false if there has been an error
  // initializing the script.  LuaRoot must be set before calling or this will
  // always fail.  Calls the 'init' entry point on the script context.
  auto init() -> bool;
  // uninit will uninitialize the LuaComponent if it is currently initialized.
  // This calls the 'uninit' entry point on the script context before
  // destroying the context.
  void uninit();

  auto initialized() const -> bool;

  template <typename Ret = LuaValue, typename... V>
  auto invoke(String const& name, V&&... args) -> std::optional<Ret>;

  template <typename Ret = LuaValue>
  auto eval(String const& code) -> std::optional<LuaValue>;

  // Returns last error, if there has been an error.  Errors can only be
  // cleared by re-initializing the context.
  auto error() const -> std::optional<String> const&;

  auto context() const -> std::optional<LuaContext> const&;
  auto context() -> std::optional<LuaContext>&;

protected:
  virtual void contextSetup();
  virtual void contextShutdown();

  void setError(String error);

  // Checks the initialization state of the script, while also reloading the
  // script and clearing the error state if a root reload has occurred.
  auto checkInitialization() -> bool;

private:
  auto makeThreadsCallbacks() -> LuaCallbacks;

  StringList m_scripts;
  StringMap<LuaCallbacks> m_callbacks;
  Ptr<LuaRoot> m_luaRoot;
  Ptr<TrackerListener> m_reloadTracker;
  std::optional<LuaContext> m_context;
  std::optional<String> m_error;

  StringMap<std::shared_ptr<ScriptableThread>> m_threads;
  mutable RecursiveMutex m_threadLock;
};

// Wraps a basic lua component to add a persistent storage table translated
// into JSON that can be stored outside of the script context.
template <typename Base>
class LuaStorableComponent : public Base {
public:
  [[nodiscard]] auto getScriptStorage() const -> JsonObject;
  void setScriptStorage(JsonObject storage);

protected:
  void contextSetup() override;
  void contextShutdown() override;

private:
  JsonObject m_storage;
};

// Wraps a basic lua component with an 'update' method and an embedded tick
// rate.  Every call to 'update' here will only call the internal script
// 'update' at the configured delta.  Adds a update tick controls under the
// 'script' callback table.
template <typename Base>
class LuaUpdatableComponent : public Base {
public:
  LuaUpdatableComponent();

  auto updateDelta() const -> unsigned;
  auto updateDt(float dt) const -> float;
  auto updateDt() const -> float;
  void setUpdateDelta(unsigned updateDelta);

  // Returns true if the next update will call the internal script update
  // method.
  auto updateReady() const -> bool;

  template <typename Ret = LuaValue, typename... V>
  auto update(V&&... args) -> std::optional<Ret>;

private:
  Periodic m_updatePeriodic;
  mutable float m_lastDt;
};

// Wraps a basic lua component so that world callbacks are added on init, and
// removed on uninit, and sets the world LuaRoot as the LuaBaseComponent
// LuaRoot automatically.
template <typename Base>
class LuaWorldComponent : public Base {
public:
  void init(World* world);
  void uninit();

protected:
  using Base::init;
  using Base::setLuaRoot;
};

// Component for scripts which can be used as entity message handlers, provides
// a 'message' table with 'setHandler' callback to set message handlers.
template <typename Base>
class LuaMessageHandlingComponent : public Base {
public:
  LuaMessageHandlingComponent();

  auto handleMessage(String const& message, bool localMessage, JsonArray const& args = {}) -> std::optional<Json>;

protected:
  void contextShutdown() override;

private:
  struct MessageHandler {
    std::optional<LuaFunction> function;
    String name;
    bool passName = true;
    bool localOnly = false;
  };

  StringMap<MessageHandler> m_messageHandlers;
};

template <typename Ret, typename... V>
auto LuaBaseComponent::invoke(String const& name, V&&... args) -> std::optional<Ret> {
  if (!checkInitialization())
    return std::nullopt;

  try {
    auto method = m_context->getPath(name);
    if (method == LuaNil)
      return std::nullopt;
    return m_context->luaTo<LuaFunction>(std::move(method)).invoke<Ret>(std::forward<V>(args)...);
  } catch (LuaException const& e) {
    Logger::error("Exception while invoking lua function '{}'. {}", name, outputException(e, true));
    setError(printException(e, false));
    return {};
  }
}

template <typename Ret>
auto LuaBaseComponent::eval(String const& code) -> std::optional<LuaValue> {
  if (!checkInitialization())
    return std::nullopt;

  try {
    return m_context->eval<Ret>(code);
  } catch (LuaException const& e) {
    Logger::error("Exception while evaluating lua in context: {}", outputException(e, true));
    return std::nullopt;
  }
}

template <typename Base>
auto LuaStorableComponent<Base>::getScriptStorage() const -> JsonObject {
  if (Base::initialized())
    return Base::context()->template getPath<JsonObject>("storage");
  else
    return m_storage;
}

template <typename Base>
void LuaStorableComponent<Base>::setScriptStorage(JsonObject storage) {
  if (Base::initialized())
    Base::context()->setPath("storage", std::move(storage));
  else
    m_storage = std::move(storage);
}

template <typename Base>
void LuaStorableComponent<Base>::contextSetup() {
  Base::contextSetup();
  Base::context()->setPath("storage", std::move(m_storage));
}

template <typename Base>
void LuaStorableComponent<Base>::contextShutdown() {
  m_storage = Base::context()->template getPath<JsonObject>("storage");
  Base::contextShutdown();
}

template <typename Base>
LuaUpdatableComponent<Base>::LuaUpdatableComponent() {
  m_updatePeriodic.setStepCount(1);

  LuaCallbacks scriptCallbacks;
  scriptCallbacks.registerCallback("updateDt", [this]() -> auto {
    return updateDt();
  });
  scriptCallbacks.registerCallback("setUpdateDelta", [this](unsigned d) -> auto {
    setUpdateDelta(d);
  });

  m_lastDt = GlobalTimestep * GlobalTimescale;
  Base::addCallbacks("script", std::move(scriptCallbacks));
}

template <typename Base>
auto LuaUpdatableComponent<Base>::updateDelta() const -> unsigned {
  return m_updatePeriodic.stepCount();
}

template <typename Base>
auto LuaUpdatableComponent<Base>::updateDt(float dt) const -> float {
  m_lastDt = dt;
  return m_updatePeriodic.stepCount() * dt;
}

template <typename Base>
auto LuaUpdatableComponent<Base>::updateDt() const -> float {
  return m_updatePeriodic.stepCount() * m_lastDt;
}

template <typename Base>
void LuaUpdatableComponent<Base>::setUpdateDelta(unsigned updateDelta) {
  m_updatePeriodic.setStepCount(updateDelta);
}

template <typename Base>
auto LuaUpdatableComponent<Base>::updateReady() const -> bool {
  return m_updatePeriodic.ready();
}

template <typename Base>
template <typename Ret, typename... V>
auto LuaUpdatableComponent<Base>::update(V&&... args) -> std::optional<Ret> {
  if (!m_updatePeriodic.tick())
    return std::nullopt;

  return Base::template invoke<Ret>("update", std::forward<V>(args)...);
}

template <typename Base>
void LuaWorldComponent<Base>::init(World* world) {
  if (Base::initialized())
    uninit();

  Base::setLuaRoot(world->luaRoot());
  Base::addCallbacks("world", LuaBindings::makeWorldCallbacks(world));
  Base::init();
}

template <typename Base>
void LuaWorldComponent<Base>::uninit() {
  Base::uninit();
  Base::removeCallbacks("world");
}

template <typename Base>
LuaMessageHandlingComponent<Base>::LuaMessageHandlingComponent() {
  LuaCallbacks scriptCallbacks;
  scriptCallbacks.registerCallback("setHandler", [this](Variant<String, Json> message, std::optional<LuaFunction> handler) -> auto {
    MessageHandler handlerInfo = {};

    if (Json* config = message.ptr<Json>()) {
      handlerInfo.passName = config->getBool("passName", false);
      handlerInfo.localOnly = config->getBool("localOnly", false);
      handlerInfo.name = config->getString("name");
    } else {
      handlerInfo.passName = true;
      handlerInfo.localOnly = false;
      handlerInfo.name = message.get<String>();
    }

    if (handler) {
      handlerInfo.function = std::move(handler);
      m_messageHandlers.set(handlerInfo.name, handlerInfo);
    } else
      m_messageHandlers.remove(handlerInfo.name);
  });

  Base::addCallbacks("message", std::move(scriptCallbacks));
}

template <typename Base>
auto LuaMessageHandlingComponent<Base>::handleMessage(
  String const& message, bool localMessage, JsonArray const& args) -> std::optional<Json> {
  if (!Base::initialized())
    return std::nullopt;

  if (auto handler = m_messageHandlers.ptr(message)) {
    try {
      if (handler->localOnly) {
        if (!localMessage)
          return std::nullopt;
        else if (handler->passName)
          return handler->function->template invoke<Json>(message, luaUnpack(args));
        else
          return handler->function->template invoke<Json>(luaUnpack(args));
      } else if (handler->passName)
        return handler->function->template invoke<Json>(message, localMessage, luaUnpack(args));
      else
        return handler->function->template invoke<Json>(localMessage, luaUnpack(args));
    } catch (LuaException const& e) {
      Logger::error(
        "Exception while invoking lua message handler for message '{}'. {}", message, outputException(e, true));
      Base::setError(String(printException(e, false)));
    }
  }
  return std::nullopt;
}

template <typename Base>
void LuaMessageHandlingComponent<Base>::contextShutdown() {
  m_messageHandlers.clear();
  Base::contextShutdown();
}
}// namespace Star
