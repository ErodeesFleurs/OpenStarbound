module;

namespace Star {
class LuaCallbacks;
class ClientApplication;
}

export module star.rendering_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeRenderingCallbacks(ClientApplication* app);
}
