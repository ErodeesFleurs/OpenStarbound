#pragma once

#include "StarLua.hpp"

namespace Star {

class MovementController;

namespace LuaBindings {
auto makeMovementControllerCallbacks(MovementController* movementController) -> LuaCallbacks;
}
}// namespace Star
