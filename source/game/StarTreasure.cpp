import star.json;
#include "StarJsonExtra.hpp"
import star.id_map;
import star.variant;
#include "StarCasting.hpp"
import star.vector;
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.list;
import star.rpc_promise;
import star.set;
import star.string;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;
import star.random;
import star.weighted_pool;
import star.periodic;
import star.periodic_function;
import star.lua;
import star.audio;
import star.map;
import star.maybe;

import star.drawable;
import star.item;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;
import star.container_entity;
import star.item_recipe;
import star.container_object;


import star.object_database;
import star.item_database;
import star.treasure;

import star.item_bag;

import star.static_random;
import star.algorithm;
namespace Star {

TreasureDatabase::TreasureDatabase() {
  auto assets = Root::singleton().assets();

  auto& treasurePools = assets->scanExtension("treasurepools");
  auto& treasureChests = assets->scanExtension("treasurechests");

  assets->queueJsons(treasurePools);
  assets->queueJsons(treasureChests);

  for (auto& file : treasurePools) {
    for (auto const& pair : assets->json(file).iterateObject()) {
      if (m_treasurePools.contains(pair.first))
        throw TreasureException(strf("Duplicate TreasurePool config '{}' from file '{}'", pair.first, file));

      auto& treasurePool = m_treasurePools[pair.first];
      for (auto const& entry : pair.second.iterateArray()) {
        if (entry.size() != 2)
          throw TreasureException("Wrong size for TreasurePool entry, list must be 2");

        float startLevel = entry.getFloat(0);
        auto config = entry.get(1);

        ItemPool itemPool;

        for (auto const& entry : config.getArray("fill", {}))
          if (entry.contains("pool"))
            itemPool.fill.append(entry.getString("pool"));
          else if (entry.contains("item"))
            itemPool.fill.append(ItemDescriptor(entry.get("item")));
          else
            throw TreasureException(strf("TreasurePool entry '{}' did not specify a valid 'item' or 'pool'", entry));

        for (auto const& entry : config.getArray("pool", {})) {
          if (!entry.contains("weight"))
            throw TreasureException(strf("TreasurePool entry '{}' did not specify a weight", entry));

          if (entry.contains("pool"))
            itemPool.pool.add(entry.getFloat("weight"), entry.getString("pool"));
          else if (entry.contains("item"))
            itemPool.pool.add(entry.getFloat("weight"), ItemDescriptor(entry.get("item")));
          else
            throw TreasureException(strf("TreasurePool entry '{}' did not specify a valid 'item' or 'pool'", entry));
        }

        auto poolRounds = config.get("poolRounds", 1);
        if (poolRounds.canConvert(Json::Type::Float)) {
          itemPool.poolRounds = WeightedPool<int>(List<std::pair<double, int>>{{1.0, poolRounds.toFloat()}});
        } else {
          for (auto const& pair : poolRounds.iterateArray())
            itemPool.poolRounds.add(pair.getDouble(0), pair.getInt(1));
        }

        itemPool.levelVariance = jsonToVec2F(config.get("levelVariance", JsonArray{0, 0}));
        itemPool.allowDuplication = config.getBool("allowDuplication", true);

        treasurePool.addPoint(startLevel, std::move(itemPool));
      }
    }
  }

  for (auto& file : treasureChests) {
    for (auto const& pair : assets->json(file).iterateObject()) {
      if (m_treasureChestSets.contains(pair.first))
        throw TreasureException(strf("Duplicate TreasureChestSet config '{}' from file '{}'", pair.first, file));

      auto& treasureChestSet = m_treasureChestSets[pair.first];
      for (auto const& entry : pair.second.iterateArray()) {
        TreasureChest treasureChest;

        treasureChest.containers = jsonToStringList(entry.get("containers"));
        treasureChest.treasurePool = entry.getString("treasurePool");
        treasureChest.minimumLevel = entry.getFloat("minimumLevel", 0);

        if (!m_treasurePools.contains(treasureChest.treasurePool))
          throw TreasureException(strf("No such TreasurePool '{}' for TreasureChestSet named '{}' in file '{}'", treasureChest.treasurePool, pair.first, file));

        treasureChestSet.append(treasureChest);
      }
    }
  }
}

StringList TreasureDatabase::treasurePools() const {
  return m_treasurePools.keys();
}

bool TreasureDatabase::isTreasurePool(String const& treasurePool) const {
  return m_treasurePools.contains(treasurePool);
}

StringList TreasureDatabase::treasureChestSets() const {
  return m_treasureChestSets.keys();
}

bool TreasureDatabase::isTreasureChestSet(String const& treasurePool) const {
  return m_treasureChestSets.contains(treasurePool);
}

TreasureDatabase::ItemPool::ItemPool() : allowDuplication() {}

TreasureDatabase::TreasureChest::TreasureChest() : minimumLevel() {}

List<std::shared_ptr<Item>> TreasureDatabase::createTreasure(String const& treasurePool, float level) const {
  return createTreasure(treasurePool, level, Random::randu64());
}

List<std::shared_ptr<Item>> TreasureDatabase::createTreasure(String const& treasurePool, float level, std::uint64_t seed) const {
  return createTreasure(treasurePool, level, seed, StringSet());
}

List<std::shared_ptr<Item>> TreasureDatabase::createTreasure(String const& treasurePool, float level, std::uint64_t seed, StringSet visitedPools) const {
  if (!m_treasurePools.contains(treasurePool))
    throw TreasureException(strf("Unknown treasure pool '{}'", treasurePool));

  if (!visitedPools.add(treasurePool))
    throw TreasureException(strf("Loop detected in treasure pool generation - set '{}' already contains '{}'", visitedPools, treasurePool));

  auto itemDatabase = Root::singleton().itemDatabase();

  List<std::shared_ptr<Item>> treasureItems;
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
      auto fillItem = itemDatabase->item(fillEntry.get<ItemDescriptor>(), itemLevel, seed + ++mix);
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
          treasureItems.append(itemDatabase->item(roundItem, itemLevel, seed + ++mix));
      }
    }
  }

  return treasureItems;
}

