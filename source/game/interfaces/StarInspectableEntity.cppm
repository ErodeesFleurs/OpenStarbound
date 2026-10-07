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

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

STAR_CLASS(InspectableEntity);

class InspectableEntity : public virtual Entity {
public:
  // Default implementation returns true
  virtual bool inspectable() const;

  // If this entity can be entered into the player log, will return the log
  // identifier.
  virtual Maybe<String> inspectionLogName() const;

  // Long description to display when inspected, if any
  virtual Maybe<String> inspectionDescription(String const& species) const;
};

inline bool InspectableEntity::inspectable() const {
  return true;
}

inline Maybe<String> InspectableEntity::inspectionLogName() const {
  return {};
}

inline Maybe<String> InspectableEntity::inspectionDescription(String const&) const {
  return {};
}

}

export module star.inspectable_entity;

export namespace Star {
  using ::Star::InspectableEntity;
  using ::Star::InspectableEntityPtr;
  using ::Star::InspectableEntityConstPtr;
  using ::Star::InspectableEntityWeakPtr;
  using ::Star::InspectableEntityConstWeakPtr;
  using ::Star::InspectableEntityUPtr;
  using ::Star::InspectableEntityConstUPtr;
}
