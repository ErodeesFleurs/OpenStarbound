#pragma once

#include "StarLua.hpp"

namespace Star {

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeConfigCallbacks(function<Json(String const&, Json const&)> getParameter);
}
}
