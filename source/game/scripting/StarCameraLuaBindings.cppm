module;


namespace Star {
class LuaCallbacks;
class WorldCamera;
}

export module star.camera_lua_bindings;

export namespace Star {
namespace LuaBindings {
  LuaCallbacks makeCameraCallbacks(WorldCamera* camera);
}
}
