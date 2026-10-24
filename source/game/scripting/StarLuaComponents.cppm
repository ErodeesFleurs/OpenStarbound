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
  friend class LuaScriptStorage;
  friend class LuaScriptUpdate;
  friend class LuaScriptMessages;
  friend class LuaScriptAnimation;

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


// These fixed-address feature values borrow their owning context. They do not
// own a LuaRoot and cannot be detached, copied or installed as dynamic plugins.
class LuaScriptStorage {
public:
  explicit LuaScriptStorage(LuaBaseComponent& host) : m_host(host) {}
  LuaScriptStorage(LuaScriptStorage const&) = delete;
  LuaScriptStorage& operator=(LuaScriptStorage const&) = delete;
  [[nodiscard]] std::expected<JsonObject, StarException> getScriptStorage() const;
  [[nodiscard]] std::expected<void, StarException> setScriptStorage(JsonObject storage);
  [[nodiscard]] std::expected<void, StarException> contextSetup();
  [[nodiscard]] std::expected<JsonObject, StarException> readStorage() const;
  [[nodiscard]] std::expected<void, StarException> finishShutdown(std::expected<JsonObject, StarException> storage, std::expected<void, StarException> shutdown);
private:
  LuaBaseComponent& m_host;
  JsonObject m_storage;
};

class LuaScriptUpdate {
public:
  explicit LuaScriptUpdate(LuaBaseComponent& host);
  LuaScriptUpdate(LuaScriptUpdate const&) = delete;
  LuaScriptUpdate& operator=(LuaScriptUpdate const&) = delete;
  unsigned updateDelta() const;
  float updateDt(float dt) const;
  float updateDt() const;
  void setUpdateDelta(unsigned delta);
  bool updateReady() const;
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args);
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args);
  [[nodiscard]] std::expected<void, StarException> contextSetup();
  [[nodiscard]] std::expected<void, StarException> contextShutdown();
  [[nodiscard]] static std::expected<void, StarException> finishShutdown(std::expected<void, StarException> stopped, std::expected<void, StarException> shutdown);
  [[nodiscard]] std::expected<void, StarException> addThreadsCallbacks(LuaCallbacks& callbacks);
  // Called by the concrete host's destructor, while all callback targets live.
  void destroy();
private:
  struct ThreadMessage {
    String originThread;
    String message;
    JsonArray args;
    RpcPromiseKeeper<Json> promise;
  };
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleThreadMessage(ThreadMessage const& message);
  LuaBaseComponent& m_host;
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

class LuaScriptWorld {
public:
  explicit LuaScriptWorld(LuaBaseComponent& host) : m_host(host) {}
  LuaScriptWorld(LuaScriptWorld const&) = delete;
  LuaScriptWorld& operator=(LuaScriptWorld const&) = delete;
  [[nodiscard]] std::expected<void, StarException> init(World* world);
  [[nodiscard]] std::expected<void, StarException> uninit();
private:
  LuaBaseComponent& m_host;
  StringSet m_worldThreadCallbacks;
};

class LuaScriptMessages {
public:
  explicit LuaScriptMessages(LuaBaseComponent& host) : m_host(host) {}
  LuaScriptMessages(LuaScriptMessages const&) = delete;
  LuaScriptMessages& operator=(LuaScriptMessages const&) = delete;
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {});
  [[nodiscard]] std::expected<void, StarException> contextSetup();
  void contextShutdown();
private:
  struct MessageHandler {
    std::optional<LuaFunction> function;
    String name;
    bool passName = true;
    bool localOnly = false;
  };
  LuaBaseComponent& m_host;
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

template <typename Ret, typename... V>
[[nodiscard]] std::expected<std::optional<Ret>, StarException> LuaScriptUpdate::update(V&&... args) {
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
  m_host.cleanThreads();
  return m_host.template invoke<Ret>("update", std::forward<V>(args)...);
}

