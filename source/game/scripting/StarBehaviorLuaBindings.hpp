#pragma once

#include "StarBehaviorState.hpp"
#include "StarConfig.hpp"
#include "StarLua.hpp"

namespace Star::LuaBindings {
auto makeBehaviorCallbacks(List<Ptr<BehaviorState>>* list) -> LuaCallbacks;
}
