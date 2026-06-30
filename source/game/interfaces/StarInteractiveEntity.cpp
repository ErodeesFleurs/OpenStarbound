#include "StarInteractiveEntity.hpp"

namespace Star {

[[nodiscard]] RectF InteractiveEntity::interactiveBoundBox() const {
  return metaBoundBox();
}

[[nodiscard]] bool InteractiveEntity::isInteractive() const {
  return true;
}

[[nodiscard]] List<QuestArcDescriptor> InteractiveEntity::offeredQuests() const {
  return {};
}

[[nodiscard]] StringSet InteractiveEntity::turnInQuests() const {
  return {};
}

[[nodiscard]] Vec2F InteractiveEntity::questIndicatorPosition() const {
  return position();
}

}
