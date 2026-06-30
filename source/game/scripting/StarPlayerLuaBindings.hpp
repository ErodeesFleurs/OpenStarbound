#pragma once

#include "StarLua.hpp"

namespace Star {

class Player;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makePlayerCallbacks(Player& player);
}
}
