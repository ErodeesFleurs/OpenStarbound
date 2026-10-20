module;

namespace Star {
class LuaCallbacks;
class Input;
}

export module star.input_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeInputCallbacks(Input& input);
}
