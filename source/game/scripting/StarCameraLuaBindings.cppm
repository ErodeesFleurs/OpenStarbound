module;


namespace Star {
class Root;
class LuaCallbacks;
class WorldCamera;
}

export module star.camera_lua_bindings;

export namespace Star {
namespace LuaBindings {
  LuaCallbacks makeCameraCallbacks(Root& root, WorldCamera* camera);
}
}
