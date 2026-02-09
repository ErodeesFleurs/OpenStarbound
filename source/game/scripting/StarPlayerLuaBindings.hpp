#pragma once

#include "StarLua.hpp"

namespace Star {

class Player;

namespace LuaBindings {
  auto makePlayerCallbacks(Player* player) -> LuaCallbacks;
}
}
