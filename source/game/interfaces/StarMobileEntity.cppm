module;
#include "StarConfig.hpp"
import star.json;
import star.id_map;
import star.maybe;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;


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
