module;
#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"

import star.status_types;

namespace Star {

STAR_CLASS(StatusEffectItem);

class StatusEffectItem {
public:
  virtual ~StatusEffectItem() {}
  virtual List<PersistentStatusEffect> statusEffects() const = 0;
};

}

export module star.status_effect_item;

export namespace Star {
  using ::Star::StatusEffectItem;
  using ::Star::StatusEffectItemPtr;
  using ::Star::StatusEffectItemConstPtr;
  using ::Star::StatusEffectItemWeakPtr;
  using ::Star::StatusEffectItemConstWeakPtr;
  using ::Star::StatusEffectItemUPtr;
  using ::Star::StatusEffectItemConstUPtr;
}
