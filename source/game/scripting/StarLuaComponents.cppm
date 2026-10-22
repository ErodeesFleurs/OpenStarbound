module;
import std;
import star.optional;
import star.string;
import star.lua;
import star.format;
import star.json;
import star.periodic;
import star.logging;
import star.output_proxy;
import star.thread;
import star.listener;
import star.game_types;
import star.list;
import star.rpc_promise;
import star.lua_root;
import star.lua_converters;
import star.lua_game_converters;
import star.world;
import star.world_lua_bindings;
import star.root;

namespace Star {

struct LuaComponentExceptionTag {
  static constexpr char const* name() { return "LuaComponentException"; }
};
using LuaComponentException = StarError<LuaComponentExceptionTag, LuaException>;

class ScriptableThread;

// Owns a script context. Missing entry points are successful empty results;
// script failures are logged, returned, and disable the context until reinit.
// Builtin callbacks are installed during contextSetup, not construction.
class LuaBaseComponent {
public:
  explicit LuaBaseComponent(Root& root);
  Root& root() const;
  // Call uninit explicitly while callbacks are still valid. Destruction does
  // not invoke script entry points.
  virtual ~LuaBaseComponent();

  LuaBaseComponent(LuaBaseComponent const&) = delete;
  LuaBaseComponent& operator=(LuaBaseComponent const&) = delete;

  StringList const& scripts() const;
  [[nodiscard]] std::expected<void, StarException> setScript(String script);
  [[nodiscard]] std::expected<void, StarException> setScripts(StringList scripts);
  [[nodiscard]] std::expected<void, StarException> addCallbacks(String groupName, LuaCallbacks callbacks);
  [[nodiscard]] std::expected<bool, StarException> removeCallbacks(String const& groupName);
  [[nodiscard]] std::expected<void, StarException> addThreadCallbacks(String groupName, LuaCallbacks callbacks);
  [[nodiscard]] std::expected<bool, StarException> removeThreadCallbacks(String const& groupName);

  bool autoReInit() const;
  void setAutoReInit(bool enabled);
  [[nodiscard]] std::expected<void, StarException> setLuaRoot(std::shared_ptr<LuaRoot> luaRoot);
  std::shared_ptr<LuaRoot> const& luaRoot();

  // A missing LuaRoot is the existing successful false initialization outcome.
  [[nodiscard]] std::expected<bool, StarException> init();
  [[nodiscard]] std::expected<void, StarException> uninit();
  bool initialized() const;

  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> invoke(String const& name, V&&... args);
  template <typename Ret = LuaValue>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> eval(String const& code);

  std::optional<String> const& error() const;
  std::optional<LuaContext> const& context() const;
  std::optional<LuaContext>& context();
  virtual RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args);

protected:
  [[nodiscard]] virtual std::expected<void, StarException> contextSetup();
  [[nodiscard]] virtual std::expected<void, StarException> contextShutdown();
  bool hasCallbacks(String const& groupName) const;
  [[nodiscard]] std::expected<void, StarException> setError(String error);
  [[nodiscard]] std::expected<bool, StarException> checkInitialization();
  [[nodiscard]] virtual std::expected<LuaCallbacks, StarException> makeThreadsCallbacks();
  void cleanThreads();
  [[nodiscard]] std::expected<void, StarException> stopThreads();

private:
  Root& m_root;
  StringList m_scripts;
  StringMap<LuaCallbacks> m_callbacks;
  StringMap<LuaCallbacks> m_threadCallbacks;
  std::shared_ptr<LuaRoot> m_luaRoot;
  std::shared_ptr<TrackerListener> m_reloadTracker;
  ListenerSubscription m_reloadSubscription;
  std::optional<LuaContext> m_context;
  std::optional<String> m_error;
  StringMap<std::shared_ptr<ScriptableThread>> m_threads;
};

template <typename Base>
class LuaStorableComponent : public Base {
public:
  using Base::Base;
  [[nodiscard]] std::expected<JsonObject, StarException> getScriptStorage() const;
  [[nodiscard]] std::expected<void, StarException> setScriptStorage(JsonObject storage);
protected:
  [[nodiscard]] std::expected<void, StarException> contextSetup() override;
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override;
private:
  JsonObject m_storage;
};

