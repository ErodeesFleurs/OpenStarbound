#pragma once

#include "StarEntity.hpp"

namespace Star {

STAR_CLASS(InspectableEntity);

class InspectableEntity : public virtual Entity {
public:
  // Default implementation returns true
  virtual bool inspectable() const;

  // If this entity can be entered into the player log, will return the log
  // identifier.
  virtual std::optional<String> inspectionLogName() const;

  // Long description to display when inspected, if any
  virtual std::optional<String> inspectionDescription(String const& species) const;
};

inline bool InspectableEntity::inspectable() const {
  return true;
}

inline std::optional<String> InspectableEntity::inspectionLogName() const {
  return {};
}

inline std::optional<String> InspectableEntity::inspectionDescription(String const&) const {
  return {};
}

}
