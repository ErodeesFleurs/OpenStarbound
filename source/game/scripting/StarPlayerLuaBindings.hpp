#pragma once

#include "StarLua.hpp"

namespace Star {

class Player;

namespace LuaBindings {
  LuaCallbacks makePlayerCallbacks(Player* player);
}
}
