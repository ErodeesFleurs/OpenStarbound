module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import std;
import star.format;
import star.json;
import star.lua;

namespace Star {
class NetworkedAnimator;
}

export module star.networked_animator_lua_bindings;

export namespace Star {
  using ::Star::NetworkedAnimator;
}

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeNetworkedAnimatorCallbacks(NetworkedAnimator* networkedAnimator);
}
