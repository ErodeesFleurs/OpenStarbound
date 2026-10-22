module;
import std;
import star.format;

namespace Star {
class LuaCallbacks;
class Voice;
}

export module star.voice_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeVoiceCallbacks(Voice& voice);
}
