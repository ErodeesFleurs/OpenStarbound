module;
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarLua.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

STAR_CLASS(ScriptedEntity);

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
