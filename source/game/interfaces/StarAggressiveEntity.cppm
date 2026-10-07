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

STAR_CLASS(AggressiveEntity);

class AggressiveEntity : public virtual Entity {
public:
  virtual bool aggressive() const = 0;
};

}

export module star.aggressive_entity;

export namespace Star {
  using ::Star::AggressiveEntity;
  using ::Star::AggressiveEntityPtr;
  using ::Star::AggressiveEntityConstPtr;
  using ::Star::AggressiveEntityWeakPtr;
  using ::Star::AggressiveEntityConstWeakPtr;
  using ::Star::AggressiveEntityUPtr;
  using ::Star::AggressiveEntityConstUPtr;
}
