module;
import std;
import star.string;
import star.maybe;
import star.json;
import star.id_map;
import star.casting;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.lua;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

// All ScriptedEntity methods should only be called on master entities
class ScriptedEntity : public virtual Entity {
public:
  // Call a script function directly with the given arguments, should return
  // nothing only on failure.
  virtual Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) = 0;

  // Execute the given code directly in the underlying context, return nothing
  // on failure.
  virtual Maybe<LuaValue> evalScript(String const& code) = 0;
};

}

export module star.scripted_entity;

export namespace Star {
  using ::Star::ScriptedEntity;
}
