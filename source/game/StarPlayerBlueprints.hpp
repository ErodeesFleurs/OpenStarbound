#pragma once

#include "StarItemDescriptor.hpp"
#include "StarSet.hpp"

namespace Star {

class PlayerBlueprints {
public:
  PlayerBlueprints();
  PlayerBlueprints(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto isKnown(ItemDescriptor const& itemDescriptor) const -> bool;
  [[nodiscard]] auto isNew(ItemDescriptor const& itemDescriptor) const -> bool;
  void add(ItemDescriptor const& itemDescriptor);
  void markAsRead(ItemDescriptor const& itemDescriptor);

private:
  HashSet<ItemDescriptor> m_knownBlueprints;
  HashSet<ItemDescriptor> m_newBlueprints;
};

}// namespace Star
