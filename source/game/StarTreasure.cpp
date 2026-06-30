#include "StarTreasure.hpp"
#include "StarObjectDatabase.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemBag.hpp"
#include "StarWorld.hpp"
#include "StarContainerObject.hpp"
#include "StarJsonExtra.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

TreasureDatabase::TreasureDatabase(AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase)
  : m_itemDatabase(requireServiceValueAs<TreasureException>(std::move(itemDatabase), "TreasureDatabase", "item database")),
    m_objectDatabase(requireServiceValueAs<TreasureException>(std::move(objectDatabase), "TreasureDatabase", "object database")) {
  assets = requireServiceValueAs<TreasureException>(std::move(assets), "TreasureDatabase", "assets");

  auto& treasurePools = assets->scanExtension("treasurepools");
  auto& treasureChests = assets->scanExtension("treasurechests");

  assets->queueJsons(treasurePools);
  assets->queueJsons(treasureChests);

  for (auto& file : treasurePools) {
    for (auto const& [poolName, poolConfig] : assets->json(file).iterateObject()) {
      if (m_treasurePools.contains(poolName))
        throw TreasureException(strf("Duplicate TreasurePool config '{}' from file '{}'", poolName, file));

      auto& treasurePool = m_treasurePools[poolName];
      for (auto const& poolEntry : poolConfig.iterateArray()) {
        if (poolEntry.size() != 2)
          throw TreasureException("Wrong size for TreasurePool entry, list must be 2");

        float startLevel = poolEntry.getFloat(0);
        auto config = poolEntry.get(1);

        ItemPool itemPool;

        for (auto const& fillEntry : config.getArray("fill", {}))
          if (fillEntry.contains("pool"))
            itemPool.fill.append(fillEntry.getString("pool"));
          else if (fillEntry.contains("item"))
            itemPool.fill.append(ItemDescriptor(fillEntry.get("item")));
          else
            throw TreasureException(strf("TreasurePool entry '{}' did not specify a valid 'item' or 'pool'", fillEntry));

        for (auto const& poolEntry : config.getArray("pool", {})) {
          if (!poolEntry.contains("weight"))
            throw TreasureException(strf("TreasurePool entry '{}' did not specify a weight", poolEntry));

          if (poolEntry.contains("pool"))
            itemPool.pool.add(poolEntry.getFloat("weight"), poolEntry.getString("pool"));
          else if (poolEntry.contains("item"))
            itemPool.pool.add(poolEntry.getFloat("weight"), ItemDescriptor(poolEntry.get("item")));
          else
            throw TreasureException(strf("TreasurePool entry '{}' did not specify a valid 'item' or 'pool'", poolEntry));
        }

        auto poolRounds = config.get("poolRounds", 1);
        if (poolRounds.canConvert(Json::Type::Float)) {
          itemPool.poolRounds = WeightedPool<int>(List<std::pair<double, int>>{{1.0, poolRounds.toFloat()}});
        } else {
          for (auto const& poolRound : poolRounds.iterateArray())
            itemPool.poolRounds.add(poolRound.getDouble(0), poolRound.getInt(1));
        }

        itemPool.levelVariance = jsonToVec2F(config.get("levelVariance", JsonArray{0, 0}));
        itemPool.allowDuplication = config.getBool("allowDuplication", true);

        treasurePool.addPoint(startLevel, std::move(itemPool));
      }
    }
  }

  for (auto& file : treasureChests) {
    for (auto const& [chestSetName, chestSetConfig] : assets->json(file).iterateObject()) {
      if (m_treasureChestSets.contains(chestSetName))
        throw TreasureException(strf("Duplicate TreasureChestSet config '{}' from file '{}'", chestSetName, file));

      auto& treasureChestSet = m_treasureChestSets[chestSetName];
      for (auto const& chestConfig : chestSetConfig.iterateArray()) {
        TreasureChest treasureChest;

        treasureChest.containers = jsonToStringList(chestConfig.get("containers"));
        treasureChest.treasurePool = chestConfig.getString("treasurePool");
        treasureChest.minimumLevel = chestConfig.getFloat("minimumLevel", 0);

        if (!m_treasurePools.contains(treasureChest.treasurePool))
          throw TreasureException(strf("No such TreasurePool '{}' for TreasureChestSet named '{}' in file '{}'", treasureChest.treasurePool, chestSetName, file));

        treasureChestSet.append(treasureChest);
      }
    }
  }
}

[[nodiscard]] StringList TreasureDatabase::treasurePools() const {
  return m_treasurePools.keys();
}

[[nodiscard]] bool TreasureDatabase::isTreasurePool(String const& treasurePool) const {
  return m_treasurePools.contains(treasurePool);
}

[[nodiscard]] StringList TreasureDatabase::treasureChestSets() const {
  return m_treasureChestSets.keys();
}

[[nodiscard]] bool TreasureDatabase::isTreasureChestSet(String const& treasurePool) const {
  return m_treasureChestSets.contains(treasurePool);
}

[[nodiscard]] List<ItemPtr> TreasureDatabase::createTreasure(String const& treasurePool, float level) const {
  return createTreasure(treasurePool, level, Random::randu64());
}

