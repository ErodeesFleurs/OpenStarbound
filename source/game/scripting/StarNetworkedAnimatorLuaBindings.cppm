module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarJson.hpp"
#include "StarLua.hpp"

namespace Star {
STAR_CLASS(NetworkedAnimator);
}

export module star.networked_animator_lua_bindings;

export namespace Star {
  using ::Star::NetworkedAnimator;
  using ::Star::NetworkedAnimatorPtr;
  using ::Star::NetworkedAnimatorConstPtr;
  using ::Star::NetworkedAnimatorWeakPtr;
  using ::Star::NetworkedAnimatorConstWeakPtr;
  using ::Star::NetworkedAnimatorUPtr;
  using ::Star::NetworkedAnimatorConstUPtr;
}

export namespace Star::LuaBindings {
  LuaCallbacks makeNetworkedAnimatorCallbacks(NetworkedAnimator* networkedAnimator);
}
