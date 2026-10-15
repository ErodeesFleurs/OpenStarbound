module;

import std;
import star.json;
import star.lua;

namespace Star {
class UniverseServer;
}

export module star.universe_server_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeUniverseServerCallbacks(UniverseServer* universe);
}
