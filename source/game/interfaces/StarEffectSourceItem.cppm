module;
#include "StarString.hpp"


namespace Star {

STAR_CLASS(EffectSourceItem);

class EffectSourceItem {
public:
  virtual ~EffectSourceItem() {}
  virtual StringSet effectSources() const = 0;
};

}

export module star.effect_source_item;

export namespace Star {
  using ::Star::EffectSourceItem;
  using ::Star::EffectSourceItemPtr;
  using ::Star::EffectSourceItemConstPtr;
  using ::Star::EffectSourceItemWeakPtr;
  using ::Star::EffectSourceItemConstWeakPtr;
  using ::Star::EffectSourceItemUPtr;
  using ::Star::EffectSourceItemConstUPtr;
}
