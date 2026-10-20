module;

namespace Star {
class Root;
class LuaCallbacks;
class Player;
}

export module star.player_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makePlayerCallbacks(Root& root, Player* player);
}
