module;
#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"


import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.movement_controller;

namespace Star {

STAR_CLASS(StatusController);
STAR_CLASS(ActorMovementController);

// A base for 'mobile' entities, which all have a MovementController.
class MobileEntity : public virtual Entity {
public:
  virtual MovementController* movementController() = 0;
};
}

export module star.mobile_entity;

export namespace Star {
  using ::Star::StatusController;
  using ::Star::StatusControllerPtr;
  using ::Star::StatusControllerConstPtr;
  using ::Star::StatusControllerWeakPtr;
  using ::Star::StatusControllerConstWeakPtr;
  using ::Star::StatusControllerUPtr;
  using ::Star::StatusControllerConstUPtr;
  using ::Star::ActorMovementController;
  using ::Star::ActorMovementControllerPtr;
  using ::Star::ActorMovementControllerConstPtr;
  using ::Star::ActorMovementControllerWeakPtr;
  using ::Star::ActorMovementControllerConstWeakPtr;
  using ::Star::ActorMovementControllerUPtr;
  using ::Star::ActorMovementControllerConstUPtr;
  using ::Star::MobileEntity;
}
