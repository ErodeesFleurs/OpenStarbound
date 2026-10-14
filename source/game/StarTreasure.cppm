module;
import std;
import star.string;
import star.list;
import star.variant;
import star.format;
import star.json;

import star.parametric_function;
import star.random;
import star.weighted_pool;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;
import star.vector;
import star.game_types;

namespace Star {

class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<const Item>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<const Item>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<const Item>;
class ItemBag;
using ItemBagPtr = std::shared_ptr<ItemBag>;
using ItemBagConstPtr = std::shared_ptr<const ItemBag>;
using ItemBagWeakPtr = std::weak_ptr<ItemBag>;
using ItemBagConstWeakPtr = std::weak_ptr<const ItemBag>;
using ItemBagUPtr = std::unique_ptr<ItemBag>;
using ItemBagConstUPtr = std::unique_ptr<const ItemBag>;
class ContainerObject;
using ContainerObjectPtr = std::shared_ptr<ContainerObject>;
using ContainerObjectConstPtr = std::shared_ptr<const ContainerObject>;
using ContainerObjectWeakPtr = std::weak_ptr<ContainerObject>;
using ContainerObjectConstWeakPtr = std::weak_ptr<const ContainerObject>;
using ContainerObjectUPtr = std::unique_ptr<ContainerObject>;
using ContainerObjectConstUPtr = std::unique_ptr<const ContainerObject>;
class TreasureDatabase;
using TreasureDatabasePtr = std::shared_ptr<TreasureDatabase>;
using TreasureDatabaseConstPtr = std::shared_ptr<const TreasureDatabase>;
using TreasureDatabaseWeakPtr = std::weak_ptr<TreasureDatabase>;
using TreasureDatabaseConstWeakPtr = std::weak_ptr<const TreasureDatabase>;
using TreasureDatabaseUPtr = std::unique_ptr<TreasureDatabase>;
using TreasureDatabaseConstUPtr = std::unique_ptr<const TreasureDatabase>;

struct TreasureExceptionTag {
  static constexpr char const* name() { return "TreasureException"; }
};
using TreasureException = StarError<TreasureExceptionTag, StarException>;

class TreasureDatabase {
public:
  TreasureDatabase();

  StringList treasurePools() const;
  bool isTreasurePool(String const& treasurePool) const;

  StringList treasureChestSets() const;
  bool isTreasureChestSet(String const& treasurePool) const;

  List<ItemPtr> createTreasure(String const& treasurePool, float level) const;
  List<ItemPtr> createTreasure(String const& treasurePool, float level, std::uint64_t seed) const;

  // Adds created treasure to the given ItemBags, does not clear the ItemBag
  // first.  Returns overflow items.
  List<ItemPtr> fillWithTreasure(ItemBagPtr const& itemBag, String const& treasurePool, float level) const;
  List<ItemPtr> fillWithTreasure(ItemBagPtr const& itemBag, String const& treasurePool, float level, std::uint64_t seed) const;

  // If the given container does not fit at this position, or if the treasure
  // box set does not have an entry with a minimum level less than the given
  // world threat level, this method will return null.
  ContainerObjectPtr createTreasureChest(World* world, String const& treasureChestSet, Vec2I const& position, Direction direction) const;
  ContainerObjectPtr createTreasureChest(World* world, String const& treasureChestSet, Vec2I const& position, Direction direction, std::uint64_t seed) const;

private:
  List<ItemPtr> createTreasure(String const& treasurePool, float level, std::uint64_t seed, StringSet visitedPools) const;

  // Specifies either an item descriptor or the name of a valid treasurepool to
  // be
  // used when an entry is selected in a "fill" or "pool" list
  typedef MVariant<String, ItemDescriptor> TreasureEntry;

  struct ItemPool {
    ItemPool();

    // If non-empty, the treasure set is pre-filled with this before selecting
    // from the pool.
    List<TreasureEntry> fill;

    // Weighted pool of items to select from.
    WeightedPool<TreasureEntry> pool;

    // Weighted pool for the number of pool rounds.
    WeightedPool<int> poolRounds;

    // Any item levels that are applied will have a random value
    // from this range added to their level.
    Vec2F levelVariance;

    // When generating more than one item, should we allow each cycle to
    // generate an item that is stackable with a previous item?  This is not to
    // say a stack could actually be formed in an ItemBag, simply that the
    // Item::stackableWith method returns true.
    // Note that this flag does not apply to child pools
    bool allowDuplication;
  };
  typedef ParametricTable<float, ItemPool> TreasurePool;

  struct TreasureChest {
    TreasureChest();

    StringList containers;
    String treasurePool;
    float minimumLevel;
  };
  typedef List<TreasureChest> TreasureChestSet;

  StringMap<TreasurePool> m_treasurePools;
  StringMap<TreasureChestSet> m_treasureChestSets;
};

}

export module star.treasure;

export namespace Star {
  using ::Star::TreasureExceptionTag;
  using ::Star::TreasureException;
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::ItemBag;
  using ::Star::ItemBagPtr;
  using ::Star::ItemBagConstPtr;
  using ::Star::ItemBagWeakPtr;
  using ::Star::ItemBagConstWeakPtr;
  using ::Star::ItemBagUPtr;
  using ::Star::ItemBagConstUPtr;
  using ::Star::ContainerObject;
  using ::Star::ContainerObjectPtr;
  using ::Star::ContainerObjectConstPtr;
  using ::Star::ContainerObjectWeakPtr;
  using ::Star::ContainerObjectConstWeakPtr;
  using ::Star::ContainerObjectUPtr;
  using ::Star::ContainerObjectConstUPtr;
  using ::Star::TreasureDatabase;
  using ::Star::TreasureDatabasePtr;
  using ::Star::TreasureDatabaseConstPtr;
  using ::Star::TreasureDatabaseWeakPtr;
  using ::Star::TreasureDatabaseConstWeakPtr;
  using ::Star::TreasureDatabaseUPtr;
  using ::Star::TreasureDatabaseConstUPtr;
}
