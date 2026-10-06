#pragma once

#include "StarGameTypes.hpp"
#include "StarJson.hpp"

#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarWorld.hpp"
#include "StarPhysicsEntity.hpp"
import star.movement_controller;
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
import star.platformer_astar_types;
#include "StarAnchorableEntity.hpp"

import star.game_timers;
import star.actor_movement_controller;
#include "StarMobileEntity.hpp"


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
