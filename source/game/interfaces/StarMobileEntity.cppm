module;
import std;
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

class StatusController;
using StatusControllerPtr = std::shared_ptr<StatusController>;
using StatusControllerConstPtr = std::shared_ptr<const StatusController>;
using StatusControllerWeakPtr = std::weak_ptr<StatusController>;
using StatusControllerConstWeakPtr = std::weak_ptr<const StatusController>;
using StatusControllerUPtr = std::unique_ptr<StatusController>;
using StatusControllerConstUPtr = std::unique_ptr<const StatusController>;
class ActorMovementController;
using ActorMovementControllerPtr = std::shared_ptr<ActorMovementController>;
using ActorMovementControllerConstPtr = std::shared_ptr<const ActorMovementController>;
using ActorMovementControllerWeakPtr = std::weak_ptr<ActorMovementController>;
using ActorMovementControllerConstWeakPtr = std::weak_ptr<const ActorMovementController>;
using ActorMovementControllerUPtr = std::unique_ptr<ActorMovementController>;
using ActorMovementControllerConstUPtr = std::unique_ptr<const ActorMovementController>;

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
