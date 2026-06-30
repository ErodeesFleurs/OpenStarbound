#pragma once

#include "StarLua.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"

namespace Star {

class NetworkedAnimator;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeNetworkedAnimatorCallbacks(NetworkedAnimator& networkedAnimator);
}

}
