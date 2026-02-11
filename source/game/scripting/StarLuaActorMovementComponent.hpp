#pragma once

#include "StarActorMovementController.hpp"
#include "StarMovementControllerLuaBindings.hpp"

import std;

namespace Star {

// Wraps a LuaUpdatableComponent to handle the particularly tricky case of
// maintaining ActorMovementController controls when we do not call the script
// update every tick.
template <typename Base>
class LuaActorMovementComponent : public Base {
public:
  LuaActorMovementComponent();

  void addActorMovementCallbacks(ActorMovementController* actorMovementController);
  void removeActorMovementCallbacks();

  // If true, then the controls are automatically cleared on script update.
  // Defaults to true
  [[nodiscard]] auto autoClearControls() const -> bool;
  void setAutoClearControls(bool autoClearControls);

  // Updates the lua script component and applies held controls.  If no script
  // update is scheduled this tick, then the controls from the last update will
  // be held and not cleared.  If a script update is scheduled this tick, then
  // the controls will be cleared only if autoClearControls is set to true.
  template <typename Ret = LuaValue, typename... V>
  auto update(V&&... args) -> std::optional<Ret>;

private:
  void performControls();
  void clearControls();

  ActorMovementController* m_movementController;
  bool m_autoClearControls{};

  float m_controlRotation{};
  Vec2F m_controlAcceleration;
  Vec2F m_controlForce;
  std::optional<std::tuple<Vec2F, float>> m_controlApproachVelocity;
  std::optional<std::tuple<float, float, float, bool>> m_controlApproachVelocityAlongAngle;
  std::optional<ActorMovementParameters> m_controlParameters;
  std::optional<ActorMovementModifiers> m_controlModifiers;
  std::optional<std::tuple<Direction, bool>> m_controlMove;
  std::optional<Direction> m_controlFace;
  bool m_controlDown{};
  bool m_controlCrouch{};
  std::optional<bool> m_controlJump;
  bool m_controlHoldJump{};
  std::optional<Vec2F> m_controlFly;

