module;
import std;
import star.lua;
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

namespace Star {

class Root;

// Wraps a LuaUpdatableComponent to handle the particularly tricky case of
// maintaining ActorMovementController controls when we do not call the script
// update every tick.
template <typename Base>
class LuaActorMovementComponent : public Base {
public:
  explicit LuaActorMovementComponent(Root& root);

  void addActorMovementCallbacks(ActorMovementController* actorMovementController);
  void removeActorMovementCallbacks();

  // If true, then the controls are automatically cleared on script update.
  // Defaults to true
  bool autoClearControls() const;
  void setAutoClearControls(bool autoClearControls);

  // Updates the lua script component and applies held controls.  If no script
  // update is scheduled this tick, then the controls from the last update will
  // be held and not cleared.  If a script update is scheduled this tick, then
  // the controls will be cleared only if autoClearControls is set to true.
  template <typename Ret = LuaValue, typename... V>
  std::optional<Ret> update(V&&... args);

private:
  void performControls();
  void clearControls();

  ActorMovementController* m_movementController;
  bool m_autoClearControls = false;

  float m_controlRotation;
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

template <typename Base>
LuaActorMovementComponent<Base>::LuaActorMovementComponent(Root& root)
  : Base(root), m_autoClearControls(true),
    m_controlRotation(0.0f),
    m_controlDown(false),
    m_controlCrouch(false),
    m_controlHoldJump(false) {}

template <typename Base>
void LuaActorMovementComponent<Base>::addActorMovementCallbacks(ActorMovementController* actorMovementController) {
  m_movementController = actorMovementController;
  if (m_movementController) {
    // inherit base mcontroller callbacks so that we have some consistency and don't need to have duplicate definitions here
    LuaCallbacks callbacks = LuaBindings::makeMovementControllerCallbacks(m_movementController);

    // replace callbacks that need to set a value here that probably shouldn't be done with virtual function overrides
    callbacks.removeCallback("setVelocity");
    callbacks.registerCallback("setVelocity", [this](Vec2F const& vel) {
      m_resetPathMove = true;
      m_movementController->setVelocity(vel);
    });
    callbacks.removeCallback("setXVelocity");
    callbacks.registerCallback("setXVelocity", [this](float xVel) {
        m_resetPathMove = true;
        m_movementController->setXVelocity(xVel);
      });
    callbacks.removeCallback("setYVelocity");
    callbacks.registerCallback("setYVelocity", [this](float yVel) {
        m_resetPathMove = true;
        m_movementController->setYVelocity(yVel);
      });
    callbacks.removeCallback("addMomentum");
    callbacks.registerCallback("addMomentum", [this](Vec2F const& momentum) {
        m_resetPathMove = true;
        m_movementController->addMomentum(momentum);
      });

    callbacks.removeCallback("setRotation");
    callbacks.registerCallback("setRotation", [this](float rotation) {
        m_resetPathMove = true;
        m_movementController->setRotation(rotation);
      });

    callbacks.removeCallback("setRotation");
    callbacks.registerCallback("setRotation", [this](float rotation) {
        m_resetPathMove = true;
        m_movementController->setRotation(rotation);
      });

    // The actual actor specific callbacks
    callbacks.registerCallback("setAnchorState", [this](EntityId anchorableEntity, std::size_t anchorPosition) {
        m_movementController->setAnchorState({anchorableEntity, anchorPosition});
      });

    callbacks.registerCallback("resetAnchorState", [this]() {
        m_movementController->resetAnchorState();
      });

    callbacks.registerCallback("anchorState", [this]() {
        if (auto anchorState = m_movementController->anchorState())
          return LuaVariadic<LuaValue>{LuaInt(anchorState->entityId), LuaInt(anchorState->positionIndex)};
        return LuaVariadic<LuaValue>();
      });

    callbacks.registerCallback("baseParameters", [this]() {
        return m_movementController->baseParameters();
      });

    callbacks.registerCallback("walking", [this]() {
        return m_movementController->walking();
      });

    callbacks.registerCallback("running", [this]() {
        return m_movementController->running();
      });

    callbacks.registerCallback("movingDirection", [this]() {
        return numericalDirection(m_movementController->movingDirection());
      });

    callbacks.registerCallback("facingDirection", [this]() {
        return numericalDirection(m_movementController->facingDirection());
      });

    callbacks.registerCallback("crouching", [this]() {
        return m_movementController->crouching();
      });

    callbacks.registerCallback("flying", [this]() {
        return m_movementController->flying();
      });

    callbacks.registerCallback("falling", [this]() {
        return m_movementController->falling();
      });

    callbacks.registerCallback("canJump", [this]() {
        return m_movementController->canJump();
      });
    callbacks.registerCallback("jumping", [this]() {
        return m_movementController->jumping();
      });

    callbacks.registerCallback("groundMovement", [this]() {
        return m_movementController->groundMovement();
      });

    callbacks.registerCallback("liquidMovement", [this]() {
        return m_movementController->liquidMovement();
      });

    callbacks.registerCallback("controlRotation", [this](float rotation) {
        m_controlRotation += rotation;
      });

    callbacks.registerCallback("controlAcceleration", [this](Vec2F const& accel) {
        m_controlAcceleration += accel;
      });

    callbacks.registerCallback("controlForce", [this](Vec2F const& force) {
        m_controlForce += force;
      });

    callbacks.registerCallback("controlApproachVelocity", [this](Vec2F const& arg1, float arg2) {
        m_controlApproachVelocity.emplace(std::make_tuple(arg1, arg2));
      });

    callbacks.registerCallback("controlApproachVelocityAlongAngle", [this](float angle, float targetVelocity, float maxControlForce, bool positiveOnly) {
        m_controlApproachVelocityAlongAngle.emplace(std::make_tuple(angle, targetVelocity, maxControlForce, positiveOnly));
      });

    callbacks.registerCallback("controlApproachXVelocity", [this](float targetXVelocity, float maxControlForce) {
        m_controlApproachVelocityAlongAngle.emplace(std::make_tuple(0.0f, targetXVelocity, maxControlForce, false));
      });

    callbacks.registerCallback("controlApproachYVelocity", [this](float targetYVelocity, float maxControlForce) {
        m_controlApproachVelocityAlongAngle.emplace(
            std::make_tuple(std::numbers::pi / 2.0f, targetYVelocity, maxControlForce, false));
      });

    callbacks.registerCallback("controlParameters", [this](ActorMovementParameters const& arg1) {
        m_controlParameters = m_controlParameters.value_or(ActorMovementParameters{}).merge(arg1);
      });

    callbacks.registerCallback("controlModifiers", [this](ActorMovementModifiers const& arg1) {
        m_controlModifiers = m_controlModifiers.value_or(ActorMovementModifiers{}).combine(arg1);
      });

    callbacks.registerCallback("controlMove", [this](std::optional<float> const& arg1, std::optional<bool> const& arg2) {
        if (auto direction = directionOf(arg1.value_or(float{})))
          m_controlMove.emplace(std::make_tuple(*direction, arg2.value_or(true)));
      });

    callbacks.registerCallback("controlFace", [this](std::optional<float> const& arg1) {
        if (auto direction = directionOf(arg1.value_or(float{})))
          m_controlFace = *direction;
      });

    callbacks.registerCallback("controlDown", [this]() {
        m_controlDown = true;
      });

    callbacks.registerCallback("controlCrouch", [this]() {
        m_controlCrouch = true;
      });

    callbacks.registerCallback("controlJump", [this](bool arg1) {
        m_controlJump = arg1;
      });

    callbacks.registerCallback("controlHoldJump", [this]() {
        m_controlHoldJump = true;
      });

    callbacks.registerCallback("controlFly", [this](Vec2F const& arg1) {
        m_controlFly = arg1;
      });

    callbacks.registerCallback("controlPathMove", [this](Vec2F const& position, std::optional<bool> run, std::optional<PlatformerAStar::Parameters> parameters) -> std::optional<bool> {
        if (m_pathMoveResult && m_pathMoveResult->first == position) {
          return std::exchange(m_pathMoveResult, std::nullopt).transform([](std::pair<Vec2F, bool> const& p) { return p.second; });
        } else {
          m_pathMoveResult.reset();
          auto result = m_movementController->pathMove(position, run.value_or(false), parameters);
          if (!result.has_value())
            m_controlPathMove = std::pair<Vec2F, bool>(position, run.value_or(false));
          return result.transform([](std::pair<Vec2F, bool> const& p) { return p.second; });
        }
      });

    callbacks.registerCallback("pathfinding", [this]() -> bool {
        return m_movementController->pathfinding();
      });

    callbacks.registerCallbackWithSignature<bool>("autoClearControls", std::bind_front(&LuaActorMovementComponent::autoClearControls, this));
    callbacks.registerCallbackWithSignature<void, bool>("setAutoClearControls", std::bind_front(&LuaActorMovementComponent::setAutoClearControls, this));
    callbacks.registerCallbackWithSignature<void>("clearControls", std::bind_front(&LuaActorMovementComponent::clearControls, this));

    Base::addCallbacks("mcontroller", callbacks);

  } else {
    Base::removeCallbacks("mcontroller");
  }
}

template <typename Base>
void LuaActorMovementComponent<Base>::removeActorMovementCallbacks() {
  addActorMovementCallbacks(nullptr);
}

template <typename Base>
bool LuaActorMovementComponent<Base>::autoClearControls() const {
  return m_autoClearControls;
}

template <typename Base>
void LuaActorMovementComponent<Base>::setAutoClearControls(bool autoClearControls) {
  m_autoClearControls = autoClearControls;
}

template <typename Base>
template <typename Ret, typename... V>
std::optional<Ret> LuaActorMovementComponent<Base>::update(V&&... args) {
  if (Base::updateReady()) {
    if (m_autoClearControls)
      clearControls();
  }
  std::optional<Ret> ret = Base::template update<Ret>(std::forward<V>(args)...);
  performControls();
  return ret;
}

template <typename Base>
void LuaActorMovementComponent<Base>::performControls() {
  if (m_movementController) {
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

    // some action was taken that has priority over pathing, setting position or velocity
    if (m_resetPathMove)
      m_controlPathMove = {};
    if (m_controlPathMove && !m_pathMoveResult.has_value())
      m_pathMoveResult = m_movementController->controlPathMove(m_controlPathMove->first, m_controlPathMove->second);
  }
}

template <typename Base>
void LuaActorMovementComponent<Base>::clearControls() {
  m_controlRotation = {};
  m_controlAcceleration = {};
  m_controlForce = {};
  m_controlApproachVelocity = {};
  m_controlApproachVelocityAlongAngle = {};
  m_controlParameters = {};
  m_controlModifiers = {};
  m_controlMove = {};
  m_controlFace = {};
  m_controlDown = {};
  m_controlCrouch = {};
  m_controlJump = {};
  m_controlHoldJump = {};
  m_controlFly = {};

  m_resetPathMove = false;
  // Clear path move result one clear after controlPathMove is no longer called
  // to keep the result available for the following update
  if (!m_controlPathMove.has_value())
    m_pathMoveResult = {};
  m_controlPathMove = {};
}
}

export module star.lua_actor_movement_component;

export namespace Star {
  using ::Star::LuaActorMovementComponent;
}
