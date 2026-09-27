module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaGameConverters.hpp"

export module star.config_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeConfigCallbacks(function<Json(String const&, Json const&)> getParameter);
}

namespace Star {

LuaCallbacks LuaBindings::makeConfigCallbacks(function<Json(String const&, Json const&)> getParameter) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("getParameter", getParameter);

  return callbacks;
}

}
