#pragma once

#include "StarLua.hpp"

namespace Star {
  
class ClientApplication;

namespace LuaBindings {
  LuaCallbacks makeRenderingCallbacks(ClientApplication* app);
}

}
