#pragma once

#include "StarLua.hpp"

namespace Star {

class Root;
class UniverseClient;
class BiomeDatabase;
using BiomeDatabaseConstPtr = SharedPtr<BiomeDatabase const>;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeCelestialCallbacks(UniverseClient& client, BiomeDatabaseConstPtr biomeDatabase);
}
}
