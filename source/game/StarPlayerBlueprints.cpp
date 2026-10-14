import std;
import star.json;
import star.byte_array;
import star.set;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;

import star.player_blueprints;

import star.algorithm;
namespace Star {

PlayerBlueprints::PlayerBlueprints() {}

PlayerBlueprints::PlayerBlueprints(Json const& variant) {
  m_knownBlueprints =
      transform<HashSet<ItemDescriptor>>(variant.get("knownBlueprints").toArray(), construct<ItemDescriptor>());
  m_newBlueprints =
      transform<HashSet<ItemDescriptor>>(variant.get("newBlueprints").toArray(), construct<ItemDescriptor>());
}

Json PlayerBlueprints::toJson() const {
  return JsonObject{{"knownBlueprints", transform<JsonArray>(m_knownBlueprints, std::mem_fn(&ItemDescriptor::toJson))},
      {"newBlueprints", transform<JsonArray>(m_newBlueprints, std::mem_fn(&ItemDescriptor::toJson))}};
}

bool PlayerBlueprints::isKnown(ItemDescriptor const& itemDescriptor) const {
  return m_knownBlueprints.contains(itemDescriptor.singular());
}

bool PlayerBlueprints::isNew(ItemDescriptor const& itemDescriptor) const {
  return m_newBlueprints.contains(itemDescriptor.singular());
}

void PlayerBlueprints::add(ItemDescriptor const& itemDescriptor) {
  if (m_knownBlueprints.add(itemDescriptor.singular()))
    m_newBlueprints.add(itemDescriptor.singular());
}

void PlayerBlueprints::markAsRead(ItemDescriptor const& itemDescriptor) {
  m_newBlueprints.remove(itemDescriptor.singular());
}

}
