#include "StarWorldLuaBindings.hpp"

#include "StarBiome.hpp"
#include "StarBlocksAlongLine.hpp"
#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarContainerObject.hpp"
#include "StarFarmableObject.hpp"
#include "StarItem.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemDrop.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"
#include "StarLoungeableObject.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarMonster.hpp"
#include "StarNpc.hpp"
#include "StarObjectDatabase.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerInventory.hpp"// IWYU pragma: export
#include "StarProjectileDatabase.hpp"
#include "StarRoot.hpp"
#include "StarSky.hpp"// IWYU pragma: export
#include "StarStagehand.hpp"
#include "StarStagehandDatabase.hpp"
#include "StarTreasure.hpp"
#include "StarUniverseSettings.hpp"// IWYU pragma: export
#include "StarUtilityLuaBindings.hpp"
#include "StarVehicle.hpp"
#include "StarVehicleDatabase.hpp"
#include "StarWorld.hpp"
#include "StarWorldClient.hpp"
#include "StarWorldParameters.hpp"
#include "StarWorldServer.hpp"
#include "StarWorldTemplate.hpp"

import std;

namespace Star::LuaBindings {

enum class EntityBoundMode { MetaBoundBox,
                             CollisionArea,
                             Position };

EnumMap<EntityBoundMode> const EntityBoundModeNames = {
  {EntityBoundMode::MetaBoundBox, "MetaBoundBox"},
  {EntityBoundMode::CollisionArea, "CollisionArea"},
  {EntityBoundMode::Position, "Position"}};

template <typename EntityT>
using Selector = std::function<bool(std::shared_ptr<EntityT> const&)>;

template <typename EntityT>
auto entityQueryImpl(World* world, LuaEngine& engine, LuaTable const& options, Selector<EntityT> selector) -> LuaTable {
  auto withoutEntityId = options.get<std::optional<EntityId>>("withoutEntityId");
  std::optional<Set<EntityType>> includedTypes;
  if (auto types = options.get<std::optional<LuaTable>>("includedTypes")) {
    includedTypes = Set<EntityType>();
    types->iterate([&includedTypes](LuaValue const&, LuaString const& type) -> auto {
      if (type == "mobile") {
        includedTypes->add(EntityType::Player);
        includedTypes->add(EntityType::Monster);
        includedTypes->add(EntityType::Npc);
        includedTypes->add(EntityType::Projectile);
        includedTypes->add(EntityType::ItemDrop);
        includedTypes->add(EntityType::Vehicle);
      } else if (type == "creature") {
        includedTypes->add(EntityType::Player);
        includedTypes->add(EntityType::Monster);
        includedTypes->add(EntityType::Npc);
      } else {
        includedTypes->add(EntityTypeNames.getLeft(type.ptr()));
      }
    });
  }

  auto callScript = options.get<std::optional<String>>("callScript");
  List<LuaValue> callScriptArgs = options.get<std::optional<List<LuaValue>>>("callScriptArgs").value();
  LuaValue callScriptResult = options.get<std::optional<LuaValue>>("callScriptResult").value_or(LuaBoolean(true));

  auto lineQuery = options.get<std::optional<Line2F>>("line");
  auto polyQuery = options.get<std::optional<PolyF>>("poly");
  auto rectQuery = options.get<std::optional<RectF>>("rect");
  std::optional<std::pair<Vec2F, float>> radiusQuery;
  if (auto radius = options.get<std::optional<float>>("radius"))
    radiusQuery = make_pair(options.get<Vec2F>("center"), *radius);

  EntityBoundMode boundMode = EntityBoundModeNames.getLeft(options.get<std::optional<String>>("boundMode").value_or("CollisionArea"));
  auto order = options.get<std::optional<LuaString>>("order");

  auto geometry = world->geometry();

  auto innerSelector = [=](std::shared_ptr<EntityT> const& entity) -> bool {
    if (selector && !selector(entity))
      return false;

    if (includedTypes && !includedTypes->contains(entity->entityType()))
      return false;

    if (withoutEntityId && entity->entityId() == *withoutEntityId)
      return false;

    if (callScript) {
      auto scriptedEntity = as<ScriptedEntity>(entity);
      if (!scriptedEntity || !scriptedEntity->isMaster())
        return false;

      auto res = scriptedEntity->callScript(*callScript, luaUnpack(callScriptArgs));
      if (!res || *res != callScriptResult)
        return false;
    }

    auto position = entity->position();
    if (boundMode == EntityBoundMode::MetaBoundBox) {
      // If using MetaBoundBox, the regular line / box query methods already
      // enforce collision with MetaBoundBox
      if (radiusQuery)
        return geometry.rectIntersectsCircle(
          entity->metaBoundBox().translated(position), radiusQuery->first, radiusQuery->second);
    } else if (boundMode == EntityBoundMode::CollisionArea) {
      // Collision area queries either query based on the collision area if
      // that's given, or as a fallback the regular bound box.
      auto collisionArea = entity->collisionArea();
      if (collisionArea.isNull())
        collisionArea = entity->metaBoundBox();
      collisionArea.translate(position);

      if (lineQuery)
        return geometry.lineIntersectsRect(*lineQuery, collisionArea);
      if (polyQuery)
        return geometry.polyIntersectsPoly(*polyQuery, PolyF(collisionArea));
      if (rectQuery)
        return geometry.rectIntersectsRect(*rectQuery, collisionArea);
      if (radiusQuery)
        return geometry.rectIntersectsCircle(collisionArea, radiusQuery->first, radiusQuery->second);
    } else if (boundMode == EntityBoundMode::Position) {
      if (lineQuery)
        return geometry.lineIntersectsRect(*lineQuery, RectF(position, position));
      if (polyQuery)
        return geometry.polyContains(*polyQuery, position);
      if (rectQuery)
        return geometry.rectContains(*rectQuery, position);
      if (radiusQuery)
        return geometry.diff(radiusQuery->first, position).magnitude() <= radiusQuery->second;
    }

    return true;
  };

  List<std::shared_ptr<EntityT>> entities;

  if (lineQuery) {
    entities = world->lineQuery<EntityT>(lineQuery->min(), lineQuery->max(), innerSelector);
  } else if (polyQuery) {
    entities = world->query<EntityT>(polyQuery->boundBox(), innerSelector);
  } else if (rectQuery) {
    entities = world->query<EntityT>(*rectQuery, innerSelector);
  } else if (radiusQuery) {
    RectF region(radiusQuery->first - Vec2F::filled(radiusQuery->second),
                 radiusQuery->first + Vec2F::filled(radiusQuery->second));
    entities = world->query<EntityT>(region, innerSelector);
  }

  if (order) {
    if (*order == "nearest") {
      Vec2F nearestPosition;
      if (lineQuery)
        nearestPosition = lineQuery->min();
      else if (polyQuery)
        nearestPosition = polyQuery->center();
      else if (rectQuery)
        nearestPosition = rectQuery->center();
      else if (radiusQuery)
        nearestPosition = radiusQuery->first;
      sortByComputedValue(entities,
                          [world, nearestPosition](std::shared_ptr<EntityT> const& entity) -> auto {
                            return world->geometry().diff(entity->position(), nearestPosition).magnitude();
                          });
    } else if (*order == "random") {
      Random::shuffle(entities);
    } else {
      throw StarException(strf("Unsupported query order {}", order->ptr()));
    }
  }

  LuaTable entityIds = engine.createTable();
  int entityIdsIndex = 1;
  for (auto entity : entities)
    entityIds.set(entityIdsIndex++, entity->entityId());

  return entityIds;
}

template <typename EntityT>
auto entityQuery(World* world,
                 LuaEngine& engine,
                 Vec2F const& pos1,
                 LuaValue const& pos2,
                 std::optional<LuaTable> options,
                 Selector<EntityT> selector = {}) -> LuaTable {
  if (!options)
    options = engine.createTable();

  if (auto radius = engine.luaMaybeTo<float>(pos2)) {
    Vec2F center = pos1;
    options->set("center", center);
    options->set("radius", *radius);
    return entityQueryImpl<EntityT>(world, engine, *options, selector);
  } else {
    RectF rect(pos1, engine.luaTo<Vec2F>(pos2));
    options->set("rect", rect);
    return entityQueryImpl<EntityT>(world, engine, *options, selector);
  }
}

template <typename EntityT>
auto entityLineQuery(World* world,
                     LuaEngine& engine,
                     Vec2F const& point1,
                     Vec2F const& point2,
                     std::optional<LuaTable> options,
                     Selector<EntityT> selector = {}) -> LuaTable {
  Line2F line(point1, point2);

  if (!options)
    options = engine.createTable();

  options->set("line", line);

  return entityQueryImpl<EntityT>(world, engine, *options, selector);
}

auto makeWorldCallbacks(World* world) -> LuaCallbacks {
  LuaCallbacks callbacks;

  addWorldDebugCallbacks(callbacks);
  addWorldEnvironmentCallbacks(callbacks, world);
  addWorldEntityCallbacks(callbacks, world);

  callbacks.registerCallbackWithSignature<float, Vec2F, std::optional<Vec2F>>("magnitude", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::magnitude(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Vec2F, Vec2F, Vec2F>("distance", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::distance(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, PolyF, Vec2F>("polyContains", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::polyContains(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, LuaValue>("xwrap", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::xwrap(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, Variant<Vec2F, float>, Variant<Vec2F, float>>("nearestTo", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::nearestTo(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });

  callbacks.registerCallbackWithSignature<bool, RectF, std::optional<CollisionSet>>("rectCollision", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::rectCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F, std::optional<CollisionSet>>("pointTileCollision", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::pointTileCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F, Vec2F, std::optional<CollisionSet>>("lineTileCollision", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::lineTileCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<std::pair<Vec2F, Vec2I>>, Vec2F, Vec2F, std::optional<CollisionSet>>("lineTileCollisionPoint", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::lineTileCollisionPoint(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<bool, RectF, std::optional<CollisionSet>>("rectTileCollision", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::rectTileCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F, std::optional<CollisionSet>>("pointCollision", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::pointCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<LuaTupleReturn<std::optional<Vec2F>, std::optional<Vec2F>>, Vec2F, Vec2F, std::optional<CollisionSet>>("lineCollision", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::lineCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<bool, PolyF, std::optional<Vec2F>, std::optional<CollisionSet>>("polyCollision", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::polyCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<List<Vec2I>, Vec2F, Vec2F, std::optional<CollisionSet>, std::optional<int>>("collisionBlocksAlongLine", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldCallbacks::collisionBlocksAlongLine(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<List<std::pair<Vec2I, LiquidLevel>>, Vec2F, Vec2F>("liquidAlongLine", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::liquidAlongLine(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<Vec2F>, PolyF, Vec2F, float, std::optional<CollisionSet>>("resolvePolyCollision", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldCallbacks::resolvePolyCollision(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<bool, Vec2I, std::optional<bool>, std::optional<bool>>("tileIsOccupied", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::tileIsOccupied(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<bool, String, Vec2I, std::optional<int>, Json>("placeObject", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldCallbacks::placeObject(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<std::optional<EntityId>, Json, Vec2F, std::optional<size_t>, Json, std::optional<Vec2F>, std::optional<float>>("spawnItem", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6) -> auto { return WorldCallbacks::spawnItem(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6)); });
  callbacks.registerCallbackWithSignature<List<EntityId>, Vec2F, String, float, std::optional<uint64_t>>("spawnTreasure", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldCallbacks::spawnTreasure(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<std::optional<EntityId>, String, Vec2F, std::optional<JsonObject>>("spawnMonster", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::spawnMonster(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<EntityId>, Vec2F, String, String, float, std::optional<uint64_t>, Json>("spawnNpc", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6) -> auto { return WorldCallbacks::spawnNpc(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6)); });
  callbacks.registerCallbackWithSignature<std::optional<EntityId>, Vec2F, String, Json>("spawnStagehand", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::spawnStagehand(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<EntityId>, String, Vec2F, std::optional<EntityId>, std::optional<Vec2F>, bool, Json>("spawnProjectile", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6) -> auto { return WorldCallbacks::spawnProjectile(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6)); });
  callbacks.registerCallbackWithSignature<std::optional<EntityId>, String, Vec2F, Json>("spawnVehicle", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::spawnVehicle(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<float>("threatLevel", [world] -> float { return world->threatLevel(); });
  callbacks.registerCallbackWithSignature<double>("time", [world] -> double { return WorldCallbacks::time(world); });
  callbacks.registerCallbackWithSignature<uint64_t>("day", [world] -> uint64_t { return WorldCallbacks::day(world); });
  callbacks.registerCallbackWithSignature<double>("timeOfDay", [world] -> double { return WorldCallbacks::timeOfDay(world); });
  callbacks.registerCallbackWithSignature<float>("dayLength", [world] -> float { return WorldCallbacks::dayLength(world); });
  callbacks.registerCallbackWithSignature<Json, String, Json>("getProperty", [world](auto&& PH1, auto&& PH2) -> auto { return WorldCallbacks::getProperty(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, Json>("setProperty", [world](auto&& PH1, auto&& PH2) -> auto { WorldCallbacks::setProperty(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<LiquidLevel>, Variant<RectF, Vec2I>>("liquidAt", [world](auto&& PH1) -> auto { return WorldCallbacks::liquidAt(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<float, Vec2F>("gravity", [world](auto&& PH1) -> auto { return WorldCallbacks::gravity(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F, LiquidId, float>("spawnLiquid", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldCallbacks::spawnLiquid(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<LiquidLevel>, Vec2F>("destroyLiquid", [world](auto&& PH1) -> auto { return WorldCallbacks::destroyLiquid(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F>("isTileProtected", [world](auto&& PH1) -> auto { return WorldCallbacks::isTileProtected(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<PlatformerAStar::Path>, Vec2F, Vec2F, ActorMovementParameters, PlatformerAStar::Parameters>("findPlatformerPath", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldCallbacks::findPlatformerPath(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<PlatformerAStar::PathFinder, Vec2F, Vec2F, ActorMovementParameters, PlatformerAStar::Parameters>("platformerPathStart", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldCallbacks::platformerPathStart(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });

  callbacks.registerCallback("type", [world](LuaEngine& engine) -> LuaString {
    if (auto serverWorld = as<WorldServer>(world)) {
      if (auto worldParameters = serverWorld->worldTemplate()->worldParameters())
        return engine.createString(worldParameters->typeName);
    } else if (auto clientWorld = as<WorldClient>(world)) {
      if (auto worldParameters = clientWorld->currentTemplate()->worldParameters())
        return engine.createString(worldParameters->typeName);
    }
    return engine.createString("unknown");
  });

  callbacks.registerCallback("size", [world]() -> Vec2I {
    if (auto serverWorld = as<WorldServer>(world))
      return (Vec2I)serverWorld->worldTemplate()->size();
    else if (auto clientWorld = as<WorldClient>(world))
      return (Vec2I)clientWorld->currentTemplate()->size();
    return {};
  });

  callbacks.registerCallback("inSurfaceLayer", [world](Vec2I const& position) -> bool {
    if (auto serverWorld = as<WorldServer>(world))
      return serverWorld->worldTemplate()->inSurfaceLayer(position);
    else if (auto clientWorld = as<WorldClient>(world))
      return clientWorld->currentTemplate()->inSurfaceLayer(position);
    return false;
  });

  callbacks.registerCallback("surfaceLevel", [world]() -> float {
    if (auto serverWorld = as<WorldServer>(world))
      return serverWorld->worldTemplate()->surfaceLevel();
    else if (auto clientWorld = as<WorldClient>(world))
      return clientWorld->currentTemplate()->surfaceLevel();
    else
      return world->geometry().size()[1] / 2.0f;
  });

  callbacks.registerCallback("terrestrial", [world]() -> bool {
    if (auto serverWorld = as<WorldServer>(world)) {
      if (auto worldParameters = serverWorld->worldTemplate()->worldParameters())
        return worldParameters->type() == WorldParametersType::TerrestrialWorldParameters;
    } else if (auto clientWorld = as<WorldClient>(world)) {
      if (auto worldParameters = clientWorld->currentTemplate()->worldParameters())
        return worldParameters->type() == WorldParametersType::TerrestrialWorldParameters;
    }
    return false;
  });

  callbacks.registerCallback("itemDropItem", [world](EntityId const& entityId) -> Json {
    if (auto itemDrop = world->get<ItemDrop>(entityId))
      return itemDrop->item()->descriptor().toJson();
    return {};
  });

  callbacks.registerCallback("biomeBlocksAt", [world](Vec2I position) -> std::optional<List<MaterialId>> {
    ConstPtr<WorldTemplate> worldTemplate;
    if (auto worldClient = as<WorldClient>(world))
      worldTemplate = worldClient->currentTemplate();
    else if (auto worldServer = as<WorldServer>(world))
      worldTemplate = worldServer->worldTemplate();

    if (worldTemplate) {
      WorldTemplate::BlockInfo block = worldTemplate->blockInfo(position[0], position[1]);
      if (ConstPtr<Biome> biome = worldTemplate->biome(block.blockBiomeIndex)) {
        List<MaterialId> blocks = {biome->mainBlock};
        blocks.appendAll(biome->subBlocks);
        return blocks;
      }
    }

    return {};
  });

  callbacks.registerCallback("dungeonId", [world](Vec2I position) -> DungeonId {
    if (auto serverWorld = as<WorldServer>(world)) {
      return serverWorld->dungeonId(position);
    } else {
      return as<WorldClient>(world)->dungeonId(position);
    }
  });

  if (auto clientWorld = as<WorldClient>(world)) {
    callbacks.registerCallback("inWorld", [clientWorld]() -> bool { return clientWorld->inWorld(); });
    callbacks.registerCallback("mainPlayer", [clientWorld]() -> EntityId { return clientWorld->clientState().playerId(); });
    callbacks.registerCallback("isClient", []() -> bool { return true; });
    callbacks.registerCallback("isServer", []() -> bool { return false; });
    callbacks.registerCallback("latency", [clientWorld]() -> int64_t { return clientWorld->latency(); });
    callbacks.registerCallbackWithSignature<void, EntityId>("resendEntity", [clientWorld](auto&& PH1) -> auto { ClientWorldCallbacks::resendEntity(clientWorld, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<RectI>("clientWindow", [clientWorld] -> RectI { return ClientWorldCallbacks::clientWindow(clientWorld); });
    callbacks.registerCallback("players", [clientWorld]() -> List<EntityId> {
      List<EntityId> playerIds;

      clientWorld->forAllEntities([&](Ptr<Entity> const& entity) -> void {
        if (entity->entityType() == EntityType::Player)
          playerIds.emplace_back(entity->entityId());
      });

      return playerIds;
    });
    callbacks.registerCallback("template", [clientWorld]() -> Json {
      return clientWorld->currentTemplate()->store();
    });
    callbacks.registerCallback("setTemplate", [clientWorld](Json worldTemplate) -> void {
      clientWorld->setTemplate(worldTemplate);
    });
    callbacks.registerCallback("wire", [clientWorld](Vec2I outputPosition, size_t outputIndex, Vec2I inputPosition, size_t inputIndex) -> void {
      clientWorld->wire(outputPosition, outputIndex, inputPosition, inputIndex);
    });
  }

  if (auto serverWorld = as<WorldServer>(world)) {
    callbacks.registerCallback("isClient", []() -> bool { return false; });
    callbacks.registerCallback("isServer", []() -> bool { return true; });

    callbacks.registerCallbackWithSignature<String>("id", [serverWorld] -> String { return ServerWorldCallbacks::id(serverWorld); });
    callbacks.registerCallbackWithSignature<bool, EntityId, bool>("breakObject", [serverWorld](auto&& PH1, auto&& PH2) -> auto { return ServerWorldCallbacks::breakObject(serverWorld, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    callbacks.registerCallbackWithSignature<bool, RectF>("isVisibleToPlayer", [serverWorld](auto&& PH1) -> auto { return ServerWorldCallbacks::isVisibleToPlayer(serverWorld, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<bool, RectF>("loadRegion", [serverWorld](auto&& PH1) -> auto { return ServerWorldCallbacks::loadRegion(serverWorld, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<bool, RectF>("regionActive", [serverWorld](auto&& PH1) -> auto { return ServerWorldCallbacks::regionActive(serverWorld, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<void, DungeonId, bool>("setTileProtection", [serverWorld](auto&& PH1, auto&& PH2) -> auto { ServerWorldCallbacks::setTileProtection(serverWorld, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    callbacks.registerCallbackWithSignature<bool, RectI>("isPlayerModified", [serverWorld](auto&& PH1) -> auto { return ServerWorldCallbacks::isPlayerModified(serverWorld, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<std::optional<LiquidLevel>, Vec2F>("forceDestroyLiquid", [serverWorld](auto&& PH1) -> auto { return ServerWorldCallbacks::forceDestroyLiquid(serverWorld, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<EntityId, String>("loadUniqueEntity", [serverWorld](auto&& PH1) -> auto { return ServerWorldCallbacks::loadUniqueEntity(serverWorld, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<void, EntityId, String>("setUniqueId", [serverWorld](auto&& PH1, auto&& PH2) -> auto { ServerWorldCallbacks::setUniqueId(serverWorld, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    callbacks.registerCallbackWithSignature<Json, EntityId, std::optional<EntityId>>("takeItemDrop", [world](auto&& PH1, auto&& PH2) -> auto { return ServerWorldCallbacks::takeItemDrop(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    callbacks.registerCallbackWithSignature<void, Vec2F, std::optional<bool>>("setPlayerStart", [world](auto&& PH1, auto&& PH2) -> auto { ServerWorldCallbacks::setPlayerStart(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    callbacks.registerCallbackWithSignature<List<EntityId>>("players", [world] -> List<EntityId> { return ServerWorldCallbacks::players(world); });
    callbacks.registerCallbackWithSignature<LuaString, LuaEngine&>("fidelity", [world](auto&& PH1) -> auto { return ServerWorldCallbacks::fidelity(world, std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<std::optional<LuaValue>, String, String, LuaVariadic<LuaValue>>("callScriptContext", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return ServerWorldCallbacks::callScriptContext(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
    callbacks.registerCallbackWithSignature<bool, ConnectionId, String, Json>("sendPacket", [serverWorld](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return ServerWorldCallbacks::sendPacket(serverWorld, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });

    callbacks.registerCallbackWithSignature<double>("skyTime", [serverWorld]() -> double {
      return serverWorld->sky()->epochTime();
    });
    callbacks.registerCallbackWithSignature<void, double>("setSkyTime", [serverWorld](double skyTime) -> void {
      return serverWorld->sky()->setEpochTime(skyTime);
    });

    callbacks.registerCallback("expiryTime", [serverWorld]() -> float { return serverWorld->expiryTime(); });
    callbacks.registerCallback("setExpiryTime", [serverWorld](float expiryTime) -> void { serverWorld->setExpiryTime(expiryTime); });

    callbacks.registerCallback("wire", [serverWorld](Vec2I outputPosition, size_t outputIndex, Vec2I inputPosition, size_t inputIndex) -> void {
      serverWorld->wire(outputPosition, outputIndex, inputPosition, inputIndex);
    });

    callbacks.registerCallback("flyingType", [serverWorld]() -> String { return FlyingTypeNames.getRight(serverWorld->sky()->flyingType()); });
    callbacks.registerCallback("warpPhase", [serverWorld]() -> String { return WarpPhaseNames.getRight(serverWorld->sky()->warpPhase()); });
    callbacks.registerCallback("setUniverseFlag", [serverWorld](String flagName) -> void { return serverWorld->universeSettings()->setFlag(flagName); });
    callbacks.registerCallback("universeFlags", [serverWorld]() -> StringSet { return serverWorld->universeSettings()->flags(); });
    callbacks.registerCallback("universeFlagSet", [serverWorld](String const& flagName) -> bool { return serverWorld->universeSettings()->flags().contains(flagName); });
    callbacks.registerCallback("placeDungeon", [serverWorld](String dungeonName, Vec2I position, std::optional<DungeonId> dungeonId) -> bool {
      return serverWorld->placeDungeon(dungeonName, position, dungeonId);
    });
    callbacks.registerCallback("tryPlaceDungeon", [serverWorld](String dungeonName, Vec2I position, std::optional<DungeonId> dungeonId) -> bool {
      return serverWorld->placeDungeon(dungeonName, position, dungeonId, false);
    });

    callbacks.registerCallback("addBiomeRegion", [serverWorld](Vec2I position, String biomeName, String subBlockSelector, int width) -> void {
      serverWorld->addBiomeRegion(position, biomeName, subBlockSelector, width);
    });
    callbacks.registerCallback("expandBiomeRegion", [serverWorld](Vec2I position, int width) -> void {
      serverWorld->expandBiomeRegion(position, width);
    });

    callbacks.registerCallback("pregenerateAddBiome", [serverWorld](Vec2I position, int width) -> bool {
      return serverWorld->pregenerateAddBiome(position, width);
    });
    callbacks.registerCallback("pregenerateExpandBiome", [serverWorld](Vec2I position, int width) -> bool {
      return serverWorld->pregenerateExpandBiome(position, width);
    });

    callbacks.registerCallback("setLayerEnvironmentBiome", [serverWorld](Vec2I position) -> void {
      serverWorld->setLayerEnvironmentBiome(position);
    });

    callbacks.registerCallback("setPlanetType", [serverWorld](String planetType, String primaryBiomeName) -> void {
      serverWorld->setPlanetType(planetType, primaryBiomeName);
    });

    callbacks.registerCallback("setDungeonGravity", [serverWorld](DungeonId dungeonId, std::optional<float> gravity) -> void {
      serverWorld->setDungeonGravity(dungeonId, gravity);
    });

    callbacks.registerCallback("setDungeonBreathable", [serverWorld](DungeonId dungeonId, std::optional<bool> breathable) -> void {
      serverWorld->setDungeonBreathable(dungeonId, breathable);
    });

    callbacks.registerCallback("setDungeonId", [serverWorld](RectI tileRegion, DungeonId dungeonId) -> void {
      serverWorld->setDungeonId(tileRegion, dungeonId);
    });

    callbacks.registerCallback("enqueuePlacement", [serverWorld](List<Json> distributionConfigs, std::optional<DungeonId> id) -> RpcPromise<Vec2I> {
      auto distributions = distributionConfigs.transformed([](Json const& config) -> BiomeItemDistribution {
        return {config, Random::randu64()};
      });
      return serverWorld->enqueuePlacement(std::move(distributions), id);
    });
    callbacks.registerCallback("template", [serverWorld]() -> Json {
      return serverWorld->worldTemplate()->store();
    });
    callbacks.registerCallback("setTemplate", [serverWorld](Json worldTemplate) -> void {
      auto newTemplate = std::make_shared<WorldTemplate>(worldTemplate);
      serverWorld->setTemplate(newTemplate);
    });
  }

  return callbacks;
}

void addWorldDebugCallbacks(LuaCallbacks& callbacks) {
  callbacks.registerCallback("debugPoint", WorldDebugCallbacks::debugPoint);
  callbacks.registerCallback("debugLine", WorldDebugCallbacks::debugLine);
  callbacks.registerCallback("debugPoly", WorldDebugCallbacks::debugPoly);
  callbacks.registerCallback("debugText", WorldDebugCallbacks::debugText);
}

void addWorldEntityCallbacks(LuaCallbacks& callbacks, World* world) {
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("entityQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::entityQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("monsterQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::monsterQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("npcQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::npcQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("objectQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::objectQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("itemDropQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::itemDropQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("playerQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::playerQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("loungeableQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::loungeableQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, Vec2F, std::optional<LuaTable>>("entityLineQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::entityLineQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, Vec2F, std::optional<LuaTable>>("objectLineQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::objectLineQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, Vec2F, std::optional<LuaTable>>("npcLineQuery", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::npcLineQuery(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallback("objectAt",
                             [world](Vec2I const& tilePosition) -> std::optional<int> {
                               if (auto object = world->findEntityAtTile(tilePosition, [](Ptr<TileEntity> const& entity) -> bool { return is<Object>(entity); }))
                                 return object->entityId();
                               else
                                 return {};
                             });

  callbacks.registerCallbackWithSignature<Ptr<Entity>, EntityId>("entity", [world](EntityId entityId) -> Ptr<Entity> {
    return world->entity(entityId);
  });

  callbacks.registerCallbackWithSignature<bool, int>("entityExists", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityExists(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, int, int>("entityCanDamage", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::entityCanDamage(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId>("entityDamageTeam", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityDamageTeam(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, EntityId>("entityAggressive", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityAggressive(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<LuaString>, LuaEngine&, int>("entityType", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::entityType(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<Vec2F>, int>("entityPosition", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityPosition(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<Vec2F>, int>("entityVelocity", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityVelocity(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<RectF>, int>("entityMetaBoundBox", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityMetaBoundBox(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<uint64_t>, EntityId, String>("entityCurrency", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::entityCurrency(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<uint64_t>, EntityId, Json, std::optional<bool>>("entityHasCountOfItem", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::entityHasCountOfItem(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<Vec2F>, EntityId>("entityHealth", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityHealth(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("entitySpecies", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entitySpecies(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("entityGender", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityGender(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("entityName", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityName(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<Json>, EntityId>("entityNametag", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityNametag(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId, std::optional<String>>("entityDescription", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::entityDescription(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<LuaNullTermWrapper<std::optional<List<Drawable>>>, EntityId, String>("entityPortrait", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::entityPortrait(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId, String>("entityHandItem", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::entityHandItem(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, String>("entityHandItemDescriptor", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::entityHandItemDescriptor(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<LuaNullTermWrapper<std::optional<String>>, EntityId>("entityUniqueId", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::entityUniqueId(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, String, std::optional<Json>>("getObjectParameter", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::getObjectParameter(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, String, std::optional<Json>>("getNpcScriptParameter", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::getNpcScriptParameter(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<List<Vec2I>, EntityId>("objectSpaces", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::objectSpaces(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<int>, EntityId>("farmableStage", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::farmableStage(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<int>, EntityId>("containerSize", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::containerSize(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, EntityId>("containerClose", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::containerClose(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, EntityId>("containerOpen", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::containerOpen(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, EntityId>("containerItems", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::containerItems(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, size_t>("containerItemAt", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerItemAt(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<bool>, EntityId, Json>("containerConsume", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerConsume(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<bool>, EntityId, size_t, int>("containerConsumeAt", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::containerConsumeAt(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<size_t>, EntityId, Json>("containerAvailable", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerAvailable(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId>("containerTakeAll", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::containerTakeAll(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, size_t>("containerTakeAt", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerTakeAt(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, size_t, int>("containerTakeNumItemsAt", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::containerTakeNumItemsAt(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<size_t>, EntityId, Json>("containerItemsCanFit", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerItemsCanFit(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, Json>("containerItemsFitWhere", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerItemsFitWhere(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, Json>("containerAddItems", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerAddItems(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, Json>("containerStackItems", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::containerStackItems(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, Json, size_t>("containerPutItemsAt", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::containerPutItemsAt(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, Json, size_t>("containerSwapItems", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::containerSwapItems(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, Json, size_t>("containerSwapItemsNoCombine", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::containerSwapItemsNoCombine(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<Json, EntityId, Json, size_t>("containerItemApply", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::containerItemApply(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<std::optional<LuaValue>, EntityId, String, LuaVariadic<LuaValue>>("callScriptedEntity", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEntityCallbacks::callScriptedEntity(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<RpcPromise<Vec2F>, String>("findUniqueEntity", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::findUniqueEntity(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<RpcPromise<Json>, LuaEngine&, LuaValue, String, LuaVariadic<Json>>("sendEntityMessage", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return WorldEntityCallbacks::sendEntityMessage(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<std::optional<List<EntityId>>, EntityId, std::optional<size_t>>("loungingEntities", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::loungingEntities(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<bool>, EntityId, std::optional<size_t>>("loungeableOccupied", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::loungeableOccupied(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<size_t>, EntityId>("loungeableAnchorCount", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::loungeableAnchorCount(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, EntityId, std::optional<bool>>("isMonster", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::isMonster(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("monsterType", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::monsterType(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("npcType", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::npcType(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("stagehandType", [world](auto&& PH1) -> auto { return WorldEntityCallbacks::stagehandType(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, EntityId, std::optional<int>>("isNpc", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEntityCallbacks::isNpc(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallback("isEntityInteractive", [world](EntityId entityId) -> std::optional<bool> {
    if (auto entity = world->get<InteractiveEntity>(entityId))
      return entity->isInteractive();
    return {};
  });
  callbacks.registerCallback("entityAimPosition", [world](EntityId entityId) -> std::optional<Vec2F> {
    if (auto entity = world->get<ToolUserEntity>(entityId))
      return entity->aimPosition();
    return {};
  });
  callbacks.registerCallback("entityMouthPosition", [world](EntityId entityId) -> std::optional<Vec2F> {
    if (auto entity = world->get<ChattyEntity>(entityId))
      return entity->mouthPosition();
    return {};
  });
  callbacks.registerCallback("entityTypeName", [world](EntityId entityId) -> std::optional<String> {
    auto entity = world->entity(entityId);
    if (auto monster = as<Monster>(entity))
      return monster->typeName();
    if (auto npc = as<Npc>(entity))
      return npc->npcType();
    if (auto vehicle = as<Vehicle>(entity))
      return vehicle->name();
    if (auto object = as<Object>(entity))
      return object->name();
    if (auto itemDrop = as<ItemDrop>(entity)) {
      if (itemDrop->item())
        return itemDrop->item()->name();
    }
    return {};
  });
}

void addWorldEnvironmentCallbacks(LuaCallbacks& callbacks, World* world) {
  callbacks.registerCallbackWithSignature<float, Vec2F>("lightLevel", [world](auto&& PH1) -> auto { return WorldEnvironmentCallbacks::lightLevel(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<float, Vec2F>("windLevel", [world](auto&& PH1) -> auto { return WorldEnvironmentCallbacks::windLevel(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F>("breathable", [world](auto&& PH1) -> auto { return WorldEnvironmentCallbacks::breathable(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F>("underground", [world](auto&& PH1) -> auto { return WorldEnvironmentCallbacks::underground(world, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, Vec2F, String>("material", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEnvironmentCallbacks::material(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, Vec2F, String>("mod", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return WorldEnvironmentCallbacks::mod(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<float, Vec2F, String>("materialHueShift", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEnvironmentCallbacks::materialHueShift(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<float, Vec2F, String>("modHueShift", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEnvironmentCallbacks::modHueShift(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<MaterialColorVariant, Vec2F, String>("materialColor", [world](auto&& PH1, auto&& PH2) -> auto { return WorldEnvironmentCallbacks::materialColor(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, Vec2F, String, MaterialColorVariant>("setMaterialColor", [world](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { WorldEnvironmentCallbacks::setMaterialColor(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });

  callbacks.registerCallback("oceanLevel", [world](Vec2I position) -> int {
    if (auto serverWorld = as<WorldServer>(world)) {
      return serverWorld->worldTemplate()->blockInfo(position[0], position[1]).oceanLiquidLevel;
    } else {
      auto clientWorld = as<WorldClient>(world);
      return clientWorld->currentTemplate()->blockInfo(position[0], position[1]).oceanLiquidLevel;
    }
  });

  callbacks.registerCallback("environmentStatusEffects", [world](Vec2F const& position) -> StringList {
    return world->environmentStatusEffects(position);
  });

  callbacks.registerCallbackWithSignature<bool, List<Vec2I>, String, Vec2F, String, float, std::optional<unsigned>, std::optional<EntityId>>("damageTiles", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6, auto&& PH7) -> auto { return WorldEnvironmentCallbacks::damageTiles(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6), std::forward<decltype(PH7)>(PH7)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F, float, String, Vec2F, String, float, std::optional<unsigned>, std::optional<EntityId>>("damageTileArea", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6, auto&& PH7, auto&& PH8) -> auto { return WorldEnvironmentCallbacks::damageTileArea(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6), std::forward<decltype(PH7)>(PH7), std::forward<decltype(PH8)>(PH8)); });
  callbacks.registerCallbackWithSignature<bool, Vec2I, String, String, std::optional<int>, bool>("placeMaterial", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5) -> auto { return WorldEnvironmentCallbacks::placeMaterial(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); });
  callbacks.registerCallbackWithSignature<bool, List<Vec2I>, String, String, std::optional<int>, bool>("replaceMaterials", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5) -> auto { return WorldEnvironmentCallbacks::replaceMaterials(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); });
  callbacks.registerCallbackWithSignature<bool, Vec2F, float, String, String, std::optional<int>, bool>("replaceMaterialArea", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6) -> auto { return WorldEnvironmentCallbacks::replaceMaterialArea(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6)); });
  callbacks.registerCallbackWithSignature<bool, Vec2I, String, String, std::optional<int>, bool>("placeMod", [world](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5) -> auto { return WorldEnvironmentCallbacks::placeMod(world, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); });

  callbacks.registerCallback("radialTileQuery", [world](Vec2F center, float radius, String layerName) -> List<Vec2I> {
    auto layer = TileLayerNames.getLeft(layerName);
    return tileAreaBrush(radius, center, false).filtered([&](Vec2I const& t) -> bool {
      return world->material(t, layer) != EmptyMaterialId;
    });
  });
}

auto WorldCallbacks::magnitude(World* world, Vec2F pos1, std::optional<Vec2F> pos2) -> float {
  if (pos2)
    return world->geometry().diff(pos1, *pos2).magnitude();
  else
    return pos1.magnitude();
}

auto WorldCallbacks::distance(World* world, Vec2F const& arg1, Vec2F const& arg2) -> Vec2F {
  return world->geometry().diff(arg1, arg2);
}

auto WorldCallbacks::polyContains(World* world, PolyF const& poly, Vec2F const& pos) -> bool {
  return world->geometry().polyContains(poly, pos);
}

auto WorldCallbacks::xwrap(World* world, LuaEngine& engine, LuaValue const& positionOrX) -> LuaValue {
  if (auto x = engine.luaMaybeTo<float>(positionOrX))
    return LuaFloat(world->geometry().xwrap(*x));
  return engine.luaFrom<Vec2F>(world->geometry().xwrap(engine.luaTo<Vec2F>(positionOrX)));
}

auto WorldCallbacks::nearestTo(World* world, LuaEngine& engine, Variant<Vec2F, float> const& sourcePositionOrX, Variant<Vec2F, float> const& targetPositionOrX) -> LuaValue {
  if (targetPositionOrX.is<Vec2F>()) {
    Vec2F targetPosition = targetPositionOrX.get<Vec2F>();
    Vec2F sourcePosition;
    if (sourcePositionOrX.is<Vec2F>())
      sourcePosition = sourcePositionOrX.get<Vec2F>();
    else
      sourcePosition[0] = sourcePositionOrX.get<float>();

    return engine.luaFrom<Vec2F>(world->geometry().nearestTo(sourcePosition, targetPosition));

  } else {
    float targetX = targetPositionOrX.get<float>();
    float sourceX;
    if (sourcePositionOrX.is<Vec2F>())
      sourceX = sourcePositionOrX.get<Vec2F>()[0];
    else
      sourceX = sourcePositionOrX.get<float>();

    return LuaFloat(world->geometry().nearestTo(sourceX, targetX));
  }
}

auto WorldCallbacks::rectCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2) -> bool {
  PolyF body = PolyF(arg1);

  if (arg2)
    return world->polyCollision(body, *arg2);
  else
    return world->polyCollision(body);
}

auto WorldCallbacks::pointTileCollision(World* world, Vec2F const& arg1, std::optional<CollisionSet> const& arg2) -> bool {
  if (arg2)
    return world->pointTileCollision(arg1, *arg2);
  else
    return world->pointTileCollision(arg1);
}

auto WorldCallbacks::lineTileCollision(
  World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3) -> bool {
  Vec2F const begin = arg1;
  Vec2F const end = arg2;

  if (arg3)
    return world->lineTileCollision(begin, end, *arg3);
  else
    return world->lineTileCollision(begin, end);
}

auto WorldCallbacks::lineTileCollisionPoint(World* world, Vec2F const& begin, Vec2F const& end, std::optional<CollisionSet> const& collisionSet) -> std::optional<std::pair<Vec2F, Vec2I>> {
  if (collisionSet)
    return world->lineTileCollisionPoint(begin, end, *collisionSet);
  else
    return world->lineTileCollisionPoint(begin, end);
}

auto WorldCallbacks::rectTileCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2) -> bool {
  RectI const region = RectI::integral(arg1);

  if (arg2)
    return world->rectTileCollision(region, *arg2);
  else
    return world->rectTileCollision(region);
}

auto WorldCallbacks::pointCollision(World* world, Vec2F const& point, std::optional<CollisionSet> const& collisionSet) -> bool {
  return world->pointCollision(point, collisionSet.value_or(DefaultCollisionSet));
}

auto WorldCallbacks::lineCollision(World* world, Vec2F const& start, Vec2F const& end, std::optional<CollisionSet> const& collisionSet) -> LuaTupleReturn<std::optional<Vec2F>, std::optional<Vec2F>> {
  std::optional<Vec2F> point;
  std::optional<Vec2F> normal;
  auto collision = world->lineCollision(Line2F(start, end), collisionSet.value_or(DefaultCollisionSet));
  if (collision) {
    point = collision->first;
    normal = collision->second;
  }
  return luaTupleReturn(point, normal);
}

auto WorldCallbacks::polyCollision(
  World* world, PolyF const& arg1, std::optional<Vec2F> const& arg2, std::optional<CollisionSet> const& arg3) -> bool {
  PolyF body = arg1;

  Vec2F center;
  if (arg2) {
    center = *arg2;
    body.translate(center);
  }

  if (arg3)
    return world->polyCollision(body, *arg3);
  else
    return world->polyCollision(body);
}

auto WorldCallbacks::collisionBlocksAlongLine(
  World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3, std::optional<int> const& arg4) -> List<Vec2I> {
  Vec2F const begin = arg1;
  Vec2F const end = arg2;

  CollisionSet collisionSet = arg3.value_or(DefaultCollisionSet);
  int const maxSize = arg4 ? *arg4 : -1;
  return world->collidingTilesAlongLine(begin, end, collisionSet, maxSize);
}

auto WorldCallbacks::liquidAlongLine(World* world, Vec2F const& start, Vec2F const& end) -> List<std::pair<Vec2I, LiquidLevel>> {
  List<std::pair<Vec2I, LiquidLevel>> levels;
  forBlocksAlongLine<float>(start, world->geometry().diff(end, start), [&](int x, int y) -> bool {
    auto liquidLevel = world->liquidLevel(RectF::withSize(Vec2F(x, y), Vec2F(1, 1)));
    if (liquidLevel.liquid != EmptyLiquidId)
      levels.append(std::pair<Vec2I, LiquidLevel>(Vec2I(x, y), liquidLevel));
    return true;
  });
  return levels;
}

auto WorldCallbacks::resolvePolyCollision(
  World* world, PolyF poly, Vec2F const& position, float maximumCorrection, std::optional<CollisionSet> const& MaybeCollisionSet) -> std::optional<Vec2F> {
  struct CollisionPoly {
    PolyF poly;
    Vec2F center;
    float sortingDistance;
  };

  poly.translate(position);
  List<CollisionPoly> collisions;
  CollisionSet collisionSet = MaybeCollisionSet.value_or(DefaultCollisionSet);
  world->forEachCollisionBlock(RectI::integral(poly.boundBox().padded(maximumCorrection + 1.0f)),
                               [&](auto const& block) -> auto {
                                 if (collisionSet.contains(block.kind))
                                   collisions.append({block.poly, Vec2F(block.space), 0.0f});
                               });

  auto resolveCollision = [&](std::optional<Vec2F> direction, float maximumDistance, int loops) -> std::optional<Vec2F> {
    PolyF body = poly;
    Vec2F correction;
    for (int i = 0; i < loops; ++i) {
      Vec2F bodyCenter = body.center();
      for (auto& cp : collisions)
        cp.sortingDistance = vmagSquared(bodyCenter - cp.center);
      sort(collisions, [](auto const& a, auto const& b) -> auto { return a.sortingDistance < b.sortingDistance; });

      bool anyIntersects = false;
      for (auto const& cp : collisions) {
        PolyF::IntersectResult intersection;
        if (direction)
          intersection = body.directionalSatIntersection(cp.poly, *direction, false);
        else
          intersection = body.satIntersection(cp.poly);

        if (intersection.intersects) {
          anyIntersects = true;
          body.translate(intersection.overlap);
          correction += intersection.overlap;
          if (vmag(correction) > maximumDistance)
            return {};
        }
      }

      if (!anyIntersects)
        return correction;
    }

    for (auto const& cp : collisions) {
      if (body.intersects(cp.poly))
        return {};
    }

    return correction;
  };

  // First try any-directional SAT separation for two loops
  if (auto resolution = resolveCollision({}, maximumCorrection, 2))
    return position + *resolution;

  // Then, try direction-limiting SAT in cardinals, then 45 degs, then in
  // between, for 16 total angles in a circle.
  for (int i : {4, 8, 12, 0, 2, 6, 10, 14, 1, 3, 7, 5, 15, 13, 9, 11}) {
    float angle = i * Constants::pi / 8;
    Vec2F dir = Vec2F::withAngle(angle, 1.0f);
    if (auto resolution = resolveCollision(dir, maximumCorrection, 1))
      return position + *resolution;
  }

  return {};
}

auto WorldCallbacks::tileIsOccupied(
  World* world, Vec2I const& arg1, std::optional<bool> const& arg2, std::optional<bool> const& arg3) -> bool {
  Vec2I const tile = arg1;
  bool const tileLayerBool = arg2.value_or(true);
  bool const includeEphemeral = arg3.value_or(false);

  TileLayer const tileLayer = tileLayerBool ? TileLayer::Foreground : TileLayer::Background;

  return world->tileIsOccupied(tile, tileLayer, includeEphemeral);
}

auto WorldCallbacks::placeObject(World* world,
                                 String const& objectType,
                                 Vec2I const& worldPosition,
                                 std::optional<int> const& objectDirection,
                                 Json const& objectParameters) -> bool {
  auto objectDatabase = Root::singleton().objectDatabase();

  try {
    Direction direction = Direction::Right;
    if (objectDirection && *objectDirection < 0)
      direction = Direction::Left;

    Json parameters = objectParameters ? objectParameters : JsonObject();

    auto placedObject = objectDatabase->createForPlacement(world, objectType, worldPosition, direction, parameters);
    if (placedObject) {
      world->addEntity(placedObject);
      return true;
    }
  } catch (StarException const& exception) {
    Logger::warn("Could not create placable object of kind '{}', exception caught: {}",
                 objectType,
                 outputException(exception, false));
  }

  return false;
}

auto WorldCallbacks::spawnItem(World* world,
                               Json const& itemType,
                               Vec2F const& worldPosition,
                               std::optional<size_t> const& inputCount,
                               Json const& inputParameters,
                               std::optional<Vec2F> const& initialVelocity,
                               std::optional<float> const& intangibleTime) -> std::optional<EntityId> {
  Vec2F const position = worldPosition;

  try {
    ItemDescriptor descriptor;
    if (itemType.isType(Json::Type::String)) {
      size_t count = inputCount.value_or(1);

      Json parameters = inputParameters ? inputParameters : JsonObject();

      descriptor = ItemDescriptor(itemType.toString(), count, parameters);
    } else {
      descriptor = ItemDescriptor(itemType);
    }

    if (auto itemDrop = ItemDrop::createRandomizedDrop(descriptor, position)) {
      if (initialVelocity)
        itemDrop->setVelocity(*initialVelocity);
      if (intangibleTime)
        itemDrop->setIntangibleTime(*intangibleTime);
      world->addEntity(itemDrop);
      return itemDrop->inWorld() ? itemDrop->entityId() : std::optional<EntityId>();
    }

    Logger::warn("Could not spawn item, item empty in WorldCallbacks::spawnItem");
  } catch (StarException const& exception) {
    Logger::warn("Could not spawn Item of kind '{}', exception caught: {}", itemType, outputException(exception, false));
  }

  return {};
}

auto WorldCallbacks::spawnTreasure(
  World* world, Vec2F const& position, String const& pool, float level, std::optional<uint64_t> seed) -> List<EntityId> {
  List<EntityId> entities;
  auto treasureDatabase = Root::singleton().treasureDatabase();
  try {
    for (auto const& treasureItem : treasureDatabase->createTreasure(pool, level, seed.value_or(Random::randu64()))) {
      Ptr<ItemDrop> entity = ItemDrop::createRandomizedDrop(treasureItem, position);
      entities.append(entity->entityId());
      world->addEntity(entity);
    }
  } catch (StarException const& exception) {
    Logger::warn(
      "Could not spawn treasure from pool '{}', exception caught: {}", pool, outputException(exception, false));
  }
  return entities;
}

auto WorldCallbacks::spawnMonster(
  World* world, String const& arg1, Vec2F const& arg2, std::optional<JsonObject> const& arg3) -> std::optional<EntityId> {
  Vec2F const spawnPosition = arg2;
  auto monsterDatabase = Root::singleton().monsterDatabase();

  try {
    JsonObject parameters;
    parameters["aggressive"] = Random::randb();
    if (arg3)
      parameters.merge(*arg3, true);

    float level = 1;
    if (parameters.contains("level"))
      level = parameters.get("level").toFloat();
    auto monster = monsterDatabase->createMonster(monsterDatabase->randomMonster(arg1, parameters), level);

    monster->setPosition(spawnPosition);
    world->addEntity(monster);
    return monster->inWorld() ? monster->entityId() : std::optional<EntityId>();
  } catch (StarException const& exception) {
    Logger::warn(
      "Could not spawn Monster of type '{}', exception caught: {}", arg1, outputException(exception, false));
    return {};
  }
}

auto WorldCallbacks::spawnNpc(World* world,
                              Vec2F const& arg1,
                              String const& arg2,
                              String const& arg3,
                              float arg4,
                              std::optional<uint64_t> arg5,
                              Json const& arg6) -> std::optional<EntityId> {
  Vec2F const spawnPosition = arg1;

  String typeName = arg3;
  float level = arg4;

  uint64_t seed;
  if (arg5)
    seed = *arg5;
  else
    seed = Random::randu64();

  Json overrides = arg6 ? arg6 : JsonObject();

  auto npcDatabase = Root::singleton().npcDatabase();
  try {
    auto npc = npcDatabase->createNpc(npcDatabase->generateNpcVariant(arg2, typeName, level, seed, overrides));
    npc->setPosition(spawnPosition);
    world->addEntity(npc);
    return npc->inWorld() ? npc->entityId() : std::optional<EntityId>();
  } catch (StarException const& exception) {
    Logger::warn("Could not spawn NPC of species '{}' and type '{}', exception caught: {}",
                 arg2,
                 typeName,
                 outputException(exception, false));
    return {};
  }
}

auto WorldCallbacks::spawnStagehand(
  World* world, Vec2F const& spawnPosition, String const& typeName, Json const& overrides) -> std::optional<EntityId> {
  auto stagehandDatabase = Root::singleton().stagehandDatabase();
  try {
    auto stagehand = stagehandDatabase->createStagehand(typeName, overrides);
    stagehand->setPosition(spawnPosition);
    world->addEntity(stagehand);
    return stagehand->inWorld() ? stagehand->entityId() : std::optional<EntityId>();
  } catch (StarException const& exception) {
    Logger::warn(
      "Could not spawn Stagehand of type '{}', exception caught: {}", typeName, outputException(exception, false));
    return {};
  }
}

auto WorldCallbacks::spawnProjectile(World* world,
                                     String const& projectileType,
                                     Vec2F const& spawnPosition,
                                     std::optional<EntityId> const& sourceEntityId,
                                     std::optional<Vec2F> const& projectileDirection,
                                     bool trackSourceEntity,
                                     Json const& projectileParameters) -> std::optional<EntityId> {

  try {
    ConstPtr<ProjectileDatabase> projectileDatabase = Root::singleton().projectileDatabase();
    auto projectile = projectileDatabase->createProjectile(projectileType, projectileParameters ? projectileParameters : JsonObject());
    projectile->setInitialPosition(spawnPosition);
    projectile->setInitialDirection(projectileDirection.value());
    projectile->setSourceEntity(sourceEntityId.value_or(NullEntityId), trackSourceEntity);
    world->addEntity(projectile);
    return projectile->inWorld() ? projectile->entityId() : std::optional<EntityId>();
  } catch (StarException const& exception) {
    Logger::warn(
      "Could not spawn Projectile of type '{}', exception caught: {}", projectileType, outputException(exception, false));
    return {};
  }
}

auto WorldCallbacks::spawnVehicle(
  World* world, String const& vehicleName, Vec2F const& pos, Json const& extraConfig) -> std::optional<EntityId> {
  ConstPtr<VehicleDatabase> vehicleDatabase = Root::singleton().vehicleDatabase();
  auto vehicle = vehicleDatabase->create(vehicleName, extraConfig);
  vehicle->setPosition(pos);
  world->addEntity(vehicle);
  if (vehicle->inWorld())
    return vehicle->entityId();
  return {};
}

auto WorldCallbacks::time(World* world) -> double {
  return world->epochTime();
}

auto WorldCallbacks::day(World* world) -> uint64_t {
  return world->day();
}

auto WorldCallbacks::timeOfDay(World* world) -> double {
  return world->timeOfDay() / world->dayLength();
}

auto WorldCallbacks::dayLength(World* world) -> float {
  return world->dayLength();
}

auto WorldCallbacks::getProperty(World* world, String const& arg1, Json const& arg2) -> Json {
  return world->getProperty(arg1, arg2);
}

void WorldCallbacks::setProperty(World* world, String const& arg1, Json const& arg2) {
  world->setProperty(arg1, arg2);
}

auto WorldCallbacks::liquidAt(World* world, Variant<RectF, Vec2I> boundBoxOrPoint) -> std::optional<LiquidLevel> {
  LiquidLevel liquidLevel = boundBoxOrPoint.call([world](auto const& bbop) -> auto { return world->liquidLevel(bbop); });
  if (liquidLevel.liquid != EmptyLiquidId)
    return liquidLevel;
  return {};
}

auto WorldCallbacks::gravity(World* world, Vec2F const& arg1) -> float {
  return world->gravity(arg1);
}

auto WorldCallbacks::spawnLiquid(World* world, Vec2F const& position, LiquidId liquid, float quantity) -> bool {
  return world->modifyTile(Vec2I::floor(position), PlaceLiquid{.liquid = liquid, .liquidLevel = quantity}, true);
}

auto WorldCallbacks::destroyLiquid(World* world, Vec2F const& position) -> std::optional<LiquidLevel> {
  auto liquidLevel = world->liquidLevel(Vec2I::floor(position));
  if (liquidLevel.liquid != EmptyLiquidId) {
    if (world->modifyTile(Vec2I::floor(position), PlaceLiquid{.liquid = EmptyLiquidId, .liquidLevel = 0}, true))
      return liquidLevel;
  }
  return {};
}

auto WorldCallbacks::isTileProtected(World* world, Vec2F const& position) -> bool {
  return world->isTileProtected(Vec2I::floor(position));
}

auto WorldCallbacks::findPlatformerPath(World* world,
                                        Vec2F const& start,
                                        Vec2F const& end,
                                        ActorMovementParameters actorMovementParameters,
                                        PlatformerAStar::Parameters searchParameters) -> std::optional<PlatformerAStar::Path> {
  PlatformerAStar::PathFinder pathFinder(world, start, end, std::move(actorMovementParameters), std::move(searchParameters));
  pathFinder.explore({});
  return pathFinder.result();
}

auto WorldCallbacks::platformerPathStart(World* world,
                                         Vec2F const& start,
                                         Vec2F const& end,
                                         ActorMovementParameters actorMovementParameters,
                                         PlatformerAStar::Parameters searchParameters) -> PlatformerAStar::PathFinder {
  return {world, start, end, std::move(actorMovementParameters), std::move(searchParameters)};
}

void ClientWorldCallbacks::resendEntity(WorldClient* world, EntityId arg1) {
  return world->resendEntity(arg1);
}

auto ClientWorldCallbacks::clientWindow(WorldClient* world) -> RectI {
  return world->clientWindow();
}

auto ServerWorldCallbacks::id(WorldServer* world) -> String {
  return world->worldId();
}

auto ServerWorldCallbacks::breakObject(WorldServer* world, EntityId arg1, bool arg2) -> bool {
  if (auto entity = world->get<Object>(arg1)) {
    bool smash = arg2;
    entity->breakObject(smash);
    return true;
  }
  return false;
}

auto ServerWorldCallbacks::isVisibleToPlayer(WorldServer* world, RectF const& arg1) -> bool {
  return world->isVisibleToPlayer(arg1);
}

auto ServerWorldCallbacks::loadRegion(WorldServer* world, RectF const& arg1) -> bool {
  return world->signalRegion(RectI::integral(arg1));
}

auto ServerWorldCallbacks::regionActive(WorldServer* world, RectF const& arg1) -> bool {
  return world->regionActive(RectI::integral(arg1));
}

void ServerWorldCallbacks::setTileProtection(WorldServer* world, DungeonId arg1, bool arg2) {
  DungeonId dungeonId = arg1;
  bool isProtected = arg2;
  world->setTileProtection(dungeonId, isProtected);
}

auto ServerWorldCallbacks::isPlayerModified(WorldServer* world, RectI const& region) -> bool {
  return world->isPlayerModified(region);
}

auto ServerWorldCallbacks::forceDestroyLiquid(WorldServer* world, Vec2F const& position) -> std::optional<LiquidLevel> {
  auto liquidLevel = world->liquidLevel(Vec2I::floor(position));
  if (liquidLevel.liquid != EmptyLiquidId) {
    if (world->forceModifyTile(Vec2I::floor(position), PlaceLiquid{.liquid = EmptyLiquidId, .liquidLevel = 0}, true))
      return liquidLevel;
  }
  return {};
}

auto ServerWorldCallbacks::loadUniqueEntity(WorldServer* world, String const& uniqueId) -> EntityId {
  return world->loadUniqueEntity(uniqueId);
}

void ServerWorldCallbacks::setUniqueId(WorldServer* world, EntityId entityId, std::optional<String> const& uniqueId) {
  auto entity = world->entity(entityId);
  if (auto npc = as<Npc>(entity.get()))
    npc->setUniqueId(uniqueId);
  else if (auto monster = as<Monster>(entity.get()))
    monster->setUniqueId(uniqueId);
  else if (auto object = as<Object>(entity.get()))
    object->setUniqueId(uniqueId);
  else if (auto stagehand = as<Stagehand>(entity.get()))
    stagehand->setUniqueId(uniqueId);
  else if (entity)
    throw StarException::format("Cannot set unique id on entity of type {}", EntityTypeNames.getRight(entity->entityType()));
  else
    throw StarException::format("No such entity with id {}", entityId);
}

auto ServerWorldCallbacks::takeItemDrop(World* world, EntityId entityId, std::optional<EntityId> const& takenBy) -> Json {
  auto itemDrop = world->get<ItemDrop>(entityId);
  if (itemDrop && itemDrop->canTake() && itemDrop->isMaster()) {
    Ptr<Item> item;
    if (takenBy)
      item = itemDrop->takeBy(*takenBy);
    else
      item = itemDrop->take();

    if (item)
      return item->descriptor().toJson();
  }

  return {};
}

void ServerWorldCallbacks::setPlayerStart(World* world, Vec2F const& playerStart, std::optional<bool> respawnInWorld) {
  as<WorldServer>(world)->setPlayerStart(playerStart, respawnInWorld.has_value() && respawnInWorld.value());
}

auto ServerWorldCallbacks::players(World* world) -> List<EntityId> {
  return as<WorldServer>(world)->players();
}

auto ServerWorldCallbacks::fidelity(World* world, LuaEngine& engine) -> LuaString {
  return engine.createString(WorldServerFidelityNames.getRight(as<WorldServer>(world)->fidelity()));
}

auto ServerWorldCallbacks::callScriptContext(World* world, String const& contextName, String const& function, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> {
  auto context = as<WorldServer>(world)->scriptContext(contextName);
  if (!context)
    throw StarException::format("Context {} does not exist", contextName);
  return context->invoke(function, args);
}

auto ServerWorldCallbacks::sendPacket(WorldServer* world, ConnectionId clientId, String const& packetType, Json const& packetData) -> bool {
  PacketType type = PacketTypeNames.getLeft(packetType);
  auto packet = createPacket(type, packetData);
  return world->sendPacket(clientId, packet);
}

void WorldDebugCallbacks::debugPoint(Vec2F const& arg1, Color const& arg2) {
  SpatialLogger::logPoint("world", arg1, arg2.toRgba());
}

void WorldDebugCallbacks::debugLine(Vec2F const& arg1, Vec2F const& arg2, Color const& arg3) {
  SpatialLogger::logLine("world", arg1, arg2, arg3.toRgba());
}

void WorldDebugCallbacks::debugPoly(PolyF const& poly, Color const& color) {
  SpatialLogger::logPoly("world", poly, color.toRgba());
}

void WorldDebugCallbacks::debugText(LuaEngine& engine, LuaVariadic<LuaValue> const& args) {
  if (args.size() < 3)
    throw StarException(strf("Too few arguments to debugText: {}", args.size()));

  const auto position = engine.luaTo<Vec2F>(args.at(args.size() - 2));
  const Vec4B color = engine.luaTo<Color>(args.at(args.size() - 1)).toRgba();

  String text = formatLua(engine.luaTo<String>(args.at(0)), slice<List<LuaValue>>(args, 1, args.size() - 2));
  SpatialLogger::logText("world", text, position, color);
}

auto WorldEntityCallbacks::entityQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityQuery<Entity>(world, engine, pos1, pos2, std::move(options));
}

auto WorldEntityCallbacks::monsterQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityQuery<Monster>(world, engine, pos1, pos2, std::move(options));
}

auto WorldEntityCallbacks::npcQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityQuery<Npc>(world, engine, pos1, pos2, std::move(options));
}

auto WorldEntityCallbacks::objectQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable {
  String objectName;
  if (options)
    objectName = options->get<std::optional<String>>("name").value();

  return LuaBindings::entityQuery<Object>(world,
                                          engine,
                                          pos1,
                                          pos2,
                                          std::move(options),
                                          [&objectName](std::shared_ptr<Object> const& entity) -> bool {
                                            return objectName.empty() || entity->name() == objectName;
                                          });
}

auto WorldEntityCallbacks::itemDropQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityQuery<ItemDrop>(world, engine, pos1, pos2, std::move(options));
}

auto WorldEntityCallbacks::playerQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityQuery<Player>(world, engine, pos1, pos2, std::move(options));
}

auto WorldEntityCallbacks::loungeableQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable {
  String orientationName;
  if (options)
    orientationName = options->get<std::optional<String>>("orientation").value();

  LoungeOrientation orientation = LoungeOrientation::None;
  if (orientationName == "sit")
    orientation = LoungeOrientation::Sit;
  else if (orientationName == "lay")
    orientation = LoungeOrientation::Lay;
  else if (orientationName == "stand")
    orientation = LoungeOrientation::Stand;
  else if (orientationName.empty())
    orientation = LoungeOrientation::None;
  else
    throw StarException(strf("Unsupported loungeableQuery orientation {}", orientationName));

  auto filter = [orientation](std::shared_ptr<LoungeableObject> const& entity) -> bool {
    auto loungeable = as<LoungeableEntity>(entity);
    if (!loungeable || loungeable->anchorCount() == 0)
      return false;

    if (orientation == LoungeOrientation::None)
      return true;
    auto pos = loungeable->loungeAnchor(0);
    return pos && pos->orientation == orientation;
  };

  return LuaBindings::entityQuery<LoungeableObject>(world, engine, pos1, pos2, std::move(options), filter);
}

auto WorldEntityCallbacks::entityLineQuery(World* world, LuaEngine& engine, Vec2F const& point1, Vec2F const& point2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityLineQuery<Entity>(world, engine, point1, point2, std::move(options));
}

auto WorldEntityCallbacks::objectLineQuery(World* world, LuaEngine& engine, Vec2F const& point1, Vec2F const& point2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityLineQuery<Object>(world, engine, point1, point2, std::move(options));
}

auto WorldEntityCallbacks::npcLineQuery(World* world, LuaEngine& engine, Vec2F const& point1, Vec2F const& point2, std::optional<LuaTable> options) -> LuaTable {
  return LuaBindings::entityLineQuery<Npc>(world, engine, point1, point2, std::move(options));
}

auto WorldEntityCallbacks::entityExists(World* world, EntityId entityId) -> bool {
  return (bool)world->entity(entityId);
}

auto WorldEntityCallbacks::entityCanDamage(World* world, EntityId sourceId, EntityId targetId) -> bool {
  auto source = world->entity(sourceId);
  auto target = world->entity(targetId);

  if (!source || !target || !source->getTeam().canDamage(target->getTeam(), false))
    return false;

  return true;
}

auto WorldEntityCallbacks::entityDamageTeam(World* world, EntityId entityId) -> Json {
  if (auto entity = world->entity(entityId))
    return entity->getTeam().toJson();
  return {};
}

auto WorldEntityCallbacks::entityAggressive(World* world, EntityId entityId) -> bool {
  auto entity = world->entity(entityId);
  if (auto monster = as<Monster>(entity))
    return monster->aggressive();
  if (auto npc = as<Npc>(entity))
    return npc->aggressive();
  return false;
}

auto WorldEntityCallbacks::entityType(World* world, LuaEngine& engine, EntityId entityId) -> std::optional<LuaString> {
  if (auto entity = world->entity(entityId))
    return engine.createString(EntityTypeNames.getRight(entity->entityType()));
  return std::nullopt;
}

auto WorldEntityCallbacks::entityPosition(World* world, EntityId entityId) -> std::optional<Vec2F> {
  if (auto entity = world->entity(entityId)) {
    return entity->position();
  } else {
    return std::nullopt;
  }
}

auto WorldEntityCallbacks::entityMetaBoundBox(World* world, EntityId entityId) -> std::optional<RectF> {
  if (auto entity = world->entity(entityId)) {
    return entity->metaBoundBox();
  } else {
    return std::nullopt;
  }
}

auto WorldEntityCallbacks::entityVelocity(World* world, EntityId entityId) -> std::optional<Vec2F> {
  auto entity = world->entity(entityId);

  if (auto monsterEntity = as<Monster>(entity))
    return monsterEntity->velocity();
  else if (auto npcEntity = as<Npc>(entity))
    return npcEntity->velocity();
  else if (auto playerEntity = as<Player>(entity))
    return playerEntity->velocity();
  else if (auto vehicleEntity = as<Vehicle>(entity))
    return vehicleEntity->velocity();
  else if (auto projectileEntity = as<Projectile>(entity))
    return projectileEntity->velocity();

  return std::nullopt;
}

auto WorldEntityCallbacks::entityCurrency(World* world, EntityId entityId, String const& currencyType) -> std::optional<uint64_t> {
  if (auto player = world->get<Player>(entityId)) {
    return player->currency(currencyType);
  } else {
    return std::nullopt;
  }
}

auto WorldEntityCallbacks::entityHasCountOfItem(World* world, EntityId entityId, Json descriptor, std::optional<bool> exactMatch) -> std::optional<uint64_t> {
  if (auto player = world->get<Player>(entityId)) {
    return player->inventory()->hasCountOfItem(ItemDescriptor(descriptor), exactMatch.value_or(false));
  } else {
    return std::nullopt;
  }
}

auto WorldEntityCallbacks::entityHealth(World* world, EntityId entityId) -> std::optional<Vec2F> {
  if (auto entity = world->get<DamageBarEntity>(entityId)) {
    return Vec2F(entity->health(), entity->maxHealth());
  } else {
    return std::nullopt;
  }
}

auto WorldEntityCallbacks::entitySpecies(World* world, EntityId entityId) -> std::optional<String> {
  if (auto player = world->get<Player>(entityId)) {
    return player->species();
  } else if (auto npc = world->get<Npc>(entityId)) {
    return npc->species();
  } else {
    return std::nullopt;
  }
}

auto WorldEntityCallbacks::entityGender(World* world, EntityId entityId) -> std::optional<String> {
  if (auto player = world->get<Player>(entityId)) {
    return GenderNames.getRight(player->gender());
  } else if (auto npc = world->get<Npc>(entityId)) {
    return GenderNames.getRight(npc->gender());
  } else {
    return std::nullopt;
  }
}

auto WorldEntityCallbacks::entityName(World* world, EntityId entityId) -> std::optional<String> {
  if (auto entity = world->entity(entityId))
    return entity->name();

  return std::nullopt;
}

auto WorldEntityCallbacks::entityNametag(World* world, EntityId entityId) -> std::optional<Json> {
  auto entity = world->entity(entityId);

  Json result;
  if (auto nametagEntity = as<NametagEntity>(entity)) {
    result = JsonObject{
      {"nametag", nametagEntity->nametag()},
      {"displayed", nametagEntity->displayNametag()},
      {"color", jsonFromColor(Color::rgb(nametagEntity->nametagColor()))},
      {"origin", jsonFromVec2F(nametagEntity->nametagOrigin())},
    };
    if (auto status = nametagEntity->statusText())
      auto _ = result.set("status", *status);
  }

  return result;
}

auto WorldEntityCallbacks::entityDescription(World* world, EntityId entityId, std::optional<String> const& species) -> std::optional<String> {
  if (auto entity = world->entity(entityId)) {
    if (auto inspectableEntity = as<InspectableEntity>(entity)) {
      if (species)
        return inspectableEntity->inspectionDescription(*species);
    }

    return entity->description();
  }

  return {};
}

auto WorldEntityCallbacks::entityPortrait(World* world, EntityId entityId, String const& portraitMode) -> LuaNullTermWrapper<std::optional<List<Drawable>>> {
  if (auto portraitEntity = as<PortraitEntity>(world->entity(entityId)))
    return portraitEntity->portrait(PortraitModeNames.getLeft(portraitMode));

  return {};
}

auto WorldEntityCallbacks::entityHandItem(World* world, EntityId entityId, String const& handName) -> std::optional<String> {
  ToolHand toolHand;
  if (handName == "primary") {
    toolHand = ToolHand::Primary;
  } else if (handName == "alt") {
    toolHand = ToolHand::Alt;
  } else {
    throw StarException(strf("Unknown tool hand {}", handName));
  }

  if (auto entity = world->get<ToolUserEntity>(entityId)) {
    if (auto item = entity->handItem(toolHand)) {
      return item->name();
    }
  }

  return std::nullopt;
}

auto WorldEntityCallbacks::entityHandItemDescriptor(World* world, EntityId entityId, String const& handName) -> Json {
  ToolHand toolHand;
  if (handName == "primary") {
    toolHand = ToolHand::Primary;
  } else if (handName == "alt") {
    toolHand = ToolHand::Alt;
  } else {
    throw StarException(strf("Unknown tool hand {}", handName));
  }

  if (auto entity = world->get<ToolUserEntity>(entityId)) {
    if (auto item = entity->handItem(toolHand)) {
      return item->descriptor().toJson();
    }
  }

  return {};
}

auto WorldEntityCallbacks::entityUniqueId(World* world, EntityId entityId) -> LuaNullTermWrapper<std::optional<String>> {
  if (auto entity = world->entity(entityId))
    return entity->uniqueId();
  return {};
}

auto WorldEntityCallbacks::getObjectParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue) -> Json {
  Json val = Json();

  if (auto objectEntity = as<Object>(world->entity(entityId))) {
    val = objectEntity->configValue(parameterName);
    if (!val && defaultValue)
      val = *defaultValue;
  }

  return val;
}

auto WorldEntityCallbacks::getNpcScriptParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue) -> Json {
  Json val = Json();

  if (auto npcEntity = as<Npc>(world->entity(entityId))) {
    val = npcEntity->scriptConfigParameter(parameterName);
    if (!val && defaultValue)
      val = *defaultValue;
  }

  return val;
}

auto WorldEntityCallbacks::objectSpaces(World* world, EntityId entityId) -> List<Vec2I> {
  if (auto tileEntity = as<TileEntity>(world->entity(entityId)))
    return tileEntity->spaces();
  return {};
}

auto WorldEntityCallbacks::farmableStage(World* world, EntityId entityId) -> std::optional<int> {
  if (auto farmable = world->get<FarmableObject>(entityId)) {
    return farmable->stage();
  }

  return std::nullopt;
}

auto WorldEntityCallbacks::containerSize(World* world, EntityId entityId) -> std::optional<int> {
  if (auto container = world->get<ContainerObject>(entityId))
    return container->containerSize();

  return std::nullopt;
}

auto WorldEntityCallbacks::containerClose(World* world, EntityId entityId) -> bool {
  if (auto container = world->get<ContainerObject>(entityId)) {
    container->containerClose();
    return true;
  }

  return false;
}

auto WorldEntityCallbacks::containerOpen(World* world, EntityId entityId) -> bool {
  if (auto container = world->get<ContainerObject>(entityId)) {
    container->containerOpen();
    return true;
  }

  return false;
}

auto WorldEntityCallbacks::containerItems(World* world, EntityId entityId) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    JsonArray res;
    ConstPtr<ItemDatabase> itemDb = Root::singleton().itemDatabase();
    for (auto const& item : container->itemBag()->items())
      res.append(itemDb->toJson(item));
    return res;
  }

  return {};
}

auto WorldEntityCallbacks::containerItemAt(World* world, EntityId entityId, size_t offset) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto items = container->itemBag()->items();
    if (offset < items.size()) {
      return itemDb->toJson(items.at(offset));
    }
  }

  return {};
}

auto WorldEntityCallbacks::containerConsume(World* world, EntityId entityId, Json const& items) -> std::optional<bool> {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto toConsume = ItemDescriptor(items);
    return container->consumeItems(toConsume).result();
  }

  return std::nullopt;
}

auto WorldEntityCallbacks::containerConsumeAt(World* world, EntityId entityId, size_t offset, int count) -> std::optional<bool> {
  if (auto container = world->get<ContainerObject>(entityId)) {
    if (offset < container->containerSize()) {
      return container->consumeItems(offset, count).result();
    }
  }

  return std::nullopt;
}

auto WorldEntityCallbacks::containerAvailable(World* world, EntityId entityId, Json const& items) -> std::optional<size_t> {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemBag = container->itemBag();
    auto toCheck = ItemDescriptor(items);
    return itemBag->available(toCheck);
  }

  return std::nullopt;
}

auto WorldEntityCallbacks::containerTakeAll(World* world, EntityId entityId) -> Json {
  auto itemDb = Root::singleton().itemDatabase();
  if (auto container = world->get<ContainerObject>(entityId)) {
    if (auto itemList = container->clearContainer().result()) {
      JsonArray res;
      for (auto item : *itemList)
        res.append(itemDb->toJson(item));
      return res;
    }
  }

  return {};
}

auto WorldEntityCallbacks::containerTakeAt(World* world, EntityId entityId, size_t offset) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    if (offset < container->containerSize()) {
      if (auto res = container->takeItems(offset).result())
        return itemDb->toJson(*res);
    }
  }

  return {};
}

auto WorldEntityCallbacks::containerTakeNumItemsAt(World* world, EntityId entityId, size_t offset, int const& count) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    if (offset < container->containerSize()) {
      if (auto res = container->takeItems(offset, count).result())
        return itemDb->toJson(*res);
    }
  }

  return {};
}

auto WorldEntityCallbacks::containerItemsCanFit(World* world, EntityId entityId, Json const& items) -> std::optional<size_t> {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto itemBag = container->itemBag();
    auto toSearch = itemDb->fromJson(items);
    return itemBag->itemsCanFit(toSearch);
  }

  return std::nullopt;
}

auto WorldEntityCallbacks::containerItemsFitWhere(World* world, EntityId entityId, Json const& items) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto itemBag = container->itemBag();
    auto toSearch = itemDb->fromJson(items);
    auto res = itemBag->itemsFitWhere(toSearch);
    return JsonObject{
      {"leftover", res.leftover},
      {"slots", jsonFromList<size_t>(res.slots)}};
  }

  return {};
}

auto WorldEntityCallbacks::containerAddItems(World* world, EntityId entityId, Json const& items) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto toInsert = itemDb->fromJson(items);
    if (auto res = container->addItems(toInsert).result())
      return itemDb->toJson(*res);
  }

  return items;
}

auto WorldEntityCallbacks::containerStackItems(World* world, EntityId entityId, Json const& items) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto toInsert = itemDb->fromJson(items);
    if (auto res = container->addItems(toInsert).result())
      return itemDb->toJson(*res);
  }

  return items;
}

auto WorldEntityCallbacks::containerPutItemsAt(World* world, EntityId entityId, Json const& items, size_t offset) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto toInsert = itemDb->fromJson(items);
    if (offset < container->containerSize()) {
      if (auto res = container->putItems(offset, toInsert).result())
        return itemDb->toJson(*res);
    }
  }

  return items;
}

auto WorldEntityCallbacks::containerSwapItems(World* world, EntityId entityId, Json const& items, size_t offset) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto toSwap = itemDb->fromJson(items);
    if (offset < container->containerSize()) {
      if (auto res = container->swapItems(offset, toSwap, true).result())
        return itemDb->toJson(*res);
    }
  }

  return items;
}

auto WorldEntityCallbacks::containerSwapItemsNoCombine(World* world, EntityId entityId, Json const& items, size_t offset) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto toSwap = itemDb->fromJson(items);
    if (offset < container->containerSize()) {
      if (auto res = container->swapItems(offset, toSwap, false).result())
        return itemDb->toJson(*res);
    }
  }

  return items;
}

auto WorldEntityCallbacks::containerItemApply(World* world, EntityId entityId, Json const& items, size_t offset) -> Json {
  if (auto container = world->get<ContainerObject>(entityId)) {
    auto itemDb = Root::singleton().itemDatabase();
    auto toSwap = itemDb->fromJson(items);
    if (offset < container->containerSize()) {
      if (auto res = container->swapItems(offset, toSwap, false).result())
        return itemDb->toJson(*res);
    }
  }

  return items;
}

auto WorldEntityCallbacks::callScriptedEntity(World* world, EntityId entityId, String const& function, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> {
  auto entity = as<ScriptedEntity>(world->entity(entityId));
  if (!entity || !entity->isMaster())
    throw StarException::format("Entity {} does not exist or is not a local master scripted entity", entityId);
  return entity->callScript(function, args);
}

auto WorldEntityCallbacks::findUniqueEntity(World* world, String const& uniqueId) -> RpcPromise<Vec2F> {
  return world->findUniqueEntity(uniqueId);
}

auto WorldEntityCallbacks::sendEntityMessage(World* world, LuaEngine& engine, LuaValue entityId, String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
  if (entityId.is<LuaString>())
    return world->sendEntityMessage(engine.luaTo<String>(entityId), message, JsonArray::from(std::move(args)));
  else
    return world->sendEntityMessage(engine.luaTo<EntityId>(entityId), message, JsonArray::from(std::move(args)));
}

auto WorldEntityCallbacks::loungingEntities(World* world, EntityId entityId, std::optional<size_t> anchorIndex) -> std::optional<List<EntityId>> {
  if (auto entity = world->get<LoungeableEntity>(entityId))
    return entity->entitiesLoungingIn(anchorIndex.value()).values();
  return std::nullopt;
}

auto WorldEntityCallbacks::loungeableOccupied(World* world, EntityId entityId, std::optional<size_t> anchorIndex) -> std::optional<bool> {
  auto entity = world->get<LoungeableEntity>(entityId);
  size_t anchor = anchorIndex.value();
  if (entity && entity->anchorCount() > anchor)
    return !entity->entitiesLoungingIn(anchor).empty();
  return std::nullopt;
}

auto WorldEntityCallbacks::loungeableAnchorCount(World* world, EntityId entityId) -> std::optional<size_t> {
  if (auto entity = world->get<LoungeableEntity>(entityId))
    return entity->anchorCount();
  return std::nullopt;
}

auto WorldEntityCallbacks::isMonster(World* world, EntityId entityId, std::optional<bool> const& aggressive) -> bool {
  if (auto entity = world->get<Monster>(entityId))
    return !aggressive || *aggressive == entity->aggressive();

  return false;
}

auto WorldEntityCallbacks::monsterType(World* world, EntityId entityId) -> std::optional<String> {
  if (auto monster = world->get<Monster>(entityId))
    return monster->typeName();

  return std::nullopt;
}

auto WorldEntityCallbacks::npcType(World* world, EntityId entityId) -> std::optional<String> {
  if (auto npc = world->get<Npc>(entityId))
    return npc->npcType();

  return std::nullopt;
}

auto WorldEntityCallbacks::stagehandType(World* world, EntityId entityId) -> std::optional<String> {
  if (auto stagehand = world->get<Stagehand>(entityId))
    return stagehand->typeName();

  return std::nullopt;
}

auto WorldEntityCallbacks::isNpc(World* world, EntityId entityId, std::optional<int> const& damageTeam) -> bool {
  if (auto entity = world->get<Npc>(entityId)) {
    return !damageTeam || *damageTeam == entity->getTeam().team;
  }

  return false;
}

auto WorldEnvironmentCallbacks::lightLevel(World* world, Vec2F const& position) -> float {
  return world->lightLevel(position);
}

auto WorldEnvironmentCallbacks::windLevel(World* world, Vec2F const& position) -> float {
  return world->windLevel(position);
}

auto WorldEnvironmentCallbacks::breathable(World* world, Vec2F const& position) -> bool {
  return world->breathable(position);
}

auto WorldEnvironmentCallbacks::underground(World* world, Vec2F const& position) -> bool {
  return world->isUnderground(position);
}

auto WorldEnvironmentCallbacks::material(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName) -> LuaValue {
  TileLayer layer;
  if (layerName == "foreground") {
    layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported material layer {}", layerName));
  }

  auto materialId = world->material(Vec2I::floor(position), layer);
  if (materialId == NullMaterialId) {
    return LuaNil;
  } else if (materialId == EmptyMaterialId) {
    return false;
  } else {
    ConstPtr<MaterialDatabase> materialDatabase = Root::singleton().materialDatabase();
    return engine.createString(materialDatabase->materialName(materialId));
  }
}

auto WorldEnvironmentCallbacks::mod(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName) -> LuaValue {
  TileLayer layer;
  if (layerName == "foreground") {
    layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported mod layer {}", layerName));
  }

  auto modId = world->mod(Vec2I::floor(position), layer);
  if (isRealMod(modId)) {
    auto materialDatabase = Root::singleton().materialDatabase();
    return engine.createString(materialDatabase->modName(modId));
  }

  return LuaNil;
}

auto WorldEnvironmentCallbacks::materialHueShift(World* world, Vec2F const& position, String const& layerName) -> float {
  TileLayer layer;
  if (layerName == "foreground") {
    layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported material layer {}", layerName));
  }

  return world->materialHueShift(Vec2I::floor(position), layer);
}

auto WorldEnvironmentCallbacks::modHueShift(World* world, Vec2F const& position, String const& layerName) -> float {
  TileLayer layer;
  if (layerName == "foreground") {
    layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported material layer {}", layerName));
  }

  return world->modHueShift(Vec2I::floor(position), layer);
}

auto WorldEnvironmentCallbacks::materialColor(World* world, Vec2F const& position, String const& layerName) -> MaterialColorVariant {
  TileLayer layer;
  if (layerName == "foreground") {
    layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported material layer {}", layerName));
  }

  return world->colorVariant(Vec2I::floor(position), layer);
}

void WorldEnvironmentCallbacks::setMaterialColor(World* world, Vec2F const& position, String const& layerName, MaterialColorVariant color) {
  TileLayer layer;
  if (layerName == "foreground") {
    layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported material layer {}", layerName));
  }

  world->modifyTile(Vec2I::floor(position), PlaceMaterialColor{.layer = layer, .color = color}, true);
}

auto WorldEnvironmentCallbacks::damageTiles(World* world,
                                            List<Vec2I> const& arg1,
                                            String const& arg2,
                                            Vec2F const& arg3,
                                            String const& arg4,
                                            float arg5,
                                            std::optional<unsigned> const& arg6,
                                            std::optional<EntityId> sourceEntity) -> bool {
  List<Vec2I> tilePositions = arg1;

  TileLayer layer;
  auto layerName = arg2;
  if (layerName == "foreground") {
    layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported tile layer {}", layerName));
  }

  unsigned harvestLevel = 999;
  if (arg6)
    harvestLevel = *arg6;

  auto tileDamage = TileDamage(TileDamageTypeNames.getLeft(arg4), arg5, harvestLevel);
  auto res = world->damageTiles(tilePositions, layer, arg3, tileDamage, sourceEntity);
  return res != TileDamageResult::None;
}

auto WorldEnvironmentCallbacks::damageTileArea(World* world,
                                               Vec2F center,
                                               float radius,
                                               String layer,
                                               Vec2F sourcePosition,
                                               String damageType,
                                               float damage,
                                               std::optional<unsigned> const& harvestLevel,
                                               std::optional<EntityId> sourceEntity) -> bool {
  auto tiles = tileAreaBrush(radius, center, false);
  return damageTiles(world, tiles, layer, sourcePosition, damageType, damage, harvestLevel, sourceEntity);
}

auto WorldEnvironmentCallbacks::placeMaterial(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5) -> bool {
  auto tilePosition = arg1;

  PlaceMaterial placeMaterial;

  std::string layerName = arg2.utf8();
  auto split = layerName.find_first_of('+');
  if (split != std::numeric_limits<std::size_t>::max()) {
    auto overrideName = layerName.substr(split + 1);
    layerName = layerName.substr(0, split);
    if (overrideName == "empty" || overrideName == "none")
      placeMaterial.collisionOverride = TileCollisionOverride::Empty;
    else if (overrideName == "block")
      placeMaterial.collisionOverride = TileCollisionOverride::Block;
    else if (overrideName == "platform")
      placeMaterial.collisionOverride = TileCollisionOverride::Platform;
    else
      throw StarException(strf("Unsupported collision override {}", overrideName));
  }

  if (layerName == "foreground")
    placeMaterial.layer = TileLayer::Foreground;
  else if (layerName == "background")
    placeMaterial.layer = TileLayer::Background;
  else
    throw StarException(strf("Unsupported tile layer {}", layerName));

  auto materialName = arg3;
  auto materialDatabase = Root::singleton().materialDatabase();
  if (!materialDatabase->materialNames().contains(materialName))
    throw StarException(strf("Unknown material name {}", materialName));
  placeMaterial.material = materialDatabase->materialId(materialName);

  if (arg4)
    placeMaterial.materialHueShift = (MaterialHue)*arg4;

  bool allowOverlap = arg5;

  return world->modifyTile(tilePosition, placeMaterial, allowOverlap);
}

auto WorldEnvironmentCallbacks::replaceMaterials(World* world,
                                                 List<Vec2I> const& tilePositions,
                                                 String const& layer,
                                                 String const& materialName,
                                                 std::optional<int> const& hueShift,
                                                 bool enableDrops) -> bool {
  PlaceMaterial placeMaterial;

  std::string layerName = layer.utf8();
  auto split = layerName.find_first_of('+');
  if (split != std::numeric_limits<std::size_t>::max()) {
    auto overrideName = layerName.substr(split + 1);
    layerName = layerName.substr(0, split);
    if (overrideName == "empty" || overrideName == "none")
      placeMaterial.collisionOverride = TileCollisionOverride::Empty;
    else if (overrideName == "block")
      placeMaterial.collisionOverride = TileCollisionOverride::Block;
    else if (overrideName == "platform")
      placeMaterial.collisionOverride = TileCollisionOverride::Platform;
    else
      throw StarException(strf("Unsupported collision override {}", overrideName));
  }

  if (layerName == "foreground")
    placeMaterial.layer = TileLayer::Foreground;
  else if (layerName == "background")
    placeMaterial.layer = TileLayer::Background;
  else
    throw StarException(strf("Unsupported tile layer {}", layerName));

  auto materialDatabase = Root::singleton().materialDatabase();
  if (!materialDatabase->materialNames().contains(materialName))
    throw StarException(strf("Unknown material name {}", materialName));
  placeMaterial.material = materialDatabase->materialId(materialName);

  if (hueShift)
    placeMaterial.materialHueShift = (MaterialHue)*hueShift;

  TileModificationList modifications;
  for (auto pos : tilePositions) {
    if (!world->isTileConnectable(pos, placeMaterial.layer, true))
      continue;
    modifications.emplaceAppend(pos, placeMaterial);
  }

  if (modifications.empty())
    return true;

  TileDamage damage;
  if (enableDrops) {
    damage.amount = 1.0f;
    damage.harvestLevel = 999;
  } else {
    damage.amount = -1.0f;
  }

  return world->replaceTiles(modifications, damage).empty();
  ;
}

auto WorldEnvironmentCallbacks::replaceMaterialArea(World* world,
                                                    Vec2F center,
                                                    float radius,
                                                    String const& layer,
                                                    String const& materialName,
                                                    std::optional<int> const& hueShift,
                                                    bool enableDrops) -> bool {
  auto tiles = tileAreaBrush(radius, center, false);
  return replaceMaterials(world, tiles, layer, materialName, hueShift, enableDrops);
}

auto WorldEnvironmentCallbacks::placeMod(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5) -> bool {
  auto tilePosition = arg1;

  PlaceMod placeMod;

  auto layerName = arg2;
  if (layerName == "foreground") {
    placeMod.layer = TileLayer::Foreground;
  } else if (layerName == "background") {
    placeMod.layer = TileLayer::Background;
  } else {
    throw StarException(strf("Unsupported tile layer {}", layerName));
  }

  auto modName = arg3;
  auto materialDatabase = Root::singleton().materialDatabase();
  if (!materialDatabase->modNames().contains(modName))
    throw StarException(strf("Unknown mod name {}", modName));
  placeMod.mod = materialDatabase->modId(modName);

  if (arg4)
    placeMod.modHueShift = (MaterialHue)*arg4;

  bool allowOverlap = arg5;

  return world->modifyTile(tilePosition, placeMod, allowOverlap);
}
}// namespace Star::LuaBindings
