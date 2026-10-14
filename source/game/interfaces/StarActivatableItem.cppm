module;
import std;


namespace Star {

class ActivatableItem;
using ActivatableItemPtr = std::shared_ptr<ActivatableItem>;
using ActivatableItemConstPtr = std::shared_ptr<const ActivatableItem>;
using ActivatableItemWeakPtr = std::weak_ptr<ActivatableItem>;
using ActivatableItemConstWeakPtr = std::weak_ptr<const ActivatableItem>;
using ActivatableItemUPtr = std::unique_ptr<ActivatableItem>;
using ActivatableItemConstUPtr = std::unique_ptr<const ActivatableItem>;

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
