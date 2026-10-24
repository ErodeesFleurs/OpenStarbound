module;

// Match client include order for SIMD intrinsics used by xxhash.
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

  namespace Detail {
    enum class AnimatorStateCallbackOrder { Networked, Scripted };
    [[nodiscard]] std::expected<void, StarException> registerAnimatorStateCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator, AnimatorStateCallbackOrder order);
    [[nodiscard]] std::expected<void, StarException> registerAnimatorLocalTransformationCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator);
    [[nodiscard]] std::expected<void, StarException> registerAnimatorPartQueryCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator);
    [[nodiscard]] std::expected<void, StarException> registerAnimatorPlaybackQueryCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator);
    [[nodiscard]] std::expected<void, StarException> registerAnimatorDrawableCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator);
  }
}
