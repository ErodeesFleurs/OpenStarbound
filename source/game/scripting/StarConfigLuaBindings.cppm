module;

import std;
import star.format;
import star.json;
import star.lua;

namespace Star {
class Json;
class String;
class LuaCallbacks;
}

export module star.config_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeConfigCallbacks(std::function<std::expected<Json, StarException>(String const&, Json const&)> getParameter);
}
