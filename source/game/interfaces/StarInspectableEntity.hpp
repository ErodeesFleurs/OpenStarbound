#pragma once

#include "StarEntity.hpp"

namespace Star {

class InspectableEntity;
using InspectableEntityPtr = SharedPtr<InspectableEntity>;

class InspectableEntity : public virtual Entity {
public:
  // Default implementation returns true
  [[nodiscard]] virtual bool inspectable() const;

  // If this entity can be entered into the player log, will return the log
  // identifier.
  [[nodiscard]] virtual Maybe<String> inspectionLogName() const;

  // Long description to display when inspected, if any
  [[nodiscard]] virtual Maybe<String> inspectionDescription(String const& species) const;
};

[[nodiscard]] inline bool InspectableEntity::inspectable() const {
  return true;
}

[[nodiscard]] inline Maybe<String> InspectableEntity::inspectionLogName() const {
  return {};
}

[[nodiscard]] inline Maybe<String> InspectableEntity::inspectionDescription(String const&) const {
  return {};
}

}
