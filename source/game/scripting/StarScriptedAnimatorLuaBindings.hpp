#pragma once

#include "StarLua.hpp"
#include "StarNetworkedAnimator.hpp"

namespace Star {

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeScriptedAnimatorCallbacks(NetworkedAnimator& animator, function<Json(String const&, Json const&)> getParameter);
}
}
