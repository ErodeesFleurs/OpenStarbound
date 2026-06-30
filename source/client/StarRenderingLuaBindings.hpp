#pragma once

#include "StarLua.hpp"

namespace Star {
  
class ClientApplication;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeRenderingCallbacks(ClientApplication& app);
}

}