// Fireable-item contexts need world callbacks, but no periodic update queue.
class LuaWorldScript final : public LuaBaseComponent {
public:
  explicit LuaWorldScript(Root& root) : LuaBaseComponent(root), m_world(*this) {}
  [[nodiscard]] std::expected<void, StarException> init(World* world) { return m_world.init(world); }
  [[nodiscard]] std::expected<void, StarException> uninit() { return m_world.uninit(); }
protected:
  using LuaBaseComponent::setLuaRoot;
private:
  LuaScriptWorld m_world;
};

// GUI scripts update without binding a world or exposing entity messages.
class LuaUpdateScript final : public LuaBaseComponent {
public:
  explicit LuaUpdateScript(Root& root) : LuaBaseComponent(root), m_update(*this) {}
  ~LuaUpdateScript() override { m_update.destroy(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    return m_update.template update<Ret>(std::forward<V>(args)...);
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
protected:
  [[nodiscard]] std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_update.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    return {};
  }
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override {
    auto stopped = m_update.contextShutdown();
    return LuaScriptUpdate::finishShutdown(std::move(stopped), LuaBaseComponent::contextShutdown());
  }
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeThreadsCallbacks() override {
    auto callbacks = LuaBaseComponent::makeThreadsCallbacks();
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = m_update.addThreadsCallbacks(*callbacks); !result)
      return std::unexpected(std::move(result.error()));
    return callbacks;
  }
private:
  LuaScriptUpdate m_update;
};

// Universe and worker-thread scripts update and handle messages without a world.
class LuaMessageScript final : public LuaBaseComponent {
public:
  explicit LuaMessageScript(Root& root) : LuaBaseComponent(root), m_update(*this), m_messages(*this) {}
  ~LuaMessageScript() override { m_update.destroy(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    return m_update.template update<Ret>(std::forward<V>(args)...);
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {}) {
    return m_messages.handleMessage(message, localMessage, args);
  }
protected:
  [[nodiscard]] std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_update.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_messages.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    return {};
  }
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override {
    m_messages.contextShutdown();
    auto stopped = m_update.contextShutdown();
    return LuaScriptUpdate::finishShutdown(std::move(stopped), LuaBaseComponent::contextShutdown());
  }
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeThreadsCallbacks() override {
    auto callbacks = LuaBaseComponent::makeThreadsCallbacks();
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = m_update.addThreadsCallbacks(*callbacks); !result)
      return std::unexpected(std::move(result.error()));
    return callbacks;
  }
private:
  LuaScriptUpdate m_update;
  LuaScriptMessages m_messages;
};

// Container interfaces bind a world and update, without entity-message state.
class LuaWorldUpdateScript final : public LuaBaseComponent {
public:
  explicit LuaWorldUpdateScript(Root& root) : LuaBaseComponent(root), m_world(*this), m_update(*this) {}
  ~LuaWorldUpdateScript() override { m_update.destroy(); }
  [[nodiscard]] std::expected<void, StarException> init(World* world) { return m_world.init(world); }
  [[nodiscard]] std::expected<void, StarException> uninit() { return m_world.uninit(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    return m_update.template update<Ret>(std::forward<V>(args)...);
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
protected:
  using LuaBaseComponent::setLuaRoot;
  [[nodiscard]] std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_update.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    return {};
  }
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override {
    auto stopped = m_update.contextShutdown();
    return LuaScriptUpdate::finishShutdown(std::move(stopped), LuaBaseComponent::contextShutdown());
  }
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeThreadsCallbacks() override {
    auto callbacks = LuaBaseComponent::makeThreadsCallbacks();
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = m_update.addThreadsCallbacks(*callbacks); !result)
      return std::unexpected(std::move(result.error()));
    return callbacks;
  }
private:
  LuaScriptWorld m_world;
  LuaScriptUpdate m_update;
};

