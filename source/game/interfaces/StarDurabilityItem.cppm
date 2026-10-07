module;
#include "StarConfig.hpp"


namespace Star {

STAR_CLASS(DurabilityItem);

class DurabilityItem {
public:
  virtual ~DurabilityItem() {}
  virtual float durabilityStatus() = 0;
};

}

export module star.durability_item;

export namespace Star {
  using ::Star::DurabilityItem;
  using ::Star::DurabilityItemPtr;
  using ::Star::DurabilityItemConstPtr;
  using ::Star::DurabilityItemWeakPtr;
  using ::Star::DurabilityItemConstWeakPtr;
  using ::Star::DurabilityItemUPtr;
  using ::Star::DurabilityItemConstUPtr;
}
