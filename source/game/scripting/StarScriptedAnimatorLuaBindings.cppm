module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import std;
import star.string;
import star.json;
import star.lua;

namespace Star {
class NetworkedAnimator;
}

export module star.scripted_animator_lua_bindings;

export namespace Star {
  using ::Star::NetworkedAnimator;
}

export namespace Star::LuaBindings {
  LuaCallbacks makeScriptedAnimatorCallbacks(NetworkedAnimator* animator, std::function<Json(String const&, Json const&)> getParameter);
}
