module;

import std;
import star.format;
import star.json;
import star.lua;

namespace Star {
class Root;
class UniverseServer;
}

export module star.universe_server_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeUniverseServerCallbacks(Root& root, UniverseServer& universe);
}
