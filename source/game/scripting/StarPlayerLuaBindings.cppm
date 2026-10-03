module;

namespace Star {
class LuaCallbacks;
class Player;
}

export module star.player_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makePlayerCallbacks(Player* player);
}
