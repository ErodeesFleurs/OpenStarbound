#include "StarPlayerBlueprints.hpp"

#include "StarItemDescriptor.hpp"

import std;

namespace Star {

PlayerBlueprints::PlayerBlueprints() = default;

PlayerBlueprints::PlayerBlueprints(Json const& variant) {
  m_knownBlueprints =
    transform<HashSet<ItemDescriptor>>(variant.get("knownBlueprints").toArray(), construct<ItemDescriptor>());
  m_newBlueprints =
    transform<HashSet<ItemDescriptor>>(variant.get("newBlueprints").toArray(), construct<ItemDescriptor>());
}

auto PlayerBlueprints::toJson() const -> Json {
  return JsonObject{{"knownBlueprints", transform<JsonArray>(m_knownBlueprints, std::mem_fn(&ItemDescriptor::toJson))},
                    {"newBlueprints", transform<JsonArray>(m_newBlueprints, std::mem_fn(&ItemDescriptor::toJson))}};
}

auto PlayerBlueprints::isKnown(ItemDescriptor const& itemDescriptor) const -> bool {
  return m_knownBlueprints.contains(itemDescriptor.singular());
}

auto PlayerBlueprints::isNew(ItemDescriptor const& itemDescriptor) const -> bool {
  return m_newBlueprints.contains(itemDescriptor.singular());
}

void PlayerBlueprints::add(ItemDescriptor const& itemDescriptor) {
  if (m_knownBlueprints.add(itemDescriptor.singular()))
    m_newBlueprints.add(itemDescriptor.singular());
}

void PlayerBlueprints::markAsRead(ItemDescriptor const& itemDescriptor) {
  m_newBlueprints.remove(itemDescriptor.singular());
}

}// namespace Star
