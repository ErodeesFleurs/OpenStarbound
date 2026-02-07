#pragma once

#include "StarEntity.hpp"
#include "StarInteractionTypes.hpp"
#include "StarQuestDescriptor.hpp"

namespace Star {

class InteractiveEntity : public virtual Entity {
public:
  // Interaction always takes place on the *server*, whether the interactive
  // entity is master or slave there.
  virtual auto interact(InteractRequest const& request) -> InteractAction = 0;

  // Defaults to metaBoundBox
  [[nodiscard]] virtual auto interactiveBoundBox() const -> RectF;

  // Defaults to true
  [[nodiscard]] virtual auto isInteractive() const -> bool;

  // Defaults to empty
  [[nodiscard]] virtual auto offeredQuests() const -> List<QuestArcDescriptor>;
  [[nodiscard]] virtual auto turnInQuests() const -> StringSet;

  // Defaults to position()
  [[nodiscard]] virtual auto questIndicatorPosition() const -> Vec2F;
};

}// namespace Star
