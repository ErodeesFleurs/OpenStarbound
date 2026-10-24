module;
import std;
import star.format;
import star.lua;
import star.lua_components;
import star.root;
import star.world;
import star.string;
import star.rpc_promise;
import star.json;
import star.game_types;
import star.list;
import star.vector;
import star.platformer_astar_types;
import star.actor_movement_controller;
import star.lua_converters;
import star.lua_game_converters;
import star.movement_controller_lua_bindings;
import star.math_common;
import star.algorithm;
import star.output_proxy;


namespace Star {

// Buffered actor controls belong to one fixed-address script host. The host
// explicitly clears before updating and applies controls afterwards, even on error.
class LuaScriptMovement {
public:
  explicit LuaScriptMovement(LuaBaseComponent& host) : m_host(host) {}
  LuaScriptMovement(LuaScriptMovement const&) = delete;
  LuaScriptMovement& operator=(LuaScriptMovement const&) = delete;
  [[nodiscard]] std::expected<void, StarException> addActorMovementCallbacks(ActorMovementController* controller);
  [[nodiscard]] std::expected<void, StarException> removeActorMovementCallbacks();
  bool autoClearControls() const;
  void setAutoClearControls(bool enabled);
  void beforeUpdate(bool updateReady) {
    if (updateReady && m_autoClearControls)
      clearControls();
  }
  [[nodiscard]] std::expected<void, StarException> performControls();
private:
  LuaBaseComponent& m_host;
  void clearControls();
  ActorMovementController* m_movementController = nullptr;
  bool m_autoClearControls = true;
  float m_controlRotation = 0.0f;
  Vec2F m_controlAcceleration;
  Vec2F m_controlForce;
  std::optional<std::tuple<Vec2F, float>> m_controlApproachVelocity;
  std::optional<std::tuple<float, float, float, bool>> m_controlApproachVelocityAlongAngle;
  std::optional<ActorMovementParameters> m_controlParameters;
  std::optional<ActorMovementModifiers> m_controlModifiers;
  std::optional<std::tuple<Direction, bool>> m_controlMove;
  std::optional<Direction> m_controlFace;
  bool m_controlDown = false;
  bool m_controlCrouch = false;
  std::optional<bool> m_controlJump;
  bool m_controlHoldJump = false;
  std::optional<Vec2F> m_controlFly;
  bool m_resetPathMove = false;
  std::optional<std::pair<Vec2F, bool>> m_controlPathMove;
  std::optional<std::pair<Vec2F, bool>> m_pathMoveResult;
};

[[nodiscard]] inline std::expected<void, StarException> LuaScriptMovement::addActorMovementCallbacks(ActorMovementController* controller) {
  if (!controller)
    return removeActorMovementCallbacks();
  auto built = LuaBindings::makeMovementControllerCallbacks(controller);
  if (!built)
    return std::unexpected(std::move(built.error()));
  auto callbacks = std::move(*built);
  callbacks.removeCallback("setVelocity");
  if (auto result = callbacks.registerCallback("setVelocity", [this](Vec2F const& velocity) {
    m_resetPathMove = true;
    m_movementController->setVelocity(velocity);
  }); !result)
    return std::unexpected(std::move(result.error()));
  callbacks.removeCallback("setXVelocity");
  if (auto result = callbacks.registerCallback("setXVelocity", [this](float velocity) {
    m_resetPathMove = true;
    m_movementController->setXVelocity(velocity);
  }); !result)
    return std::unexpected(std::move(result.error()));
  callbacks.removeCallback("setYVelocity");
  if (auto result = callbacks.registerCallback("setYVelocity", [this](float velocity) {
    m_resetPathMove = true;
    m_movementController->setYVelocity(velocity);
  }); !result)
    return std::unexpected(std::move(result.error()));
  callbacks.removeCallback("addMomentum");
  if (auto result = callbacks.registerCallback("addMomentum", [this](Vec2F const& momentum) {
    m_resetPathMove = true;
    m_movementController->addMomentum(momentum);
  }); !result)
    return std::unexpected(std::move(result.error()));
  callbacks.removeCallback("setRotation");
  if (auto result = callbacks.registerCallback("setRotation", [this](float rotation) {
    m_resetPathMove = true;
    m_movementController->setRotation(rotation);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setAnchorState", [this](EntityId entity, std::size_t position) {
    return m_movementController->setAnchorState({entity, position});
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("resetAnchorState", [this]() { return m_movementController->resetAnchorState(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("anchorState", [this]() {
    if (auto state = m_movementController->anchorState())
      return LuaVariadic<LuaValue>{LuaInt(state->entityId), LuaInt(state->positionIndex)};
    return LuaVariadic<LuaValue>{};
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("baseParameters", [this]() { return m_movementController->baseParameters(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("walking", [this]() { return m_movementController->walking(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("running", [this]() { return m_movementController->running(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("movingDirection", [this]() { return numericalDirection(m_movementController->movingDirection()); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("facingDirection", [this]() { return numericalDirection(m_movementController->facingDirection()); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("crouching", [this]() { return m_movementController->crouching(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("flying", [this]() { return m_movementController->flying(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("falling", [this]() { return m_movementController->falling(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("canJump", [this]() { return m_movementController->canJump(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("jumping", [this]() { return m_movementController->jumping(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("groundMovement", [this]() { return m_movementController->groundMovement(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("liquidMovement", [this]() { return m_movementController->liquidMovement(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlRotation", [this](float rotation) { m_controlRotation += rotation; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlAcceleration", [this](Vec2F const& acceleration) { m_controlAcceleration += acceleration; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlForce", [this](Vec2F const& force) { m_controlForce += force; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlApproachVelocity", [this](Vec2F const& velocity, float force) {
    m_controlApproachVelocity.emplace(velocity, force);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlApproachVelocityAlongAngle", [this](float angle, float velocity, float force, bool positive) {
    m_controlApproachVelocityAlongAngle.emplace(angle, velocity, force, positive);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlApproachXVelocity", [this](float velocity, float force) {
    m_controlApproachVelocityAlongAngle.emplace(0.0f, velocity, force, false);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlApproachYVelocity", [this](float velocity, float force) {
    m_controlApproachVelocityAlongAngle.emplace(std::numbers::pi / 2.0f, velocity, force, false);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlParameters", [this](ActorMovementParameters const& parameters) {
    m_controlParameters = m_controlParameters.value_or(ActorMovementParameters{}).merge(parameters);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlModifiers", [this](ActorMovementModifiers const& modifiers) {
    m_controlModifiers = m_controlModifiers.value_or(ActorMovementModifiers{}).combine(modifiers);
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlMove", [this](std::optional<float> const& direction, std::optional<bool> const& run) {
    if (auto value = directionOf(direction.value_or(float{})))
      m_controlMove.emplace(*value, run.value_or(true));
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlFace", [this](std::optional<float> const& direction) {
    if (auto value = directionOf(direction.value_or(float{})))
      m_controlFace = *value;
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlDown", [this]() { m_controlDown = true; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlCrouch", [this]() { m_controlCrouch = true; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlJump", [this](bool jump) { m_controlJump = jump; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlHoldJump", [this]() { m_controlHoldJump = true; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlFly", [this](Vec2F const& velocity) { m_controlFly = velocity; }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("controlPathMove", [this](Vec2F const& position, std::optional<bool> run, std::optional<PlatformerAStar::Parameters> parameters) -> std::expected<std::optional<bool>, StarException> {
    if (m_pathMoveResult && m_pathMoveResult->first == position)
      return std::exchange(m_pathMoveResult, std::nullopt).transform([](auto const& result) { return result.second; });
    m_pathMoveResult.reset();
    auto result = m_movementController->pathMove(position, run.value_or(false), parameters);
    if (!result)
      return std::unexpected(std::move(result.error()));
    if (!*result)
      m_controlPathMove = std::pair<Vec2F, bool>(position, run.value_or(false));
    return result->transform([](auto const& value) { return value.second; });
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("pathfinding", [this]() { return m_movementController->pathfinding(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool>("autoClearControls", std::bind_front(&LuaScriptMovement::autoClearControls, this)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, bool>("setAutoClearControls", std::bind_front(&LuaScriptMovement::setAutoClearControls, this)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void>("clearControls", std::bind_front(&LuaScriptMovement::clearControls, this)); !result)
    return std::unexpected(std::move(result.error()));
  auto previous = m_movementController;
  m_movementController = controller;
  if (auto added = m_host.addCallbacks("mcontroller", std::move(callbacks)); !added) {
    m_movementController = previous;
    return std::unexpected(std::move(added.error()));
  }
  return {};
}

[[nodiscard]] inline std::expected<void, StarException> LuaScriptMovement::removeActorMovementCallbacks() {
  auto removed = m_host.removeCallbacks("mcontroller");
  if (!removed)
    return std::unexpected(std::move(removed.error()));
  m_movementController = nullptr;
  return {};
}

inline bool LuaScriptMovement::autoClearControls() const { return m_autoClearControls; }

inline void LuaScriptMovement::setAutoClearControls(bool enabled) { m_autoClearControls = enabled; }

[[nodiscard]] inline std::expected<void, StarException> LuaScriptMovement::performControls() {
  if (!m_movementController)
    return {};
  m_movementController->controlRotation(m_controlRotation);
  m_movementController->controlAcceleration(m_controlAcceleration);
  m_movementController->controlForce(m_controlForce);
  if (m_controlApproachVelocity)
    std::apply(std::bind_front(&ActorMovementController::controlApproachVelocity, m_movementController), *m_controlApproachVelocity);
  if (m_controlApproachVelocityAlongAngle)
    std::apply(std::bind_front(&ActorMovementController::controlApproachVelocityAlongAngle, m_movementController), *m_controlApproachVelocityAlongAngle);
  if (m_controlParameters)
    m_movementController->controlParameters(*m_controlParameters);
  if (m_controlModifiers)
    m_movementController->controlModifiers(*m_controlModifiers);
  if (m_controlMove)
    std::apply(std::bind_front(&ActorMovementController::controlMove, m_movementController), *m_controlMove);
  if (m_controlFace)
    m_movementController->controlFace(*m_controlFace);
  if (m_controlDown)
    m_movementController->controlDown();
  if (m_controlCrouch)
    m_movementController->controlCrouch();
  if (m_controlJump)
    m_movementController->controlJump(*m_controlJump);
  if (m_controlHoldJump && !m_movementController->onGround())
    m_movementController->controlJump();
  if (m_controlFly)
    m_movementController->controlFly(*m_controlFly);
  if (m_resetPathMove)
    m_controlPathMove.reset();
  if (m_controlPathMove && !m_pathMoveResult) {
    auto result = m_movementController->controlPathMove(m_controlPathMove->first, m_controlPathMove->second);
    if (!result)
      return std::unexpected(std::move(result.error()));
    m_pathMoveResult = std::move(*result);
  }
  return {};
}

inline void LuaScriptMovement::clearControls() {
  m_controlRotation = {};
  m_controlAcceleration = {};
  m_controlForce = {};
  m_controlApproachVelocity.reset();
  m_controlApproachVelocityAlongAngle.reset();
  m_controlParameters.reset();
  m_controlModifiers.reset();
  m_controlMove.reset();
  m_controlFace.reset();
  m_controlDown = false;
  m_controlCrouch = false;
  m_controlJump.reset();
  m_controlHoldJump = false;
  m_controlFly.reset();
  m_resetPathMove = false;
  if (!m_controlPathMove)
    m_pathMoveResult.reset();
  m_controlPathMove.reset();
}

// Persistent entity scripts snapshot storage before stopping child threads.
class LuaActorScript final : public LuaBaseComponent {
public:
  explicit LuaActorScript(Root& root) : LuaBaseComponent(root), m_world(*this), m_update(*this), m_storage(*this), m_messages(*this), m_movement(*this) {}
  ~LuaActorScript() override { m_update.destroy(); }
  [[nodiscard]] std::expected<void, StarException> init(World* world) { return m_world.init(world); }
  [[nodiscard]] std::expected<void, StarException> uninit() { return m_world.uninit(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    m_movement.beforeUpdate(m_update.updateReady());
    auto result = m_update.template update<Ret>(std::forward<V>(args)...);
    auto controls = m_movement.performControls();
    if (!controls) {
      if (!result)
        return std::unexpected(controls.error().withContext(strf("Original Lua movement update failure: {}", outputException(result.error(), true))));
      return std::unexpected(std::move(controls.error()));
    }
    return result;
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
  [[nodiscard]] std::expected<JsonObject, StarException> getScriptStorage() const { return m_storage.getScriptStorage(); }
  [[nodiscard]] std::expected<void, StarException> setScriptStorage(JsonObject storage) { return m_storage.setScriptStorage(std::move(storage)); }
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {}) {
    return m_messages.handleMessage(message, localMessage, args);
  }
  [[nodiscard]] std::expected<void, StarException> addActorMovementCallbacks(ActorMovementController* controller) { return m_movement.addActorMovementCallbacks(controller); }
  [[nodiscard]] std::expected<void, StarException> removeActorMovementCallbacks() { return m_movement.removeActorMovementCallbacks(); }
  bool autoClearControls() const { return m_movement.autoClearControls(); }
  void setAutoClearControls(bool enabled) { m_movement.setAutoClearControls(enabled); }
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
  LuaScriptMovement m_movement;
};

// Active-item / quest scripts install storage before update callbacks, and read
// it back after stopping child threads. This differs intentionally from actors.
class LuaActionScript final : public LuaBaseComponent {
public:
  explicit LuaActionScript(Root& root) : LuaBaseComponent(root), m_world(*this), m_update(*this), m_storage(*this), m_messages(*this), m_movement(*this) {}
  ~LuaActionScript() override { m_update.destroy(); }
  [[nodiscard]] std::expected<void, StarException> init(World* world) { return m_world.init(world); }
  [[nodiscard]] std::expected<void, StarException> uninit() { return m_world.uninit(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    m_movement.beforeUpdate(m_update.updateReady());
    auto result = m_update.template update<Ret>(std::forward<V>(args)...);
    auto controls = m_movement.performControls();
    if (!controls) {
      if (!result)
        return std::unexpected(controls.error().withContext(strf("Original Lua movement update failure: {}", outputException(result.error(), true))));
      return std::unexpected(std::move(controls.error()));
    }
    return result;
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
  [[nodiscard]] std::expected<JsonObject, StarException> getScriptStorage() const { return m_storage.getScriptStorage(); }
  [[nodiscard]] std::expected<void, StarException> setScriptStorage(JsonObject storage) { return m_storage.setScriptStorage(std::move(storage)); }
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {}) {
    return m_messages.handleMessage(message, localMessage, args);
  }
  [[nodiscard]] std::expected<void, StarException> addActorMovementCallbacks(ActorMovementController* controller) { return m_movement.addActorMovementCallbacks(controller); }
  [[nodiscard]] std::expected<void, StarException> removeActorMovementCallbacks() { return m_movement.removeActorMovementCallbacks(); }
  bool autoClearControls() const { return m_movement.autoClearControls(); }
  void setAutoClearControls(bool enabled) { m_movement.setAutoClearControls(enabled); }
protected:
  using LuaBaseComponent::setLuaRoot;
  [[nodiscard]] std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    if (auto result = m_storage.contextSetup(); !result)
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
    auto storage = m_storage.readStorage();
    auto shutdown = m_storage.finishShutdown(std::move(storage), LuaBaseComponent::contextShutdown());
    return LuaScriptUpdate::finishShutdown(std::move(stopped), std::move(shutdown));
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
  LuaScriptMovement m_movement;
};

// Status-effect scripts retain actor controls without allocating script storage.
class LuaStatusScript final : public LuaBaseComponent {
public:
  explicit LuaStatusScript(Root& root) : LuaBaseComponent(root), m_world(*this), m_update(*this), m_messages(*this), m_movement(*this) {}
  ~LuaStatusScript() override { m_update.destroy(); }
  [[nodiscard]] std::expected<void, StarException> init(World* world) { return m_world.init(world); }
  [[nodiscard]] std::expected<void, StarException> uninit() { return m_world.uninit(); }
  unsigned updateDelta() const { return m_update.updateDelta(); }
  float updateDt(float dt) const { return m_update.updateDt(dt); }
  float updateDt() const { return m_update.updateDt(); }
  void setUpdateDelta(unsigned delta) { m_update.setUpdateDelta(delta); }
  bool updateReady() const { return m_update.updateReady(); }
  template <typename Ret = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Ret>, StarException> update(V&&... args) {
    m_movement.beforeUpdate(m_update.updateReady());
    auto result = m_update.template update<Ret>(std::forward<V>(args)...);
    auto controls = m_movement.performControls();
    if (!controls) {
      if (!result)
        return std::unexpected(controls.error().withContext(strf("Original Lua movement update failure: {}", outputException(result.error(), true))));
      return std::unexpected(std::move(controls.error()));
    }
    return result;
  }
  RpcPromise<Json> threadPassMessage(String const& thread, String const& message, JsonArray const& args) override {
    return m_update.threadPassMessage(thread, message, args);
  }
  [[nodiscard]] std::expected<std::optional<ChainableJsonMessageResponse>, StarException> handleMessage(String const& message, bool localMessage, JsonArray const& args = {}) {
    return m_messages.handleMessage(message, localMessage, args);
  }
  [[nodiscard]] std::expected<void, StarException> addActorMovementCallbacks(ActorMovementController* controller) { return m_movement.addActorMovementCallbacks(controller); }
  [[nodiscard]] std::expected<void, StarException> removeActorMovementCallbacks() { return m_movement.removeActorMovementCallbacks(); }
  bool autoClearControls() const { return m_movement.autoClearControls(); }
  void setAutoClearControls(bool enabled) { m_movement.setAutoClearControls(enabled); }
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
  LuaScriptMovement m_movement;
};

}

export module star.lua_actor_movement_component;

export namespace Star {
  using ::Star::LuaScriptMovement;
  using ::Star::LuaActorScript;
  using ::Star::LuaActionScript;
  using ::Star::LuaStatusScript;
}
