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
  virtual InteractAction interact(InteractRequest const& request) = 0;

  // Defaults to metaBoundBox
  virtual RectF interactiveBoundBox() const;

  // Defaults to true
  virtual bool isInteractive() const;

  // Defaults to empty
  virtual List<QuestArcDescriptor> offeredQuests() const;
  virtual StringSet turnInQuests() const;

  // Defaults to position()
  virtual Vec2F questIndicatorPosition() const;
};

}
