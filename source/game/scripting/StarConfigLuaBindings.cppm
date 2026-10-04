#ifdef STAR_MODULE_IMPLEMENTATION
module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarJson.hpp"
#include "StarLua.hpp"
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
#else
module;

#include <functional>

namespace Star {
class Json;
class String;
class LuaCallbacks;
}

export module star.config_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeConfigCallbacks(std::function<Json(String const&, Json const&)> getParameter);
}
#endif