  bool m_resetPathMove;
  std::optional<std::pair<Vec2F, bool>> m_controlPathMove;
  std::optional<std::pair<Vec2F, bool>> m_pathMoveResult;
};

template <typename Base>
LuaActorMovementComponent<Base>::LuaActorMovementComponent() = default;

template <typename Base>
void LuaActorMovementComponent<Base>::addActorMovementCallbacks(ActorMovementController* actorMovementController) {
  m_movementController = actorMovementController;
  if (m_movementController) {
    // inherit base mcontroller callbacks so that we have some consistency and don't need to have duplicate definitions here
    LuaCallbacks callbacks = LuaBindings::makeMovementControllerCallbacks(m_movementController);

    // replace callbacks that need to set a value here that probably shouldn't be done with virtual function overrides
    callbacks.removeCallback("setVelocity");
    callbacks.registerCallback("setVelocity", [this](Vec2F const& vel) -> auto {
      m_resetPathMove = true;
      m_movementController->setVelocity(vel);
    });
    callbacks.removeCallback("setXVelocity");
    callbacks.registerCallback("setXVelocity", [this](float xVel) -> auto {
      m_resetPathMove = true;
      m_movementController->setXVelocity(xVel);
    });
    callbacks.removeCallback("setYVelocity");
    callbacks.registerCallback("setYVelocity", [this](float yVel) -> auto {
      m_resetPathMove = true;
      m_movementController->setYVelocity(yVel);
    });
    callbacks.removeCallback("addMomentum");
    callbacks.registerCallback("addMomentum", [this](Vec2F const& momentum) -> auto {
      m_resetPathMove = true;
      m_movementController->addMomentum(momentum);
    });

    callbacks.removeCallback("setRotation");
    callbacks.registerCallback("setRotation", [this](float rotation) -> auto {
      m_resetPathMove = true;
      m_movementController->setRotation(rotation);
    });

    callbacks.removeCallback("setRotation");
    callbacks.registerCallback("setRotation", [this](float rotation) -> auto {
      m_resetPathMove = true;
      m_movementController->setRotation(rotation);
    });

    // The actual actor specific callbacks
    callbacks.registerCallback("setAnchorState", [this](EntityId anchorableEntity, size_t anchorPosition) -> auto {
      m_movementController->setAnchorState({.entityId = anchorableEntity, .positionIndex = anchorPosition});
    });

    callbacks.registerCallback("resetAnchorState", [this]() -> auto {
      m_movementController->resetAnchorState();
    });

    callbacks.registerCallback("anchorState", [this]() -> auto {
      if (auto anchorState = m_movementController->anchorState())
        return LuaVariadic<LuaValue>{LuaInt(anchorState->entityId), LuaInt(anchorState->positionIndex)};
      return LuaVariadic<LuaValue>();
    });

    callbacks.registerCallback("baseParameters", [this]() -> auto {
      return m_movementController->baseParameters();
    });

    callbacks.registerCallback("walking", [this]() -> auto {
      return m_movementController->walking();
    });

    callbacks.registerCallback("running", [this]() -> auto {
      return m_movementController->running();
    });

    callbacks.registerCallback("movingDirection", [this]() -> auto {
      return numericalDirection(m_movementController->movingDirection());
    });

    callbacks.registerCallback("facingDirection", [this]() -> auto {
      return numericalDirection(m_movementController->facingDirection());
    });

    callbacks.registerCallback("crouching", [this]() -> auto {
      return m_movementController->crouching();
    });

    callbacks.registerCallback("flying", [this]() -> auto {
      return m_movementController->flying();
    });

    callbacks.registerCallback("falling", [this]() -> auto {
      return m_movementController->falling();
    });

    callbacks.registerCallback("canJump", [this]() -> auto {
      return m_movementController->canJump();
    });

    callbacks.registerCallback("jumping", [this]() -> auto {
      return m_movementController->jumping();
    });

    callbacks.registerCallback("groundMovement", [this]() -> auto {
      return m_movementController->groundMovement();
    });

    callbacks.registerCallback("liquidMovement", [this]() -> auto {
      return m_movementController->liquidMovement();
    });

    callbacks.registerCallback("controlRotation", [this](float rotation) -> auto {
      m_controlRotation += rotation;
    });

    callbacks.registerCallback("controlAcceleration", [this](Vec2F const& accel) -> auto {
      m_controlAcceleration += accel;
    });

    callbacks.registerCallback("controlForce", [this](Vec2F const& force) -> auto {
      m_controlForce += force;
    });

    callbacks.registerCallback("controlApproachVelocity", [this](Vec2F const& arg1, float arg2) -> auto {
      m_controlApproachVelocity = std::make_tuple(arg1, arg2);
    });

    callbacks.registerCallback("controlApproachVelocityAlongAngle", [this](float angle, float targetVelocity, float maxControlForce, bool positiveOnly) -> auto {
      m_controlApproachVelocityAlongAngle = std::make_tuple(angle, targetVelocity, maxControlForce, positiveOnly);
    });

    callbacks.registerCallback("controlApproachXVelocity", [this](float targetXVelocity, float maxControlForce) -> auto {
      m_controlApproachVelocityAlongAngle = std::make_tuple(0.0f, targetXVelocity, maxControlForce, false);
    });

    callbacks.registerCallback("controlApproachYVelocity", [this](float targetYVelocity, float maxControlForce) -> auto {
      m_controlApproachVelocityAlongAngle =
        std::make_tuple(Constants::pi / 2.0f, targetYVelocity, maxControlForce, false);
    });

    callbacks.registerCallback("controlParameters", [this](ActorMovementParameters const& arg1) -> auto {
      m_controlParameters = m_controlParameters.value().merge(arg1);
    });

    callbacks.registerCallback("controlModifiers", [this](ActorMovementModifiers const& arg1) -> auto {
      m_controlModifiers = m_controlModifiers.value().combine(arg1);
    });

    callbacks.registerCallback("controlMove", [this](std::optional<float> const& arg1, std::optional<bool> const& arg2) -> auto {
      if (auto direction = directionOf(arg1.value()))
        m_controlMove = std::make_tuple(*direction, arg2.value_or(true));
    });

    callbacks.registerCallback("controlFace", [this](std::optional<float> const& arg1) -> auto {
      if (auto direction = directionOf(arg1.value()))
        m_controlFace = *direction;
    });

    callbacks.registerCallback("controlDown", [this]() -> auto {
      m_controlDown = true;
    });

    callbacks.registerCallback("controlCrouch", [this]() -> auto {
      m_controlCrouch = true;
    });

    callbacks.registerCallback("controlJump", [this](bool arg1) -> auto {
      m_controlJump = arg1;
    });

    callbacks.registerCallback("controlHoldJump", [this]() -> auto {
      m_controlHoldJump = true;
    });

    callbacks.registerCallback("controlFly", [this](Vec2F const& arg1) -> auto {
      m_controlFly = arg1;
    });

    callbacks.registerCallback("controlPathMove", [this](Vec2F const& position, std::optional<bool> run, std::optional<PlatformerAStar::Parameters> parameters) -> std::optional<bool> {
      if (m_pathMoveResult && m_pathMoveResult->first == position) {
        return take(m_pathMoveResult).transform([](std::pair<Vec2F, bool> const& p) -> auto { return p.second; });
      } else {
        m_pathMoveResult.reset();
        auto result = m_movementController->pathMove(position, run.value_or(false), parameters);
        if (!result)
          m_controlPathMove = std::pair<Vec2F, bool>(position, run.value_or(false));
        return result.transform([](std::pair<Vec2F, bool> const& p) -> auto { return p.second; });
      }
    });

    callbacks.registerCallback("pathfinding", [this]() -> bool {
      return m_movementController->pathfinding();
    });

    callbacks.registerCallbackWithSignature<bool>("autoClearControls", std::bind(&LuaActorMovementComponent::autoClearControls, this));
    callbacks.registerCallbackWithSignature<void, bool>("setAutoClearControls", std::bind(&LuaActorMovementComponent::setAutoClearControls, this, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<void>("clearControls", std::bind(&LuaActorMovementComponent::clearControls, this));

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
auto LuaActorMovementComponent<Base>::autoClearControls() const -> bool {
  return m_autoClearControls;
}

template <typename Base>
void LuaActorMovementComponent<Base>::setAutoClearControls(bool autoClearControls) {
  m_autoClearControls = autoClearControls;
}

template <typename Base>
template <typename Ret, typename... V>
auto LuaActorMovementComponent<Base>::update(V&&... args) -> std::optional<Ret> {
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
      tupleUnpackFunction([this](auto&& PH1, auto&& PH2) -> auto { m_movementController->controlApproachVelocity(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, *m_controlApproachVelocity);
    if (m_controlApproachVelocityAlongAngle)
      tupleUnpackFunction([this](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { m_movementController->controlApproachVelocityAlongAngle(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); }, *m_controlApproachVelocityAlongAngle);
    if (m_controlParameters)
      m_movementController->controlParameters(*m_controlParameters);
    if (m_controlModifiers)
      m_movementController->controlModifiers(*m_controlModifiers);
    if (m_controlMove)
      tupleUnpackFunction([this](auto&& PH1, auto&& PH2) -> auto { m_movementController->controlMove(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, *m_controlMove);
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
      m_controlPathMove = std::nullopt;
    if (m_controlPathMove && !m_pathMoveResult)
      m_pathMoveResult = m_movementController->controlPathMove(m_controlPathMove->first, m_controlPathMove->second);
  }
}

template <typename Base>
void LuaActorMovementComponent<Base>::clearControls() {
  m_controlRotation = {};
  m_controlAcceleration = {};
  m_controlForce = {};
  m_controlApproachVelocity = std::nullopt;
  m_controlApproachVelocityAlongAngle = std::nullopt;
  m_controlParameters = std::nullopt;
  m_controlModifiers = std::nullopt;
  m_controlMove = std::nullopt;
  m_controlFace = std::nullopt;
  m_controlDown = {};
  m_controlCrouch = {};
  m_controlJump = std::nullopt;
  m_controlHoldJump = {};
  m_controlFly = std::nullopt;

  m_resetPathMove = false;
  // Clear path move result one clear after controlPathMove is no longer called
  // to keep the result available for the following update
  if (!m_controlPathMove)
    m_pathMoveResult = std::nullopt;
  m_controlPathMove = std::nullopt;
}
}// namespace Star
