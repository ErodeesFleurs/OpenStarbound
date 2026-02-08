#pragma once

#include "StarLua.hpp"

import std;

namespace Star::LuaBindings {
auto makeConfigCallbacks(std::function<Json(String const&, Json const&)> getParameter) -> LuaCallbacks;
}
