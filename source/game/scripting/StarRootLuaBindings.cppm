module;
#include "StarConfig.hpp"
import star.lua;

export module star.root_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeRootCallbacks();
}

