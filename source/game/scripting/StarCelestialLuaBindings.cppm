module;

import std;

namespace Star {
class LuaCallbacks;
class CelestialDatabase;
class Universe;
}

export module star.celestial_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeCelestialCallbacks(std::shared_ptr<CelestialDatabase> database);
  LuaCallbacks makeCelestialCallbacks(Universe* universe);
}
