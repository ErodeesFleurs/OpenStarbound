#pragma once

#include "StarAnchorableEntity.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarGameTimers.hpp"
#include "StarMovementController.hpp"
#include "StarPlatformerAStarTypes.hpp"

import std;

namespace Star {

using ActorMovementControllerException = ExceptionDerived<"ActorMovementControllerException", MovementControllerException>;

class PathController;
namespace PlatformerAStar {
class PathFinder;
}

struct ActorJumpProfile {
  ActorJumpProfile();
  ActorJumpProfile(Json const& config);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto merge(ActorJumpProfile const& rhs) const -> ActorJumpProfile;

  std::optional<float> jumpSpeed;
  std::optional<float> jumpControlForce;
  std::optional<float> jumpInitialPercentage;

  // If this is greater than 0.0, jump hold time is limited by this factor.
  std::optional<float> jumpHoldTime;
  // If this is greater than 0.0, then the total jump time for *all jumps in a
  // multi jump set* is limited by this factor.
  std::optional<float> jumpTotalHoldTime;

  std::optional<bool> multiJump;
  std::optional<float> reJumpDelay;
  std::optional<bool> autoJump;
  std::optional<bool> collisionCancelled;
};

auto operator>>(DataStream& ds, ActorJumpProfile& movementParameters) -> DataStream&;
auto operator<<(DataStream& ds, ActorJumpProfile const& movementParameters) -> DataStream&;

// A not-quite superset of MovementParameters, with some fields from
// MovementParameters ignored because they make no sense, and other fields
// expanded out to different cases based on Actor specific things.
struct ActorMovementParameters {
  // Load sensible defaults from a config file.
  static auto sensibleDefaults() -> ActorMovementParameters;

  // Construct parameters from config with only those specified in the config
  // set, if any.
  explicit ActorMovementParameters(Json const& config = Json());

  [[nodiscard]] auto toJson() const -> Json;

  // Merge the given set of movement parameters on top of this one, with any
  // set parameters in rhs overwriting the ones in this set.
  [[nodiscard]] auto merge(ActorMovementParameters const& rhs) const -> ActorMovementParameters;

  std::optional<float> mass;
  std::optional<float> gravityMultiplier;
  std::optional<float> liquidBuoyancy;
  std::optional<float> airBuoyancy;
  std::optional<float> bounceFactor;
  std::optional<bool> stopOnFirstBounce;
  std::optional<bool> enableSurfaceSlopeCorrection;
  std::optional<float> slopeSlidingFactor;
  std::optional<float> maxMovementPerStep;
  std::optional<float> maximumCorrection;
  std::optional<float> speedLimit;

  std::optional<PolyF> standingPoly;
  std::optional<PolyF> crouchingPoly;

  std::optional<bool> stickyCollision;
  std::optional<float> stickyForce;

  std::optional<float> walkSpeed;
  std::optional<float> runSpeed;
  std::optional<float> flySpeed;

  std::optional<float> airFriction;
  std::optional<float> liquidFriction;

  std::optional<float> minimumLiquidPercentage;
  std::optional<float> liquidImpedance;

  std::optional<float> normalGroundFriction;
  std::optional<float> ambulatingGroundFriction;

  std::optional<float> groundForce;
  std::optional<float> airForce;
  std::optional<float> liquidForce;

  ActorJumpProfile airJumpProfile;
  ActorJumpProfile liquidJumpProfile;

  std::optional<float> fallStatusSpeedMin;
  std::optional<int> fallThroughSustainFrames;
  std::optional<float> maximumPlatformCorrection;
  std::optional<float> maximumPlatformCorrectionVelocityFactor;

  std::optional<StringSet> physicsEffectCategories;

  std::optional<float> groundMovementMinimumSustain;
  std::optional<float> groundMovementMaximumSustain;
  std::optional<float> groundMovementCheckDistance;

  std::optional<bool> collisionEnabled;
  std::optional<bool> frictionEnabled;
  std::optional<bool> gravityEnabled;

  std::optional<float> pathExploreRate;
};

auto operator>>(DataStream& ds, ActorMovementParameters& movementParameters) -> DataStream&;
auto operator<<(DataStream& ds, ActorMovementParameters const& movementParameters) -> DataStream&;

// A set of normalized values that act as "modifiers" or "bonuses" to movement,
// and can be combined sensibly.  A modifier of 0.0 represents a 0% change, a
// modifier of 0.2 represents a 20% increase, and a modifier of -0.2 represents
// a 20% decrease.  Also includes some flags that disable functionality
// combined with logical OR.
struct ActorMovementModifiers {
  explicit ActorMovementModifiers(Json const& config = Json());

