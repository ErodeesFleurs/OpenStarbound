module;
import std;
import star.string;


namespace Star {

class EffectSourceItem;
using EffectSourceItemPtr = std::shared_ptr<EffectSourceItem>;
using EffectSourceItemConstPtr = std::shared_ptr<const EffectSourceItem>;
using EffectSourceItemWeakPtr = std::weak_ptr<EffectSourceItem>;
using EffectSourceItemConstWeakPtr = std::weak_ptr<const EffectSourceItem>;
using EffectSourceItemUPtr = std::unique_ptr<EffectSourceItem>;
using EffectSourceItemConstUPtr = std::unique_ptr<const EffectSourceItem>;

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
