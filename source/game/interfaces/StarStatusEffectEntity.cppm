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

STAR_CLASS(StatusEffectEntity);

class StatusEffectEntity : public virtual Entity {
public:
  virtual List<PersistentStatusEffect> statusEffects() const = 0;
  virtual PolyF statusEffectArea() const = 0;
};

}

export module star.status_effect_entity;

export namespace Star {
  using ::Star::StatusEffectEntity;
  using ::Star::StatusEffectEntityPtr;
  using ::Star::StatusEffectEntityConstPtr;
  using ::Star::StatusEffectEntityWeakPtr;
  using ::Star::StatusEffectEntityConstWeakPtr;
  using ::Star::StatusEffectEntityUPtr;
  using ::Star::StatusEffectEntityConstUPtr;
}
