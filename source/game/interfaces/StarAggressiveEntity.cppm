module;
#include "StarJson.hpp"
import star.id_map;
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
import star.game_types;
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
