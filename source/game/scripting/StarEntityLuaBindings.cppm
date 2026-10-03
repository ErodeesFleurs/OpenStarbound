module;

namespace Star {
class LuaCallbacks;
class Entity;
}

export module star.entity_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeEntityCallbacks(Entity const* entity);
}
