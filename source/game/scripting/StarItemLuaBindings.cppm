module;

namespace Star {
class LuaCallbacks;
class Item;
}

export module star.item_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeItemCallbacks(Item* item);
}

