#pragma once

#include "StarLua.hpp"

namespace Star {

class NetworkedAnimator;

namespace LuaBindings {
auto makeNetworkedAnimatorCallbacks(NetworkedAnimator* networkedAnimator) -> LuaCallbacks;
}

}// namespace Star
