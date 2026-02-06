#pragma once

#include <optional>

#include "StarJson.hpp"

#include "StarWorld.hpp"
#include "StarPhysicsEntity.hpp"

namespace Star {

STAR_EXCEPTION(MovementControllerException, StarException);

STAR_CLASS(MovementController);

// List of all movement parameters that define a specific sort of movable
// object.  Each parameter is optional so that this structure can be used to
// selectively merge a specific set of parameters on top of another.
struct MovementParameters {
  // Load sensible defaults from a config file.
  static MovementParameters sensibleDefaults();

  // Construct parameters from config with only those specified in the config
  // set, if any.
  MovementParameters(Json const& config = Json());

  // Merge the given set of movement parameters on top of this one, with any
  // set parameters in rhs overwriting the ones in this set.
  MovementParameters merge(MovementParameters const& rhs) const;

  Json toJson() const;

  std::optional<float> mass;
  std::optional<float> gravityMultiplier;
  std::optional<float> liquidBuoyancy;
  std::optional<float> airBuoyancy;
  std::optional<float> bounceFactor;
  // If set to true, during an update that has more than one internal movement
  // step, the movement will stop on the first bounce.
  std::optional<bool> stopOnFirstBounce;
  // Cheat when sliding on the ground, by trying to correct upwards before
  // other directions (within a set limit).  Allows smooth sliding along
  // horizontal ground without losing horizontal speed.
  std::optional<bool> enableSurfaceSlopeCorrection;
  std::optional<float> slopeSlidingFactor;
  std::optional<float> maxMovementPerStep;
  std::optional<float> maximumCorrection;
  std::optional<float> speedLimit;
  std::optional<float> discontinuityThreshold;

  std::optional<PolyF> collisionPoly;

  std::optional<bool> stickyCollision;
  std::optional<float> stickyForce;

  std::optional<float> airFriction;
  std::optional<float> liquidFriction;
  std::optional<float> groundFriction;

  std::optional<bool> collisionEnabled;
  std::optional<bool> frictionEnabled;
  std::optional<bool> gravityEnabled;

  std::optional<bool> ignorePlatformCollision;
  std::optional<float> maximumPlatformCorrection;
  std::optional<float> maximumPlatformCorrectionVelocityFactor;

  std::optional<StringSet> physicsEffectCategories;

  std::optional<int> restDuration;
};

DataStream& operator>>(DataStream& ds, MovementParameters& movementParameters);
DataStream& operator<<(DataStream& ds, MovementParameters const& movementParameters);

class MovementController : public NetElementGroup {
public:
  // Constructs a MovementController with parameters loaded from sensible
  // defaults, and the given parameters (if any) applied on top of them.
  explicit MovementController(MovementParameters const& parameters = MovementParameters());

  MovementParameters const& parameters() const;

  // Apply any set parameters from the given set on top of the current set.
  void applyParameters(MovementParameters const& parameters);

  // Reset the parameters from the sensible defaults, and apply the given
  // parameters (if any) on top of them.
  void resetParameters(MovementParameters const& parameters = MovementParameters());

  // Stores and loads position, velocity, and rotation.
  Json storeState() const;
  void loadState(Json const& state);

  // Currently active mass parameter
  float mass() const;

  // Currently active collisionPoly parameter
  PolyF const& collisionPoly() const;
  void setCollisionPoly(PolyF const& poly);

  virtual Vec2F position() const;
  float xPosition() const;
  float yPosition() const;

  Vec2F velocity() const;
  float xVelocity() const;
  float yVelocity() const;

  virtual float rotation() const;

  // CollisionPoly rotated and translated by position
  PolyF collisionBody() const;

  // Gets the bounding box of the collisionPoly() rotated by current rotation,
  // but not translated into world space
  RectF localBoundBox() const;

  // Shorthand for getting the bound box of the current collisionBody()
  RectF collisionBoundBox() const;

  // Is the collision body colliding with any collision geometry.
  bool isColliding() const;
  // Is the collision body colliding with special "Null" collision blocks.
  bool isNullColliding() const;

  // Is the body currently stuck in an un-solvable collision.
  bool isCollisionStuck() const;

  // If this body is sticking, this is the angle toward the surface it's stuck to
  std::optional<float> stickingDirection() const;

  // From 0.0 to 1.0, the amount of the collision body (or if the collision
  // body is null, just the center position) that is in liquid.
  float liquidPercentage() const;

  // Returns the liquid that the body is most in, if any
  LiquidId liquidId() const;

  bool onGround() const;
  bool zeroG() const;

  bool atWorldLimit(bool bottomOnly = false) const;

  void setPosition(Vec2F position);
  void setXPosition(float xPosition);
  void setYPosition(float yPosition);

  void translate(Vec2F const& direction);

  void setVelocity(Vec2F velocity);
  void setXVelocity(float xVelocity);
  void setYVelocity(float yVelocity);

  void addMomentum(Vec2F const& momentum);

  void setRotation(float angle);

  // Apply one timestep of rotation.
  void rotate(float rotationRate);

