module;

import std;
import star.json;
import star.byte_array;
import star.set;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;

namespace Star {

class PlayerBlueprints;
using PlayerBlueprintsPtr = std::shared_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstPtr = std::shared_ptr<const PlayerBlueprints>;
using PlayerBlueprintsWeakPtr = std::weak_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstWeakPtr = std::weak_ptr<const PlayerBlueprints>;
using PlayerBlueprintsUPtr = std::unique_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstUPtr = std::unique_ptr<const PlayerBlueprints>;

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
