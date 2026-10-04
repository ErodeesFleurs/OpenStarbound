module;

namespace Star {
class LuaCallbacks;
class UniverseClient;
}

export module star.universe_client_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeUniverseClientThreadCallbacks(UniverseClient* universe); // thread-safe callbacks
  LuaCallbacks makeUniverseClientCallbacks(UniverseClient* universe); // non-thread-safe callbacks
}