template <typename Base>
class LuaUpdatableComponent : public Base {
public:
  struct ThreadMessage {
    String originThread;
    String message;
    JsonArray args;
    RpcPromiseKeeper<Json> promise;
  };
  explicit LuaUpdatableComponent(Root& root);
  ~LuaUpdatableComponent() override;
  unsigned updateDelta() const;
  float updateDt(float dt) const;
  float updateDt() const;
  void setUpdateDelta(unsigned delta);
  bool updateReady() const;
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args);
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override;
protected:
  [[nodiscard]] std::expected<void, StarException> contextSetup() override;
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override;
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeThreadsCallbacks() override;
private:
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleThreadMessage(ThreadMessage const& message);
  Periodic m_updatePeriodic;
  mutable float m_lastDt;
  List<ThreadMessage> m_threadMessages;
  mutable RecursiveMutex m_threadMessageMutex;
  struct ThreadMessageHandler {
    std::optional<LuaFunction> function;
    String name;
  };
  StringMap<ThreadMessageHandler> m_threadMessageHandlers;
};

template <typename Base>
class LuaWorldComponent : public Base {
public:
  using Base::Base;
  [[nodiscard]] std::expected<void, StarException> init(World* world);
  [[nodiscard]] std::expected<void, StarException> uninit();
protected:
  using Base::setLuaRoot;
  using Base::init;
private:
  StringSet m_worldThreadCallbacks;
};

template <typename Base>
class LuaMessageHandlingComponent : public Base {
public:
  using Base::Base;
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {});
protected:
  [[nodiscard]] std::expected<void, StarException> contextSetup() override;
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override;
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
[[nodiscard]] std::expected<std::optional<Ret>, StarException> LuaBaseComponent::invoke(String const& name, V&&... args) {
  auto initialized = checkInitialization();
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  if (!*initialized)
    return std::optional<Ret>{};
  auto result = [&]() -> std::expected<std::optional<Ret>, StarException> {
    auto method = m_context->getPath(name);
    if (!method)
      return std::unexpected(std::move(method.error()));
    if (std::holds_alternative<LuaNilType>(*method))
      return std::optional<Ret>{};
    auto function = m_context->template luaTo<LuaFunction>(std::move(*method));
    if (!function)
      return std::unexpected(std::move(function.error()));
    auto value = function->template invoke<Ret>(std::forward<V>(args)...);
    if (!value)
      return std::unexpected(std::move(value.error()));
    return std::optional<Ret>(std::move(*value));
  }();
  if (!result) {
    if (auto cleanup = setError(printException(result.error(), false)); !cleanup)
      result.error() = cleanup.error().withContext(strf("Original Lua context failure: {}", outputException(result.error(), true)));
    if (auto logged = root().logger().error("Exception while invoking lua function '{}'. {}", name, outputException(result.error(), true)); !logged)
      return std::unexpected(logged.error().withContext(strf("Could not log Lua function failure: {}", outputException(result.error(), true))));
  }
  return result;
}

template <typename Ret>
[[nodiscard]] std::expected<std::optional<Ret>, StarException> LuaBaseComponent::eval(String const& code) {
  auto initialized = checkInitialization();
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  if (!*initialized)
    return std::optional<Ret>{};
  auto result = m_context->template eval<Ret>(code);
  if (!result) {
    if (auto logged = root().logger().error("Exception while evaluating lua in context: {}", outputException(result.error(), true)); !logged)
      return std::unexpected(logged.error().withContext(strf("Could not log Lua evaluation failure: {}", outputException(result.error(), true))));
    return std::unexpected(std::move(result.error()));
  }
  return std::optional<Ret>(std::move(*result));
}

