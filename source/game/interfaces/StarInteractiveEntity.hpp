#pragma once

#include "StarInteractionTypes.hpp"
#include "StarEntity.hpp"
#include "StarQuestDescriptor.hpp"

namespace Star {

class InteractiveEntity;
using InteractiveEntityPtr = SharedPtr<InteractiveEntity>;

class InteractiveEntity : public virtual Entity {
public:
  // Interaction always takes place on the *server*, whether the interactive
  // entity is master or slave there.
  [[nodiscard]] virtual InteractAction interact(InteractRequest const& request) = 0;

  // Defaults to metaBoundBox
  [[nodiscard]] virtual RectF interactiveBoundBox() const;

  // Defaults to true
  [[nodiscard]] virtual bool isInteractive() const;

  // Defaults to empty
  [[nodiscard]] virtual List<QuestArcDescriptor> offeredQuests() const;
  [[nodiscard]] virtual StringSet turnInQuests() const;

  // Defaults to position()
  [[nodiscard]] virtual Vec2F questIndicatorPosition() const;
};

}
