module;

namespace Star {
class LuaCallbacks;
class MovementController;
}

export module star.movement_controller_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeMovementControllerCallbacks(MovementController* movementController);
}
