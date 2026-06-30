#pragma once

#include "StarAssets.hpp"
#include "StarThread.hpp"
#include "StarParametricFunction.hpp"
#include "StarWeightedPool.hpp"
#include "StarItemDescriptor.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class World;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class Item;
using ItemPtr = SharedPtr<Item>;
class ItemBag;
using ItemBagPtr = SharedPtr<ItemBag>;
class ContainerObject;
using ContainerObjectPtr = SharedPtr<ContainerObject>;
class TreasureDatabase;
using TreasureDatabasePtr = SharedPtr<TreasureDatabase>;
using TreasureDatabaseConstPtr = SharedPtr<TreasureDatabase const>;

struct TreasureExceptionTag { static constexpr char const* typeName = "TreasureException"; };
using TreasureException = TypedException<StarException, TreasureExceptionTag>;

class TreasureDatabase {
public:
  TreasureDatabase(AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase);

  [[nodiscard]] StringList treasurePools() const;
  [[nodiscard]] bool isTreasurePool(String const& treasurePool) const;

  [[nodiscard]] StringList treasureChestSets() const;
  [[nodiscard]] bool isTreasureChestSet(String const& treasurePool) const;

  [[nodiscard]] List<ItemPtr> createTreasure(String const& treasurePool, float level) const;
  [[nodiscard]] List<ItemPtr> createTreasure(String const& treasurePool, float level, uint64_t seed) const;

  // Adds created treasure to the given ItemBags, does not clear the ItemBag
  // first.  Returns overflow items.
  [[nodiscard]] List<ItemPtr> fillWithTreasure(ItemBagPtr const& itemBag, String const& treasurePool, float level) const;
  [[nodiscard]] List<ItemPtr> fillWithTreasure(ItemBagPtr const& itemBag, String const& treasurePool, float level, uint64_t seed) const;

  // If the given container does not fit at this position, or if the treasure
  // box set does not have an entry with a minimum level less than the given
  // world threat level, this method will return null.
  [[nodiscard]] ContainerObjectPtr createTreasureChest(World& world, String const& treasureChestSet, Vec2I const& position, Direction direction) const;
  [[nodiscard]] ContainerObjectPtr createTreasureChest(World& world, String const& treasureChestSet, Vec2I const& position, Direction direction, uint64_t seed) const;

private:
  [[nodiscard]] List<ItemPtr> createTreasure(String const& treasurePool, float level, uint64_t seed, StringSet visitedPools) const;

  // Specifies either an item descriptor or the name of a valid treasurepool to
  // be
  // used when an entry is selected in a "fill" or "pool" list
  using TreasureEntry = MVariant<String, ItemDescriptor>;

  struct ItemPool {
    ItemPool() = default;

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
    bool allowDuplication{};
  };
  using TreasurePool = ParametricTable<float, ItemPool>;

  struct TreasureChest {
    TreasureChest() = default;

    StringList containers;
    String treasurePool;
    float minimumLevel{};
  };
  using TreasureChestSet = List<TreasureChest>;

  StringMap<TreasurePool> m_treasurePools;
  StringMap<TreasureChestSet> m_treasureChestSets;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
};

}
