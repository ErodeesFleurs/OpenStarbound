module;

namespace Star {
class LuaCallbacks;
}

export module star.input_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeInputCallbacks();
}
