module;
import std;
import star.string;
import star.maybe;
import star.json;
import star.id_map;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.lua;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

class ScriptedEntity;
using ScriptedEntityPtr = std::shared_ptr<ScriptedEntity>;
using ScriptedEntityConstPtr = std::shared_ptr<const ScriptedEntity>;
using ScriptedEntityWeakPtr = std::weak_ptr<ScriptedEntity>;
using ScriptedEntityConstWeakPtr = std::weak_ptr<const ScriptedEntity>;
using ScriptedEntityUPtr = std::unique_ptr<ScriptedEntity>;
using ScriptedEntityConstUPtr = std::unique_ptr<const ScriptedEntity>;

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
  using ::Star::ScriptedEntityPtr;
  using ::Star::ScriptedEntityConstPtr;
  using ::Star::ScriptedEntityWeakPtr;
  using ::Star::ScriptedEntityConstWeakPtr;
  using ::Star::ScriptedEntityUPtr;
  using ::Star::ScriptedEntityConstUPtr;
}
