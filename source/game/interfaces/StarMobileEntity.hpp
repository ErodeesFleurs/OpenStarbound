#pragma once

#include "StarJson.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarWorld.hpp"
#include "StarPhysicsEntity.hpp"
import star.movement_controller;
#include "StarEntity.hpp"


namespace Star {

STAR_CLASS(StatusController);
STAR_CLASS(ActorMovementController);

// A base for 'mobile' entities, which all have a MovementController.
class MobileEntity : public virtual Entity {
public:
  virtual MovementController* movementController() = 0;
};
}
