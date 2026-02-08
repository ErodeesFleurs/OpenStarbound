#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarParametricFunction.hpp"
#include "StarWeightedPool.hpp"

import std;

namespace Star {

class ItemBag;
class ContainerObject;
class World;

using TreasureException = ExceptionDerived<"TreasureException">;

class TreasureDatabase {
public:
  TreasureDatabase();

  [[nodiscard]] auto treasurePools() const -> StringList;
  [[nodiscard]] auto isTreasurePool(String const& treasurePool) const -> bool;

  [[nodiscard]] auto treasureChestSets() const -> StringList;
  [[nodiscard]] auto isTreasureChestSet(String const& treasurePool) const -> bool;

  [[nodiscard]] auto createTreasure(String const& treasurePool, float level) const -> List<Ptr<Item>>;
  [[nodiscard]] auto createTreasure(String const& treasurePool, float level, std::uint64_t seed) const -> List<Ptr<Item>>;

  // Adds created treasure to the given ItemBags, does not clear the ItemBag
  // first.  Returns overflow items.
  [[nodiscard]] auto fillWithTreasure(Ptr<ItemBag> const& itemBag, String const& treasurePool, float level) const -> List<Ptr<Item>>;
  [[nodiscard]] auto fillWithTreasure(Ptr<ItemBag> const& itemBag, String const& treasurePool, float level, std::uint64_t seed) const -> List<Ptr<Item>>;

  // If the given container does not fit at this position, or if the treasure
  // box set does not have an entry with a minimum level less than the given
  // world threat level, this method will return null.
  auto createTreasureChest(World* world, String const& treasureChestSet, Vec2I const& position, Direction direction) const -> Ptr<ContainerObject>;
  auto createTreasureChest(World* world, String const& treasureChestSet, Vec2I const& position, Direction direction, std::uint64_t seed) const -> Ptr<ContainerObject>;

private:
  [[nodiscard]] auto createTreasure(String const& treasurePool, float level, std::uint64_t seed, StringSet visitedPools) const -> List<Ptr<Item>>;

  // Specifies either an item descriptor or the name of a valid treasurepool to
  // be
  // used when an entry is selected in a "fill" or "pool" list
  using TreasureEntry = MVariant<String, ItemDescriptor>;

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
  using TreasurePool = ParametricTable<float, ItemPool>;

  struct TreasureChest {
    TreasureChest();

    StringList containers;
    String treasurePool;
    float minimumLevel;
  };
  using TreasureChestSet = List<TreasureChest>;

  StringMap<TreasurePool> m_treasurePools;
  StringMap<TreasureChestSet> m_treasureChestSets;
};

}// namespace Star
