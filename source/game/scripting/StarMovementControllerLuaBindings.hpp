#pragma once

#include "StarLua.hpp"

namespace Star {

class MovementController;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeMovementControllerCallbacks(MovementController& movementController);
}
}
