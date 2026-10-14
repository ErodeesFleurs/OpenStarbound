module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarConfig.hpp"
import star.string;
import star.json;
import star.lua;
#include "StarJsonExtra.hpp"
#include "StarLuaGameConverters.hpp"

module star.config_lua_bindings;

namespace Star {

LuaCallbacks LuaBindings::makeConfigCallbacks(function<Json(String const&, Json const&)> getParameter) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("getParameter", getParameter);

  return callbacks;
}

}
