module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import std;
import star.list;
import star.json;
import star.lua;

namespace Star {
class Root;
class BehaviorState;
}

export module star.behavior_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeBehaviorCallbacks(Root& root, List<std::shared_ptr<BehaviorState>>* list);
}

