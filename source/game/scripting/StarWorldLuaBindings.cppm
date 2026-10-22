module;
import std;
import star.format;
import star.lua;

namespace Star {
class World;
class Root;
namespace LuaBindings {
  // Borrows the script domain's root and world; both must outlive the callbacks.
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeWorldThreadCallbacks(Root& root, World* world);
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeWorldCallbacks(Root& root, World* world);
}
}

export module star.world_lua_bindings;

export namespace Star::LuaBindings {
  using ::Star::LuaBindings::makeWorldThreadCallbacks;
  using ::Star::LuaBindings::makeWorldCallbacks;
}
