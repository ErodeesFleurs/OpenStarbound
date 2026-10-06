module;

#include "StarByteArray.hpp"
#include "StarSet.hpp"
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
import star.item_descriptor;

namespace Star {

STAR_CLASS(PlayerBlueprints);

class PlayerBlueprints {
public:
  PlayerBlueprints();
  PlayerBlueprints(Json const& json);

  Json toJson() const;

  bool isKnown(ItemDescriptor const& itemDescriptor) const;
  bool isNew(ItemDescriptor const& itemDescriptor) const;
  void add(ItemDescriptor const& itemDescriptor);
  void markAsRead(ItemDescriptor const& itemDescriptor);

private:
  HashSet<ItemDescriptor> m_knownBlueprints;
  HashSet<ItemDescriptor> m_newBlueprints;
};

}

export module star.player_blueprints;

export namespace Star {
  using ::Star::PlayerBlueprints;
  using ::Star::PlayerBlueprintsPtr;
  using ::Star::PlayerBlueprintsConstPtr;
  using ::Star::PlayerBlueprintsWeakPtr;
  using ::Star::PlayerBlueprintsConstWeakPtr;
  using ::Star::PlayerBlueprintsUPtr;
  using ::Star::PlayerBlueprintsConstUPtr;
}
