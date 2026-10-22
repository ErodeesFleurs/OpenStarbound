module;
import std;
import star.format;
import star.lua;


namespace Star {
class Root;
class LuaCallbacks;
class WorldCamera;
}

export module star.camera_lua_bindings;

export namespace Star {
namespace LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeCameraCallbacks(Root& root, WorldCamera* camera);
}
}
