module;

#include "StarJson.hpp"
#include "StarLua.hpp"

namespace Star {
STAR_CLASS(UniverseServer);
}

export module star.universe_server_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeUniverseServerCallbacks(UniverseServer* universe);
}
