#pragma once

#include "StarLua.hpp"
#include "StarBehaviorState.hpp"

namespace Star {

class Root;
class UniverseClient;

namespace LuaBindings {
  LuaCallbacks makeBehaviorCallbacks(List<BehaviorStatePtr>* list);
}
}
