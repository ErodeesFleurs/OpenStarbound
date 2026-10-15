module;
import std;


namespace Star {

class DurabilityItem {
public:
  virtual ~DurabilityItem() {}
  virtual float durabilityStatus() = 0;
};

}

export module star.durability_item;

export namespace Star {
  using ::Star::DurabilityItem;
}
