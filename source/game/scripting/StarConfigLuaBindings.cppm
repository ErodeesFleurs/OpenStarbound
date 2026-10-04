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