  // Apply one timestep of acceleration.
  void accelerate(Vec2F const& acceleration);

  // Apply one timestep of force.
  void force(Vec2F const& force);

  // Apply up to the maxControlForce of force to approach the given velocity.
  void approachVelocity(Vec2F const& targetVelocity, float maxControlForce);

  // Approach a velocity in the given angle, ignoring the component of velocity
  // normal to that angle.  If positiveOnly is true, then only approaches the
  // velocity by applying force in the direction of the given angle, never
  // opposite it, so avoids slowing down.
  void approachVelocityAlongAngle(float angle, float targetVelocity, float maxControlForce, bool positiveOnly = false);

  // Shorthand for approachVelocityAlongAngle with 0 and pi/2.
  void approachXVelocity(float targetXVelocity, float maxControlForce);
  void approachYVelocity(float targetYVelocity, float maxControlForce);

  void init(World* world);
  void uninit();

  // Stores dt value for Lua calls.
  void setTimestep(float dt);

  // Integrates the ActorMovementController one GlobalTimestep and applies all
  // forces.
  void tickMaster(float dt);

  // Does not integrate, only tracks master state and updates non-networked
  // fields based on local data
  void tickSlave(float dt);

  void setIgnorePhysicsEntities(Set<EntityId> ignorePhysicsEntities);
  // iterate over all physics entity collision polys in the region, iteration stops if the callback returns false
  void forEachMovingCollision(RectF const& region, function<bool(MovingCollisionId, PhysicsMovingCollision, PolyF, RectF)> callback);

protected:
  // forces the movement controller onGround status, used when manually controlling movement outside the movement controller
  void updateForceRegions(float dt);
  void updateLiquidPercentage();
  void setOnGround(bool onGround);

  // whether force regions were applied in the last update
  bool appliedForceRegion() const;
  // The collision correction applied during the most recent update, if any.
  Vec2F collisionCorrection() const;
  // Horizontal slope of the ground the collision body has collided with, if
  // any.
  Vec2F surfaceSlope() const;
  // Velocity of the surface that the body is resting on, if any
  Vec2F surfaceVelocity() const;

  World* world();

private:
  struct CollisionResult {
    Vec2F movement;
    Vec2F correction;
    std::optional<MovingCollisionId> surfaceMovingCollisionId;
    bool isStuck;
    bool onGround;
    Vec2F groundSlope;
    CollisionKind collisionKind;
  };

  struct CollisionSeparation {
    Vec2F correction;
    bool solutionFound;
    std::optional<MovingCollisionId> movingCollisionId;
    CollisionKind collisionKind;
  };

  struct CollisionPoly {
    PolyF poly;
    RectF polyBounds;
    Vec2F sortPosition;
    std::optional<MovingCollisionId> movingCollisionId;
    CollisionKind collisionKind;
    float sortDistance;
  };

  static CollisionKind maxOrNullCollision(CollisionKind a, CollisionKind b);
  static CollisionResult collisionMove(List<CollisionPoly>& collisionPolys, PolyF const& body, Vec2F const& movement,
      bool ignorePlatforms, bool enableSurfaceSlopeCorrection, float maximumCorrection, float maximumPlatformCorrection, Vec2F sortCenter, float dt);
  static CollisionSeparation collisionSeparate(List<CollisionPoly>& collisionPolys, PolyF const& poly,
      bool ignorePlatforms, float maximumPlatformCorrection, Vec2F const& sortCenter, bool upward, float separationTolerance);

  void updateParameters(MovementParameters parameters);
  void updatePositionInterpolators();

  void queryCollisions(RectF const& region);

  float gravity();

  MovementParameters m_parameters;

  World* m_world;

  Set<EntityId> m_ignorePhysicsEntities;

  NetElementData<PolyF> m_collisionPoly;
  NetElementFloat m_mass;
  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;
  NetElementFloat m_xVelocity;
  NetElementFloat m_yVelocity;
  NetElementFloat m_rotation;

  NetElementBool m_colliding;
  NetElementBool m_collisionStuck;
  NetElementBool m_nullColliding;
  NetElementData<std::optional<float>> m_stickingDirection;
  NetElementBool m_onGround;
  NetElementBool m_zeroG;

  float m_liquidPercentage;
  LiquidId m_liquidId;

  NetElementData<std::optional<MovingCollisionId>> m_surfaceMovingCollision;
  NetElementFloat m_xRelativeSurfaceMovingCollisionPosition;
  NetElementFloat m_yRelativeSurfaceMovingCollisionPosition;

  bool m_appliedForceRegion;
  Vec2F m_collisionCorrection;
  Vec2F m_surfaceSlope;
  Vec2F m_surfaceMovingCollisionPosition;
  Vec2F m_surfaceVelocity;
  Vec2F m_environmentVelocity;

  bool m_resting;
  int m_restTicks;
  float m_timeStep;

  List<CollisionPoly> m_workingCollisions;
  List<PolyF> m_collisionBuffers;
};

}
