module;
#include "StarConfig.hpp"
import star.list;
import star.json;
import star.id_map;
#include "StarStrongTypedef.hpp"
import star.data_stream;

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
