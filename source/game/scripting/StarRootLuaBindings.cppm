module;
#include "StarProjectile.hpp"

#include "StarJson.hpp"
#include "StarLua.hpp"
export module star.root_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeRootCallbacks();
}

