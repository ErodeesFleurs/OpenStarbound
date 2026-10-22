module;
import std;
import star.format;
import star.lua;

namespace Star {
class LuaCallbacks;
class Entity;
}

export module star.entity_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeEntityCallbacks(Entity const* entity);
}
