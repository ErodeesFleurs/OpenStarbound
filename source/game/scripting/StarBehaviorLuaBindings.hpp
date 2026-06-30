#pragma once

#include "StarLua.hpp"
#include "StarBehaviorState.hpp"

namespace Star {

class BehaviorDatabase;
using BehaviorDatabaseConstPtr = SharedPtr<BehaviorDatabase const>;

namespace LuaBindings {
  LuaCallbacks makeBehaviorCallbacks(List<BehaviorStatePtr>* list, BehaviorDatabaseConstPtr behaviorDatabase);
}
}
