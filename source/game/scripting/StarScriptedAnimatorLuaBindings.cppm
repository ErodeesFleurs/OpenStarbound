module;

// Match client include order for SIMD intrinsics used by xxhash.
import std;
import star.format;
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
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeScriptedAnimatorCallbacks(NetworkedAnimator* animator, std::function<std::expected<Json, StarException>(String const&, Json const&)> getParameter);
}
