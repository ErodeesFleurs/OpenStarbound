module;

#include <memory>

namespace Star {
class LuaCallbacks;
class CelestialDatabase;
using CelestialDatabasePtr = std::shared_ptr<CelestialDatabase>;
class Universe;
}

export module star.celestial_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeCelestialCallbacks(CelestialDatabasePtr database);
  LuaCallbacks makeCelestialCallbacks(Universe* universe);
}
