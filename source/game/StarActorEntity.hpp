#pragma once

#include "StarActorMovementController.hpp"
#include "StarEntity.hpp"

namespace Star {

class StatusController;

// this is just used to have a base for what the game generally considers 'actors' as they all use the ActorMovementController, as well as have a StatusController
// theres potentially more things shared that could be moved here
class ActorEntity : public virtual Entity {
public:
  virtual auto movementController() -> ActorMovementController* = 0;
  virtual auto statusController() -> StatusController* = 0;
};
}// namespace Star
