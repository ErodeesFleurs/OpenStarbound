#pragma once

#include "StarLua.hpp"
#include "StarNetworkedAnimator.hpp"

import std;

namespace Star::LuaBindings {
auto makeScriptedAnimatorCallbacks(NetworkedAnimator* animator, std::function<Json(String const&, Json const&)> getParameter) -> LuaCallbacks;
}
