#pragma once

#include "StarLua.hpp"

namespace Star {

class WorldCamera;

namespace LuaBindings {
  LuaCallbacks makeCameraCallbacks(WorldCamera* camera);
}
}
