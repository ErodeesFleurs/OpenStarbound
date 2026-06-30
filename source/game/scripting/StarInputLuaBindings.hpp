#pragma once

#include "StarLua.hpp"

namespace Star {

class Input;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeInputCallbacks(Input& input);
}

}
