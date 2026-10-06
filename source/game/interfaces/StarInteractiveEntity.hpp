#pragma once

#include "StarGameTypes.hpp"
#include "StarJson.hpp"
import star.interaction_types;
#include "StarEntity.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarItemDescriptor.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
import star.quest_descriptor;

namespace Star {

STAR_CLASS(InteractiveEntity);

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
