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

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

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
}
