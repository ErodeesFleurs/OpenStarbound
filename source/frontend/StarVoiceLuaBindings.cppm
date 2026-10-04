module;

namespace Star {
class LuaCallbacks;
}

export module star.voice_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeVoiceCallbacks();
}
