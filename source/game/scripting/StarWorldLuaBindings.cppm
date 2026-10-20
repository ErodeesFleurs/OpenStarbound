module;

namespace Star {
class LuaCallbacks;
class World;
class Root;
namespace LuaBindings {
  // Borrows the script domain's root and world; both must outlive the callbacks.
  LuaCallbacks makeWorldThreadCallbacks(Root& root, World* world);
  LuaCallbacks makeWorldCallbacks(Root& root, World* world);
}
}

export module star.world_lua_bindings;

export namespace Star::LuaBindings {
  using ::Star::LuaBindings::makeWorldThreadCallbacks;
  using ::Star::LuaBindings::makeWorldCallbacks;
}
