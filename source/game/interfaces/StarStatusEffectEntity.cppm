module;
import std;
import star.list;
import star.json;
import star.id_map;
import star.casting;
import star.data_stream;
import star.game_types;
import star.poly;
import star.strong_typedef;
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
