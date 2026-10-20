module;

namespace Star {
class LuaCallbacks;
class Voice;
}

export module star.voice_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeVoiceCallbacks(Voice& voice);
}
