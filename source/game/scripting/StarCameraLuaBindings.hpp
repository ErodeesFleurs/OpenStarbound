#pragma once

#include "StarLua.hpp"

namespace Star {

class WorldCamera;
class Configuration;
using ConfigurationPtr = SharedPtr<Configuration>;

namespace LuaBindings {
  LuaCallbacks makeCameraCallbacks(WorldCamera* camera, ConfigurationPtr configuration);
}
}