[[nodiscard]] List<ItemPtr> TreasureDatabase::createTreasure(String const& treasurePool, float level, uint64_t seed) const {
  return createTreasure(treasurePool, level, seed, StringSet());
}

[[nodiscard]] List<ItemPtr> TreasureDatabase::createTreasure(String const& treasurePool, float level, uint64_t seed, StringSet visitedPools) const {
  if (!m_treasurePools.contains(treasurePool))
    throw TreasureException(strf("Unknown treasure pool '{}'", treasurePool));

  if (!visitedPools.add(treasurePool))
    throw TreasureException(strf("Loop detected in treasure pool generation - set '{}' already contains '{}'", visitedPools, treasurePool));

  List<ItemPtr> treasureItems;
  HashSet<ItemDescriptor> previousDescriptors;
  auto itemPool = m_treasurePools.get(treasurePool).get(level);

  int mix = 0;
  for (auto const& fillEntry : itemPool.fill) {
    if (fillEntry.is<String>()) {
      auto poolContents = createTreasure(fillEntry.get<String>(), level, seed + ++mix, visitedPools);
      for (auto item : poolContents) {
        if (itemPool.allowDuplication || previousDescriptors.add(item->descriptor().singular()))
          treasureItems.append(item);
      }
    } else {
      float itemLevel = level + itemPool.levelVariance[0] + staticRandomFloat(seed, ++mix, "FillLevelVariance") * (itemPool.levelVariance[1] - itemPool.levelVariance[0]);
      auto fillItem = m_itemDatabase->item(fillEntry.get<ItemDescriptor>(), itemLevel, seed + ++mix);
      if (itemPool.allowDuplication || previousDescriptors.add(fillItem->descriptor().singular()))
        treasureItems.append(fillItem);
    }
  }

  if (!itemPool.pool.empty()) {
    int poolRounds = itemPool.poolRounds.select(staticRandomU64(seed, "TreasurePoolRounds"));

    for (int i = 0; i < poolRounds; ++i) {
      auto poolEntry = itemPool.pool.select(staticRandomU64(seed, i, "TreasureItem"));

      if (poolEntry.is<String>()) {
        auto poolContents = createTreasure(poolEntry.get<String>(), level, staticRandomU64(seed, i, "TreasureSeedRecursion"), visitedPools);
        for (auto item : poolContents) {
          if (itemPool.allowDuplication || previousDescriptors.add(item->descriptor().singular()))
            treasureItems.append(item);
        }
      } else {
        float itemLevel = level + itemPool.levelVariance[0] + staticRandomFloat(staticRandomU64(seed, i, "TreasureLevelSeedMixer"), "PoolLevelVariance") * (itemPool.levelVariance[1] - itemPool.levelVariance[0]);
        auto roundItem = poolEntry.get<ItemDescriptor>();
        if (itemPool.allowDuplication || previousDescriptors.add(roundItem.singular()))
          treasureItems.append(m_itemDatabase->item(roundItem, itemLevel, seed + ++mix));
      }
    }
  }

  return treasureItems;
}

[[nodiscard]] List<ItemPtr> TreasureDatabase::fillWithTreasure(
    ItemBagPtr const& itemBag, String const& treasurePool, float level) const {
  return fillWithTreasure(itemBag, treasurePool, level, Random::randu64());
}

[[nodiscard]] List<ItemPtr> TreasureDatabase::fillWithTreasure(
    ItemBagPtr const& itemBag, String const& treasurePool, float level, uint64_t seed) const {
  List<ItemPtr> overflowItems;
  for (auto const& treasureItem : createTreasure(treasurePool, level, seed)) {
    if (auto overflow = itemBag->addItems(treasureItem))
      overflowItems.append(overflow);
  }

  return overflowItems;
}

[[nodiscard]] ContainerObjectPtr TreasureDatabase::createTreasureChest(World& world, String const& treasureChestSet, Vec2I const& position, Direction direction) const {
  return createTreasureChest(world, treasureChestSet, position, direction, Random::randu64());
}

[[nodiscard]] ContainerObjectPtr TreasureDatabase::createTreasureChest(World& world, String const& treasureChestSet, Vec2I const& position, Direction direction, uint64_t seed) const {
  if (!m_treasureChestSets.contains(treasureChestSet))
    throw StarException(strf("Unknown treasure chest set '{}'", treasureChestSet));

  auto level = world.threatLevel();
  auto boxSet = m_treasureChestSets.get(treasureChestSet);
  eraseWhere(boxSet, [level](TreasureChest const& treasureChest) { return level < treasureChest.minimumLevel; });

  if (boxSet.empty())
    return {};

  auto const& treasureChest = staticRandomFrom(boxSet, seed, "TreasureChest");
  auto const& containerName = staticRandomFrom(treasureChest.containers, seed, "ContainerName");
  ContainerObjectPtr containerObject;
  auto parameters = JsonObject{{"treasurePools", JsonArray{treasureChest.treasurePool}}, {"treasureSeed", seed}};
  if (auto object = m_objectDatabase->createForPlacement(world, containerName, position, direction, parameters))
    containerObject = convert<ContainerObject>(object);

  return containerObject;
}

}
