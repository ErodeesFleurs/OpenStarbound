module;

import std;

namespace Star {
class Root;
class LuaCallbacks;
class CelestialDatabase;
class Universe;
}

export module star.celestial_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeCelestialCallbacks(Root& root, std::shared_ptr<CelestialDatabase> database);
  LuaCallbacks makeCelestialCallbacks(Root& root, Universe* universe);
}
