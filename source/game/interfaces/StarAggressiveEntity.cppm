module;
import std;
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

class AggressiveEntity : public virtual Entity {
public:
  virtual bool aggressive() const = 0;
};

}

export module star.aggressive_entity;

export namespace Star {
  using ::Star::AggressiveEntity;
}
