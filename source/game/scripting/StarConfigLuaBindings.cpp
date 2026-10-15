module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import std;
import star.string;
import star.json;
import star.lua;
import star.json_extra;
import star.lua_game_converters;

module star.config_lua_bindings;

namespace Star {

LuaCallbacks LuaBindings::makeConfigCallbacks(std::function<Json(String const&, Json const&)> getParameter) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("getParameter", getParameter);

  return callbacks;
}

}
