module;
import std;
import star.list;
import star.json;
import star.id_map;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
import star.poly;
#include "StarStrongTypedef.hpp"
import star.bi_map;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

class StatusEffectEntity : public virtual Entity {
public:
  virtual List<PersistentStatusEffect> statusEffects() const = 0;
  virtual PolyF statusEffectArea() const = 0;
};

}

export module star.status_effect_entity;

export namespace Star {
  using ::Star::StatusEffectEntity;
}
