#include "StarContainerEntity.hpp"

import star.item_bag;

namespace Star {

size_t ContainerEntity::containerSize() const {
  return itemBag()->size();
}

List<ItemPtr> ContainerEntity::containerItems() const {
  return itemBag()->items();
}

}

