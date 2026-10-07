module;
#include "StarConfig.hpp"


namespace Star {

STAR_CLASS(ActivatableItem);

class ActivatableItem {
public:
  virtual ~ActivatableItem() {}
  virtual bool active() const = 0;
  virtual void setActive(bool active) = 0;
  virtual bool usable() const = 0;
  virtual void activate() = 0;
};

}

export module star.activatable_item;

export namespace Star {
  using ::Star::ActivatableItem;
  using ::Star::ActivatableItemPtr;
  using ::Star::ActivatableItemConstPtr;
  using ::Star::ActivatableItemWeakPtr;
  using ::Star::ActivatableItemConstWeakPtr;
  using ::Star::ActivatableItemUPtr;
  using ::Star::ActivatableItemConstUPtr;
}
