module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import std;
import star.json;
import star.lua;

namespace Star {
class NetworkedAnimator;
using NetworkedAnimatorPtr = std::shared_ptr<NetworkedAnimator>;
using NetworkedAnimatorConstPtr = std::shared_ptr<const NetworkedAnimator>;
using NetworkedAnimatorWeakPtr = std::weak_ptr<NetworkedAnimator>;
using NetworkedAnimatorConstWeakPtr = std::weak_ptr<const NetworkedAnimator>;
using NetworkedAnimatorUPtr = std::unique_ptr<NetworkedAnimator>;
using NetworkedAnimatorConstUPtr = std::unique_ptr<const NetworkedAnimator>;
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
