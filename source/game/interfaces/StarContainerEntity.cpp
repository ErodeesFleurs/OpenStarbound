#include "StarContainerEntity.hpp"
#include "StarItemBag.hpp"

namespace Star {

[[nodiscard]] size_t ContainerEntity::containerSize() const {
  return itemBag()->size();
}

[[nodiscard]] List<ItemPtr> ContainerEntity::containerItems() const {
  return itemBag()->items();
}

}
