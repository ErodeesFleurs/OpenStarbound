module;
import std;
import star.format;
import star.lua;

namespace Star {
class LuaCallbacks;
class StatusController;
}

export module star.status_controller_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeStatusControllerCallbacks(StatusController& statController);
}