template <typename Base>
[[nodiscard]] std::expected<JsonObject, StarException> LuaStorableComponent<Base>::getScriptStorage() const {
  if (Base::initialized())
    return Base::context()->template getPath<JsonObject>("storage");
  return m_storage;
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaStorableComponent<Base>::setScriptStorage(JsonObject storage) {
  if (Base::initialized())
    return Base::context()->setPath("storage", std::move(storage));
  m_storage = std::move(storage);
  return {};
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaStorableComponent<Base>::contextSetup() {
  if (auto result = Base::contextSetup(); !result)
    return std::unexpected(std::move(result.error()));
  return Base::context()->setPath("storage", m_storage);
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaStorableComponent<Base>::contextShutdown() {
  auto storage = Base::context()->template getPath<JsonObject>("storage");
  auto shutdown = Base::contextShutdown();
  if (storage)
    m_storage = std::move(*storage);
  else {
    if (!shutdown)
      return std::unexpected(storage.error().withContext(strf("Error shutting down Lua storage context: {}", outputException(shutdown.error(), true))));
    return std::unexpected(std::move(storage.error()));
  }
  return shutdown;
}

template <typename Base>
LuaUpdatableComponent<Base>::LuaUpdatableComponent(Root& root) : Base(root), m_lastDt(root.timing().timestep * root.timing().timescale) {
  m_updatePeriodic.setStepCount(1);
}

template <typename Base>
LuaUpdatableComponent<Base>::~LuaUpdatableComponent() {
  // Child threads must stop before the parent-message queue is destroyed.
  if (auto stopped = Base::stopThreads(); !stopped)
    EmergencyLog::error("Error stopping Lua component threads: {}", outputException(stopped.error(), true));
  List<ThreadMessage> messages;
  {
    RecursiveMutexLocker locker(m_threadMessageMutex);
    messages = std::move(m_threadMessages);
  }
  for (auto& message : messages) {
    if (auto failed = message.promise.fail("Lua component destroyed"); !failed)
      EmergencyLog::error("Error settling destroyed Lua component message: {}", outputException(failed.error(), true));
  }
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaUpdatableComponent<Base>::contextSetup() {
  if (auto result = Base::contextSetup(); !result)
    return std::unexpected(std::move(result.error()));
  if (!Base::hasCallbacks("script")) {
    LuaCallbacks callbacks;
    if (auto result = callbacks.registerCallback("updateDt", [this]() { return updateDt(); }); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = callbacks.registerCallback("setUpdateDelta", [this](unsigned delta) { setUpdateDelta(delta); }); !result)
      return std::unexpected(std::move(result.error()));
    return Base::addCallbacks("script", std::move(callbacks));
  }
  return {};
}

template <typename Base>
unsigned LuaUpdatableComponent<Base>::updateDelta() const { return m_updatePeriodic.stepCount(); }

template <typename Base>
float LuaUpdatableComponent<Base>::updateDt(float dt) const {
  m_lastDt = dt;
  return m_updatePeriodic.stepCount() * dt;
}

template <typename Base>
float LuaUpdatableComponent<Base>::updateDt() const { return m_updatePeriodic.stepCount() * m_lastDt; }

template <typename Base>
void LuaUpdatableComponent<Base>::setUpdateDelta(unsigned delta) { m_updatePeriodic.setStepCount(delta); }

template <typename Base>
bool LuaUpdatableComponent<Base>::updateReady() const { return m_updatePeriodic.ready(); }

template <typename Base>
RpcPromise<Json> LuaUpdatableComponent<Base>::threadPassMessage(String const& thread, String const& message, JsonArray const& args) {
  auto pair = RpcPromise<Json>::createPair();
  RecursiveMutexLocker locker(m_threadMessageMutex);
  m_threadMessages.append({thread, message, args, pair.second});
  return pair.first;
}

template <typename Base>
[[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> LuaUpdatableComponent<Base>::handleThreadMessage(ThreadMessage const& message) {
  if (!Base::initialized())
    return std::optional<ChainableJsonMessageResponse>{};
  if (auto handler = m_threadMessageHandlers.ptr(message.message); handler && handler->function) {
    auto result = handler->function->template invoke<ChainableJsonMessageResponse>(message.originThread, message.message, luaUnpack(message.args));
    if (!result) {
      if (auto cleanup = Base::setError(printException(result.error(), false)); !cleanup)
        result.error() = cleanup.error().withContext(strf("Original Lua thread message failure: {}", outputException(result.error(), true)));
      if (auto logged = Base::root().logger().error("Exception while invoking lua thread message handler for message '{}'. {}", message.message, outputException(result.error(), true)); !logged)
        return std::unexpected(logged.error().withContext(strf("Could not log Lua thread message failure: {}", outputException(result.error(), true))));
      return std::unexpected(std::move(result.error()));
    }
    return std::optional<ChainableJsonMessageResponse>(std::move(*result));
  }
  return std::optional<ChainableJsonMessageResponse>{};
}

template <typename Base>
[[nodiscard]] std::expected<LuaCallbacks, StarException> LuaUpdatableComponent<Base>::makeThreadsCallbacks() {
  auto callbacks = Base::makeThreadsCallbacks();
  if (!callbacks)
    return std::unexpected(std::move(callbacks.error()));
  if (auto result = callbacks->registerCallback("setMessageHandler", [this](String message, std::optional<LuaFunction> handler) {
    if (handler) {
      ThreadMessageHandler info;
      info.name = message;
      info.function = std::move(handler);
      m_threadMessageHandlers.set(message, std::move(info));
    } else {
      m_threadMessageHandlers.remove(message);
    }
  }); !result)
    return std::unexpected(std::move(result.error()));
  return callbacks;
}

template <typename Base>
template <typename Ret, typename... V>
[[nodiscard]] std::expected<std::optional<Ret>, StarException> LuaUpdatableComponent<Base>::update(V&&... args) {
  List<ThreadMessage> messages;
  {
    RecursiveMutexLocker locker(m_threadMessageMutex);
    messages = std::move(m_threadMessages);
  }
  std::optional<StarException> error;
  for (auto& message : messages) {
    auto response = handleThreadMessage(message);
    std::expected<void, StarException> settled;
    if (!response) {
      settled = message.promise.fail(printException(response.error(), false));
      if (!error)
        error = std::move(response.error());
    } else if (!*response) {
      settled = message.promise.fail("Message not handled");
    } else if (auto promise = std::get_if<RpcPromise<Json>>(&**response)) {
      settled = message.promise.chain(*promise);
    } else {
      settled = message.promise.fulfill(std::get<Json>(**response));
    }
    if (!settled) {
      if (error)
        error = settled.error().withContext(strf("Earlier Lua thread message failure: {}", outputException(*error, true)));
      else
        error = std::move(settled.error());
    }
  }
  if (error)
    return std::unexpected(std::move(*error));
  if (!m_updatePeriodic.tick())
    return std::optional<Ret>{};
  Base::cleanThreads();
  return Base::template invoke<Ret>("update", std::forward<V>(args)...);
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaUpdatableComponent<Base>::contextShutdown() {
  // Stop producers before draining, so child uninit cannot enqueue a late,
  // permanently pending parent message.
  auto stopped = Base::stopThreads();
  List<ThreadMessage> messages;
  {
    RecursiveMutexLocker locker(m_threadMessageMutex);
    messages = std::move(m_threadMessages);
  }
  std::optional<StarException> error;
  if (!stopped)
    error = std::move(stopped.error());
  for (auto& message : messages) {
    if (auto result = message.promise.fail("Lua component uninitialized"); !result) {
      if (!error)
        error = std::move(result.error());
      else
        error = result.error().withContext(strf("Earlier Lua shutdown failure: {}", outputException(*error, true)));
    }
  }
  m_threadMessageHandlers.clear();
  auto shutdown = Base::contextShutdown();
  if (error) {
    if (!shutdown)
      return std::unexpected(error->withContext(strf("Error shutting down Lua component: {}", outputException(shutdown.error(), true))));
    return std::unexpected(std::move(*error));
  }
  return shutdown;
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaWorldComponent<Base>::init(World* world) {
  if (Base::initialized()) {
    if (auto result = uninit(); !result)
      return std::unexpected(std::move(result.error()));
  }
  if (auto result = Base::setLuaRoot(world->luaRoot()); !result)
    return std::unexpected(std::move(result.error()));
  auto worldCallbacks = LuaBindings::makeWorldCallbacks(Base::root(), world);
  if (!worldCallbacks)
    return std::unexpected(std::move(worldCallbacks.error()));
  if (auto result = Base::addCallbacks("world", std::move(*worldCallbacks)); !result)
    return std::unexpected(std::move(result.error()));
  auto result = [&]() -> std::expected<void, StarException> {
    auto callbacks = LuaBindings::makeWorldThreadCallbacks(Base::root(), world);
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto added = Base::addThreadCallbacks("world", std::move(*callbacks)); !added)
      return std::unexpected(std::move(added.error()));
    auto threadCallbacks = world->luaThreadCallbacks();
    if (!threadCallbacks)
      return std::unexpected(std::move(threadCallbacks.error()));
    for (auto const& p : *threadCallbacks) {
      if (auto added = Base::addThreadCallbacks(p.first, p.second); !added)
        return std::unexpected(std::move(added.error()));
      m_worldThreadCallbacks.add(p.first);
    }
    auto initialized = Base::init();
    if (!initialized)
      return std::unexpected(std::move(initialized.error()));
    return {};
  }();
  if (!result) {
    if (auto rollback = uninit(); !rollback)
      return std::unexpected(result.error().withContext(strf("Error rolling back Lua world component: {}", outputException(rollback.error(), true))));
  }
  return result;
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaWorldComponent<Base>::uninit() {
  std::optional<StarException> error;
  auto remember = [&](auto result) {
    if (!result) {
      if (!error)
        error = std::move(result.error());
      else
        error = result.error().withContext(strf("Earlier Lua world shutdown failure: {}", outputException(*error, true)));
    }
  };
  remember(Base::uninit());
  remember(Base::removeCallbacks("world"));
  remember(Base::removeThreadCallbacks("world"));
  for (auto const& name : m_worldThreadCallbacks)
    remember(Base::removeThreadCallbacks(name));
  m_worldThreadCallbacks.clear();
  if (error)
    return std::unexpected(std::move(*error));
  return {};
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaMessageHandlingComponent<Base>::contextSetup() {
  if (auto result = Base::contextSetup(); !result)
    return std::unexpected(std::move(result.error()));
  if (Base::hasCallbacks("message"))
    return {};
  LuaCallbacks callbacks;
  if (auto result = callbacks.registerCallback("setHandler", [this](std::variant<String, Json> message, std::optional<LuaFunction> handler) -> std::expected<void, StarException> {
    MessageHandler info;
    if (auto config = std::get_if<Json>(&message)) {
      auto passName = config->getBool("passName", false);
      if (!passName)
        return std::unexpected(std::move(passName.error()));
      auto localOnly = config->getBool("localOnly", false);
      if (!localOnly)
        return std::unexpected(std::move(localOnly.error()));
      auto name = config->getString("name");
      if (!name)
        return std::unexpected(std::move(name.error()));
      info.passName = *passName;
      info.localOnly = *localOnly;
      info.name = std::move(*name);
    } else {
      info.name = std::move(std::get<String>(message));
    }
    if (handler) {
      info.function = std::move(handler);
      auto name = info.name;
      m_messageHandlers.set(name, std::move(info));
    } else {
      m_messageHandlers.remove(info.name);
    }
    return {};
  }); !result)
    return std::unexpected(std::move(result.error()));
  return Base::addCallbacks("message", std::move(callbacks));
}

template <typename Base>
[[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> LuaMessageHandlingComponent<Base>::handleMessage(String const& message, bool local, JsonArray const& args) {
  if (!Base::initialized())
    return std::optional<ChainableJsonMessageResponse>{};
  auto handler = m_messageHandlers.ptr(message);
  if (!handler || !handler->function || (handler->localOnly && !local))
    return std::optional<ChainableJsonMessageResponse>{};
  auto result = [&]() -> std::expected<ChainableJsonMessageResponse, StarException> {
    if (handler->localOnly) {
      if (handler->passName)
        return handler->function->template invoke<ChainableJsonMessageResponse>(message, luaUnpack(args));
      return handler->function->template invoke<ChainableJsonMessageResponse>(luaUnpack(args));
    }
    if (handler->passName)
      return handler->function->template invoke<ChainableJsonMessageResponse>(message, local, luaUnpack(args));
    return handler->function->template invoke<ChainableJsonMessageResponse>(local, luaUnpack(args));
  }();
  if (!result) {
    if (auto cleanup = Base::setError(printException(result.error(), false)); !cleanup)
      result.error() = cleanup.error().withContext(strf("Original Lua message handler failure: {}", outputException(result.error(), true)));
    if (auto logged = Base::root().logger().error("Exception while invoking lua message handler for message '{}'. {}", message, outputException(result.error(), true)); !logged)
      return std::unexpected(logged.error().withContext(strf("Could not log Lua message handler failure: {}", outputException(result.error(), true))));
    return std::unexpected(std::move(result.error()));
  }
  return std::optional<ChainableJsonMessageResponse>(std::move(*result));
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaMessageHandlingComponent<Base>::contextShutdown() {
  m_messageHandlers.clear();
  return Base::contextShutdown();
}

}

export module star.lua_components;

export namespace Star {
  using ::Star::LuaComponentExceptionTag;
  using ::Star::LuaComponentException;
  using ::Star::LuaBaseComponent;
  using ::Star::LuaStorableComponent;
  using ::Star::LuaUpdatableComponent;
  using ::Star::LuaWorldComponent;
  using ::Star::LuaMessageHandlingComponent;
}
