module;

namespace Star {
class LuaCallbacks;
class UniverseClient;
class Root;
}

export module star.universe_client_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeUniverseClientThreadCallbacks(Root& root, UniverseClient* universe); // thread-safe callbacks
  LuaCallbacks makeUniverseClientCallbacks(Root& root, UniverseClient* universe); // non-thread-safe callbacks
}
