module;

import std;
import star.json;
import star.lua;

namespace Star {
class UniverseServer;
using UniverseServerPtr = std::shared_ptr<UniverseServer>;
using UniverseServerConstPtr = std::shared_ptr<const UniverseServer>;
using UniverseServerWeakPtr = std::weak_ptr<UniverseServer>;
using UniverseServerConstWeakPtr = std::weak_ptr<const UniverseServer>;
using UniverseServerUPtr = std::unique_ptr<UniverseServer>;
using UniverseServerConstUPtr = std::unique_ptr<const UniverseServer>;
}

export module star.universe_server_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeUniverseServerCallbacks(UniverseServer* universe);
}
