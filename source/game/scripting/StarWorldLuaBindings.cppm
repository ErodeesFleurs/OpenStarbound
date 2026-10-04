module;

namespace Star {
class LuaCallbacks;
class World;
namespace LuaBindings {
  LuaCallbacks makeWorldThreadCallbacks(World* world);
  LuaCallbacks makeWorldCallbacks(World* world);
}
}

export module star.world_lua_bindings;

export namespace Star::LuaBindings {
  using ::Star::LuaBindings::makeWorldThreadCallbacks;
  using ::Star::LuaBindings::makeWorldCallbacks;
}
