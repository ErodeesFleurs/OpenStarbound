module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import std;
import star.list;
import star.json;
import star.lua;

namespace Star {
class BehaviorState;
using BehaviorStatePtr = std::shared_ptr<BehaviorState>;
using BehaviorStateConstPtr = std::shared_ptr<const BehaviorState>;
using BehaviorStateWeakPtr = std::weak_ptr<BehaviorState>;
using BehaviorStateConstWeakPtr = std::weak_ptr<const BehaviorState>;
using BehaviorStateUPtr = std::unique_ptr<BehaviorState>;
using BehaviorStateConstUPtr = std::unique_ptr<const BehaviorState>;
}

export module star.behavior_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeBehaviorCallbacks(List<BehaviorStatePtr>* list);
}

