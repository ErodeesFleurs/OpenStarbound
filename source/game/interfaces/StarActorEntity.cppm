module;
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"

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
