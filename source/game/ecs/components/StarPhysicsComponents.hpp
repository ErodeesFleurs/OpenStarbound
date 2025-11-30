#pragma once

// Physics-related ECS components for OpenStarbound
// These components handle physics, collision, and movement state

#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarCollisionBlock.hpp"
#include "StarGameTypes.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Physics Components
//=============================================================================

// Physical properties for movement
struct PhysicsBodyComponent {
  float mass = 1.0f;
  float gravityMultiplier = 1.0f;
  float liquidBuoyancy = 0.0f;
  float airBuoyancy = 0.0f;
  float bounceFactor = 0.0f;
  float airFriction = 0.0f;
  float liquidFriction = 0.0f;
  float groundFriction = 0.0f;
  bool collisionEnabled = true;
  bool gravityEnabled = true;
  bool frictionEnabled = true;
  bool stickyCollision = false;
  float stickyForce = 0.0f;
  float maxSpeed = 100.0f;
};

// Collision shape and state
struct CollisionComponent {
  PolyF standingPoly = {};
  PolyF crouchingPoly = {};
  CollisionSet collisionSet = DefaultCollisionSet;
  bool onGround = false;
  bool inLiquid = false;
  float liquidPercentage = 0.0f;
  bool collidingWithPlatform = false;
  bool collisionStuck = false;
  
  PolyF const& currentPoly(bool crouching) const {
    return crouching ? crouchingPoly : standingPoly;
  }
};

// Movement state for actor entities
struct MovementStateComponent {
  float walkSpeed = 8.0f;
  float runSpeed = 14.0f;
  float flySpeed = 15.0f;
  Direction facingDirection = Direction::Right;
  Direction movingDirection = Direction::Right;
  bool walking = false;
  bool running = false;
  bool crouching = false;
  bool flying = false;
  bool jumping = false;
  bool falling = false;
  bool groundMovement = false;
  bool liquidMovement = false;
  
  // Jump parameters
  float jumpSpeed = 15.0f;
  float jumpHoldTime = 0.0f;
  float maxJumpHoldTime = 0.3f;
  int jumpsRemaining = 1;
  int maxJumps = 1;
  
  bool canJump() const {
    return (groundMovement || liquidMovement || jumpsRemaining > 0);
  }
  
  void resetJumps() {
    jumpsRemaining = maxJumps;
  }
};

// Movement control input (what the player/AI wants to do)
struct MovementControlComponent {
  Vec2F moveDirection = {};
  bool wantJump = false;
  bool wantCrouch = false;
  bool wantRun = false;
  bool wantFly = false;
  float moveSpeedMultiplier = 1.0f;
};

// Force region for physics effects
struct ForceRegionComponent {
  RectF region = {};
  Vec2F force = {};
  bool categoryEnabled = true;
  String category = "";
};

} // namespace ECS
} // namespace Star