  [[nodiscard]] auto toJson() const -> Json;

  // Combines each modifier value through addition.
  [[nodiscard]] auto combine(ActorMovementModifiers const& rhs) const -> ActorMovementModifiers;

  float groundMovementModifier;
  float liquidMovementModifier;
  float speedModifier;
  float airJumpModifier;
  float liquidJumpModifier;

  bool runningSuppressed;
  bool jumpingSuppressed;
  // Suppresses left, right, down, crouch, jump, and fly controls
  bool movementSuppressed;
  bool facingSuppressed;
};

auto operator>>(DataStream& ds, ActorMovementModifiers& movementModifiers) -> DataStream&;
auto operator<<(DataStream& ds, ActorMovementModifiers const& movementModifiers) -> DataStream&;

class ActorMovementController : public virtual MovementController {
public:
  // Constructs an ActorMovementController with parameters loaded from sensible
  // defaults, and the given parameters (if any) applied on top of them.
  explicit ActorMovementController(ActorMovementParameters const& parameters = ActorMovementParameters());

  // Currently active parameters.
  auto baseParameters() const -> ActorMovementParameters const&;

  // Apply any set parameters from the given set on top of the current set.
  void updateBaseParameters(ActorMovementParameters const& parameters);

  // Reset the parameters from the sensible defaults, and apply the given
  // parameters (if any) on top of them.
  void resetBaseParameters(ActorMovementParameters const& parameters = ActorMovementParameters());

  // Currently active modifiers.
  auto baseModifiers() const -> ActorMovementModifiers const&;

  // Combine the given modifiers with the already active modifiers.
  void updateBaseModifiers(ActorMovementModifiers const& modifiers);

  // Reset all modifiers to the given values
  void resetBaseModifiers(ActorMovementModifiers const& modifiers = ActorMovementModifiers());

  // Stores and loads position, velocity, rotation, movingDirection,
  // facingDirection, and crouching
  auto storeState() const -> Json;
  void loadState(Json const& state);

  // Optionaly anchor this ActorMovementController to the given
  // AnchorableEntity.  position, rotation, and facing direction will be set
  // based on the entity anchor alone every tick, and on slaved
  // ActorMovementControllers it will be updated based on the actual slave-side
  // AnchorableEntity state.
  void setAnchorState(EntityAnchorState anchorState);
  void resetAnchorState();
  auto anchorState() const -> std::optional<EntityAnchorState>;
  auto entityAnchor() const -> ConstPtr<EntityAnchor>;

  // ActorMovementController position and rotation honor the entity anchor, if
  // an anchor is set.
  auto position() const -> Vec2F override;
  auto rotation() const -> float override;

  auto walking() const -> bool;
  auto running() const -> bool;
  auto movingDirection() const -> Direction;
  auto facingDirection() const -> Direction;
  auto crouching() const -> bool;
  auto flying() const -> bool;
  auto falling() const -> bool;
  auto canJump() const -> bool;
  auto jumping() const -> bool;
  // Slightly different than onGround, in that this is sustained for a few
  // extra frames of movement before it becomes false.
  auto groundMovement() const -> bool;
  auto liquidMovement() const -> bool;
  auto pathfinding() const -> bool;

  // Basic direct physics controls that can be called multiple times per
  // update and will be combined.
  void controlRotation(float rotationRate);
  void controlAcceleration(Vec2F const& acceleration);
  void controlForce(Vec2F const& force);
  void controlApproachVelocity(Vec2F const& targetVelocity, float maxControlForce);
  void controlApproachVelocityAlongAngle(float angle, float targetVelocity, float maxControlForce, bool positiveOnly = false);
  void controlApproachXVelocity(float targetXVelocity, float maxControlForce);
  void controlApproachYVelocity(float targetYVelocity, float maxControlForce);

  // Apply ActorMovementParameters / ActorMovementModifiers only as long as
  // the controls are active.  Can be called multiple times per update and
  // will be combined.
  void controlParameters(ActorMovementParameters const& parameters);
  void controlModifiers(ActorMovementModifiers const& modifiers);

  // Higher level movement controls that use forces defined in the
  // ActorMovementParameters.  Calling more than once per update will override
  // previous calls.
  void controlMove(Direction direction, bool run = true);
  void controlFace(Direction direction);
  void controlDown();
  void controlCrouch();
  void controlJump(bool jumpEvenIfUnable = false);
  void controlFly(Vec2F const& velocity);

