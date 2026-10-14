module;

#include "StarConfig.hpp"
import star.json;
import star.lua;

namespace Star {
STAR_CLASS(UniverseServer);
}

export module star.universe_server_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeUniverseServerCallbacks(UniverseServer* universe);
}
