module;
import std;
import star.format;

namespace Star {
class LuaCallbacks;
class UniverseClient;
class Root;
}

export module star.universe_client_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeUniverseClientThreadCallbacks(Root& root, UniverseClient* universe); // thread-safe callbacks
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeUniverseClientCallbacks(Root& root, UniverseClient* universe); // non-thread-safe callbacks
}
