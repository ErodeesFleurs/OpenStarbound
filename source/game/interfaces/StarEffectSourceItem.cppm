module;
import std;
import star.string;


namespace Star {

class EffectSourceItem {
public:
  virtual ~EffectSourceItem() {}
  virtual StringSet effectSources() const = 0;
};

}

export module star.effect_source_item;

export namespace Star {
  using ::Star::EffectSourceItem;
}