// Transient entities handle messages without allocating persistent storage.
class LuaEntityScript final : public LuaBaseComponent {
public:
  explicit LuaEntityScript(Root& root) : LuaBaseComponent(root), m_world(*this), m_update(*this), m_messages(*this) {}
  ~LuaEntityScript() override { m_update.destroy(); }
  [[nodiscard]] std::expected<void, StarException> init(World* world) { return m_world.init(world); }
  [[nodiscard]] std::expected<void, StarException> uninit() { return m_world.uninit(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    return m_update.template update<Ret>(std::forward<V>(args)...);
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {}) {
    return m_messages.handleMessage(message, localMessage, args);
  }
protected:
  using LuaBaseComponent::setLuaRoot;
  [[nodiscard]] std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_update.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_messages.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    return {};
  }
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override {
    m_messages.contextShutdown();
    auto stopped = m_update.contextShutdown();
    return LuaScriptUpdate::finishShutdown(std::move(stopped), LuaBaseComponent::contextShutdown());
  }
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeThreadsCallbacks() override {
    auto callbacks = LuaBaseComponent::makeThreadsCallbacks();
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = m_update.addThreadsCallbacks(*callbacks); !result)
      return std::unexpected(std::move(result.error()));
    return callbacks;
  }
private:
  LuaScriptWorld m_world;
  LuaScriptUpdate m_update;
  LuaScriptMessages m_messages;
};

// Persistent entities snapshot storage before child-thread shutdown, then commit
// that snapshot after the producer queue has stopped and every promise is settled.
class LuaStoredEntityScript final : public LuaBaseComponent {
public:
  explicit LuaStoredEntityScript(Root& root) : LuaBaseComponent(root), m_world(*this), m_update(*this), m_storage(*this), m_messages(*this) {}
  ~LuaStoredEntityScript() override { m_update.destroy(); }
  [[nodiscard]] std::expected<void, StarException> init(World* world) { return m_world.init(world); }
  [[nodiscard]] std::expected<void, StarException> uninit() { return m_world.uninit(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    return m_update.template update<Ret>(std::forward<V>(args)...);
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
  [[nodiscard]] std::expected<JsonObject, StarException> getScriptStorage() const { return m_storage.getScriptStorage(); }
  [[nodiscard]] std::expected<void, StarException> setScriptStorage(JsonObject storage) { return m_storage.setScriptStorage(std::move(storage)); }
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {}) {
    return m_messages.handleMessage(message, localMessage, args);
  }
protected:
  using LuaBaseComponent::setLuaRoot;
  [[nodiscard]] std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_update.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_storage.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_messages.contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    return {};
  }
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override {
    m_messages.contextShutdown();
    auto storage = m_storage.readStorage();
    auto stopped = m_update.contextShutdown();
    auto shutdown = LuaScriptUpdate::finishShutdown(std::move(stopped), LuaBaseComponent::contextShutdown());
    return m_storage.finishShutdown(std::move(storage), std::move(shutdown));
  }
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeThreadsCallbacks() override {
    auto callbacks = LuaBaseComponent::makeThreadsCallbacks();
    if (!callbacks)
      return std::unexpected(std::move(callbacks.error()));
    if (auto result = m_update.addThreadsCallbacks(*callbacks); !result)
      return std::unexpected(std::move(result.error()));
    return callbacks;
  }
private:
  LuaScriptWorld m_world;
  LuaScriptUpdate m_update;
  LuaScriptStorage m_storage;
  LuaScriptMessages m_messages;
};

}

export module star.lua_components;

export namespace Star {
  using ::Star::LuaComponentExceptionTag;
  using ::Star::LuaComponentException;
  using ::Star::LuaBaseComponent;
  using ::Star::LuaScriptStorage;
  using ::Star::LuaScriptUpdate;
  using ::Star::LuaScriptWorld;
  using ::Star::LuaScriptMessages;
  using ::Star::LuaWorldScript;
  using ::Star::LuaUpdateScript;
  using ::Star::LuaMessageScript;
  using ::Star::LuaWorldUpdateScript;
  using ::Star::LuaEntityScript;
  using ::Star::LuaStoredEntityScript;
}