  auto pathMove(Vec2F const& pathPosition, bool run = false, std::optional<PlatformerAStar::Parameters> const& parameters = {}) -> std::optional<std::pair<Vec2F, bool>>;
  auto controlPathMove(Vec2F const& pathPosition, bool run = false, std::optional<PlatformerAStar::Parameters> const& parameters = {}) -> std::optional<std::pair<Vec2F, bool>>;

  // Used for user controller input.
  void setMoveSpeedMultiplier(float multiplier = 1.0f);

  // Clears all control data.
  void clearControls();

  // Integrates the ActorMovementController and applies all
  // the control data and clears it for the next step.
  void tickMaster(float dt);

  void tickSlave(float dt);

private:
  struct ApproachVelocityCommand {
    Vec2F targetVelocity;
    float maxControlForce;
  };

  struct ApproachVelocityAlongAngleCommand {
    float alongAngle;
    float targetVelocity;
    float maxControlForce;
    bool positiveOnly;
  };

  void applyMCParameters(ActorMovementParameters const& parameters);
  void doSetAnchorState(std::optional<EntityAnchorState> anchorState);

  ActorMovementParameters m_baseParameters;
  ActorMovementModifiers m_baseModifiers;

  // State data

  NetElementBool m_walking;
  NetElementBool m_running;
  NetElementEnum<Direction> m_movingDirection;
  NetElementEnum<Direction> m_facingDirection;
  NetElementBool m_crouching;
  NetElementBool m_flying;
  NetElementBool m_falling;
  NetElementBool m_canJump;
  NetElementBool m_jumping;
  NetElementBool m_groundMovement;
  NetElementBool m_liquidMovement;
  NetElementData<std::optional<EntityAnchorState>> m_anchorState;
  ConstPtr<EntityAnchor> m_entityAnchor;

  // Command data

  float m_controlRotationRate;
  Vec2F m_controlAcceleration;
  Vec2F m_controlForce;
  List<ApproachVelocityCommand> m_controlApproachVelocities;
  List<ApproachVelocityAlongAngleCommand> m_controlApproachVelocityAlongAngles;

  std::optional<Direction> m_controlMove;
  std::optional<Direction> m_controlFace;
  bool m_controlRun;
  bool m_controlCrouch;
  bool m_controlDown;
  bool m_controlJump;
  bool m_controlJumpAnyway;

  std::optional<Vec2F> m_controlFly;

  std::optional<std::pair<Vec2F, bool>> m_controlPathMove;
  std::optional<std::pair<Vec2F, bool>> m_pathMoveResult;
  Ptr<PathController> m_pathController;

  ActorMovementParameters m_controlParameters;
  ActorMovementModifiers m_controlModifiers;

  // Internal state data

  int m_fallThroughSustain;
  bool m_lastControlJump;
  bool m_lastControlDown;
  float m_moveSpeedMultiplier;

  GameTimer m_reJumpTimer;
  std::optional<GameTimer> m_jumpHoldTimer;
  GameTimer m_groundMovementSustainTimer;

  // Target horizontal velocity for walking / running
  float m_targetHorizontalAmbulatingVelocity;
};

class PathController {
public:
  PathController(World* world);

  auto parameters() -> PlatformerAStar::Parameters const&;
  void setParameters(PlatformerAStar::Parameters const& parameters);
  void reset();
  [[nodiscard]] auto pathfinding() const -> bool;
  [[nodiscard]] auto targetPosition() const -> std::optional<Vec2F>;
  [[nodiscard]] auto facing() const -> std::optional<Direction>;
  [[nodiscard]] auto curAction() const -> std::optional<PlatformerAStar::Action>;

  // return true for reaching goal, false for failing to find path, nothing for running
  auto findPath(ActorMovementController& movementController, Vec2F const& targetPosition) -> std::optional<bool>;
  auto move(ActorMovementController& movementController, ActorMovementParameters const& parameters, ActorMovementModifiers const& modifiers, bool run, float dt) -> std::optional<bool>;

private:
  auto validateEdge(ActorMovementController& movementController, PlatformerAStar::Edge const& edge) -> bool;
  auto movingCollision(ActorMovementController& movementController, PolyF const& collisionPoly) -> bool;

private:
  [[nodiscard]] auto onGround(ActorMovementController const& movementController, Vec2F const& position, CollisionSet const& collisionSet) const -> bool;

  World* m_world;
  PlatformerAStar::Parameters m_parameters;

  std::optional<Vec2F> m_startPosition;
  std::optional<Vec2F> m_targetPosition;
  Ptr<PlatformerAStar::PathFinder> m_pathFinder;

  std::optional<Direction> m_controlFace;

  size_t m_edgeIndex;
  float m_edgeTimer;
  std::optional<PlatformerAStar::Path> m_path;
};

}// namespace Star
