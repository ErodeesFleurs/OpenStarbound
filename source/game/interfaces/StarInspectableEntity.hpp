#pragma once

#include "StarEntity.hpp"

import std;

namespace Star {

class InspectableEntity : public virtual Entity {
public:
  // Default implementation returns true
  [[nodiscard]] virtual auto inspectable() const -> bool;

  // If this entity can be entered into the player log, will return the log
  // identifier.
  [[nodiscard]] virtual auto inspectionLogName() const -> std::optional<String>;

  // Long description to display when inspected, if any
  [[nodiscard]] virtual auto inspectionDescription(String const& species) const -> std::optional<String>;
};

inline auto InspectableEntity::inspectable() const -> bool {
  return true;
}

inline auto InspectableEntity::inspectionLogName() const -> std::optional<String> {
  return {};
}

inline auto InspectableEntity::inspectionDescription(String const&) const -> std::optional<String> {
  return {};
}

}// namespace Star
