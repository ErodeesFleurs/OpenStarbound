module;
import std;
import star.format;
import star.lua;

namespace Star {
class LuaCallbacks;
class Item;
}

export module star.item_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeItemCallbacks(Item& item);
}

