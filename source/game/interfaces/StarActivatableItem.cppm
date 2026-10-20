module;
import std;
import star.type_identity;


namespace Star {

class ActivatableItem {
public:
  virtual void const* queryType(TypeKey key) const noexcept {
    return queryTypeOf<ActivatableItem>(this, key);
  }
  virtual std::string_view dynamicTypeName() const noexcept {
    return Star::typeName<ActivatableItem>();
  }

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
}
