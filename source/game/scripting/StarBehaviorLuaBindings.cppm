module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarConfig.hpp"
import star.list;
import star.json;
import star.lua;

namespace Star {
STAR_CLASS(BehaviorState);
}

export module star.behavior_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeBehaviorCallbacks(List<BehaviorStatePtr>* list);
}

