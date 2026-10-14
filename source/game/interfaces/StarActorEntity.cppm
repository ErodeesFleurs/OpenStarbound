module;
#include "StarConfig.hpp"
import star.json;
import star.id_map;
import star.game_types;
import star.maybe;
import star.bi_map;
import star.a_star;
#include "StarCasting.hpp"
import star.data_stream;
#include "StarStrongTypedef.hpp"
import star.variant;
import star.list;
import star.rpc_promise;

import star.mobile_entity;
import star.actor_movement_controller;

namespace Star {

STAR_CLASS(StatusController);
STAR_CLASS(ActorMovementController);

// this is just used to have a base for what the game generally considers 'actors' as they all use the ActorMovementController, as well as have a StatusController
// theres potentially more things shared that could be moved here
class ActorEntity : public virtual MobileEntity {
public:
  virtual ActorMovementController* movementController() override = 0;
  virtual StatusController* statusController() = 0;
};
}

export module star.actor_entity;

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
  using ::Star::ActorEntity;
}
