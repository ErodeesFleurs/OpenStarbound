#pragma once

#include "StarEntity.hpp"
#include "StarLua.hpp"

import std;

namespace Star {

// All ScriptedEntity methods should only be called on master entities
class ScriptedEntity : public virtual Entity {
public:
  // Call a script function directly with the given arguments, should return
  // nothing only on failure.
  virtual auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> = 0;

  // Execute the given code directly in the underlying context, return nothing
  // on failure.
  virtual auto evalScript(String const& code) -> std::optional<LuaValue> = 0;
};

}// namespace Star
