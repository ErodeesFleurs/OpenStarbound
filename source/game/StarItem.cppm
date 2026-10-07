module;
#include "StarItem.inc"

export module star.item;

export namespace Star {
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::GenericItem;
  using ::Star::GenericItemPtr;
  using ::Star::GenericItemConstPtr;
  using ::Star::GenericItemWeakPtr;
  using ::Star::GenericItemConstWeakPtr;
  using ::Star::GenericItemUPtr;
  using ::Star::GenericItemConstUPtr;
  using ::Star::ItemExceptionTag;
  using ::Star::ItemException;
  using ::Star::itemSafeCount;
  using ::Star::itemSafeTwoHanded;
  using ::Star::itemSafeOneHanded;
  using ::Star::itemSafeDescriptor;
}

