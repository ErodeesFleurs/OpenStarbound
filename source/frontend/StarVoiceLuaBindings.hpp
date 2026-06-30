#pragma once

#include "StarLua.hpp"

namespace Star {

class Voice;
using VoicePtr = SharedPtr<Voice>;

namespace LuaBindings {
  LuaCallbacks makeVoiceCallbacks();
}

}
