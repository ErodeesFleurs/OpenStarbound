module;
import std;


namespace Star {

class DurabilityItem;
using DurabilityItemPtr = std::shared_ptr<DurabilityItem>;
using DurabilityItemConstPtr = std::shared_ptr<const DurabilityItem>;
using DurabilityItemWeakPtr = std::weak_ptr<DurabilityItem>;
using DurabilityItemConstWeakPtr = std::weak_ptr<const DurabilityItem>;
using DurabilityItemUPtr = std::unique_ptr<DurabilityItem>;
using DurabilityItemConstUPtr = std::unique_ptr<const DurabilityItem>;

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
