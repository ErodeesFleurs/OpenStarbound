#pragma once

#include "StarLua.hpp"

namespace Star {

class Root;
class UniverseClient;

namespace LuaBindings {
  LuaCallbacks makeCelestialCallbacks(UniverseClient* client);
}
}