List<std::shared_ptr<Item>> TreasureDatabase::fillWithTreasure(
    std::shared_ptr<ItemBag> const& itemBag, String const& treasurePool, float level) const {
  return fillWithTreasure(itemBag, treasurePool, level, Random::randu64());
}

List<std::shared_ptr<Item>> TreasureDatabase::fillWithTreasure(
    std::shared_ptr<ItemBag> const& itemBag, String const& treasurePool, float level, std::uint64_t seed) const {
  List<std::shared_ptr<Item>> overflowItems;
  for (auto const& treasureItem : createTreasure(treasurePool, level, seed)) {
    if (auto overflow = itemBag->addItems(treasureItem))
      overflowItems.append(overflow);
  }

  return overflowItems;
}

std::shared_ptr<ContainerObject> TreasureDatabase::createTreasureChest(World* world, String const& treasureChestSet, Vec2I const& position, Direction direction) const {
  return createTreasureChest(world, treasureChestSet, position, direction, Random::randu64());
}

std::shared_ptr<ContainerObject> TreasureDatabase::createTreasureChest(World* world, String const& treasureChestSet, Vec2I const& position, Direction direction, std::uint64_t seed) const {
  auto objectDatabase = Root::singleton().objectDatabase();

  if (!m_treasureChestSets.contains(treasureChestSet))
    throw StarException(strf("Unknown treasure chest set '{}'", treasureChestSet));

  auto level = world->threatLevel();
  auto boxSet = m_treasureChestSets.get(treasureChestSet);
  eraseWhere(boxSet, [level](TreasureChest const& treasureChest) { return level < treasureChest.minimumLevel; });

  if (boxSet.empty())
    return {};

  auto const& treasureChest = staticRandomFrom(boxSet, seed, "TreasureChest");
  auto const& containerName = staticRandomFrom(treasureChest.containers, seed, "ContainerName");
  std::shared_ptr<ContainerObject> containerObject;
  auto parameters = JsonObject{{"treasurePools", JsonArray{treasureChest.treasurePool}}, {"treasureSeed", seed}};
  if (auto object = objectDatabase->createForPlacement(world, containerName, position, direction, parameters))
    containerObject = convert<ContainerObject>(object);

  return containerObject;
}

}
