#pragma once

#include "StarLua.hpp"

namespace Star {

class MovementController;

namespace LuaBindings {
  LuaCallbacks makeMovementControllerCallbacks(MovementController* movementController);
}
}
