module;
import std;
import star.list;
import star.json;
import star.id_map;
#include "StarStrongTypedef.hpp"
import star.data_stream;

import star.status_types;

namespace Star {

class StatusEffectItem;
using StatusEffectItemPtr = std::shared_ptr<StatusEffectItem>;
using StatusEffectItemConstPtr = std::shared_ptr<const StatusEffectItem>;
using StatusEffectItemWeakPtr = std::weak_ptr<StatusEffectItem>;
using StatusEffectItemConstWeakPtr = std::weak_ptr<const StatusEffectItem>;
using StatusEffectItemUPtr = std::unique_ptr<StatusEffectItem>;
using StatusEffectItemConstUPtr = std::unique_ptr<const StatusEffectItem>;

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
