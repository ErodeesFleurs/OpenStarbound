module;
import std;
import star.format;

namespace Star {
class LuaCallbacks;
class ClientApplication;
}

export module star.rendering_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeRenderingCallbacks(ClientApplication& app);
}
