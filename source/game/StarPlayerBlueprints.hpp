#pragma once

#include "StarByteArray.hpp"
#include "StarSet.hpp"
#include "StarItemDescriptor.hpp"

namespace Star {

class PlayerBlueprints;
using PlayerBlueprintsPtr = SharedPtr<PlayerBlueprints>;

class PlayerBlueprints {
public:
  PlayerBlueprints() = default;
  PlayerBlueprints(Json const& json);

  [[nodiscard]] Json toJson() const;

  [[nodiscard]] bool isKnown(ItemDescriptor const& itemDescriptor) const;
  [[nodiscard]] bool isNew(ItemDescriptor const& itemDescriptor) const;
  void add(ItemDescriptor const& itemDescriptor);
  void markAsRead(ItemDescriptor const& itemDescriptor);

private:
  HashSet<ItemDescriptor> m_knownBlueprints;
  HashSet<ItemDescriptor> m_newBlueprints;
};

}
