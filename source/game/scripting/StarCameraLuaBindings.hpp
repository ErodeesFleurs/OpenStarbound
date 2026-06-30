#pragma once

#include "StarLua.hpp"

#include "StarConfiguration.hpp"
namespace Star {

class WorldCamera;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeCameraCallbacks(WorldCamera& camera, ConfigurationPtr configuration);
}
}
