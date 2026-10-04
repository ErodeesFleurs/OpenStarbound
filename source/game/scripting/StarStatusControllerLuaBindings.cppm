module;

namespace Star {
class LuaCallbacks;
class StatusController;
}

export module star.status_controller_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeStatusControllerCallbacks(StatusController* statController);
}
