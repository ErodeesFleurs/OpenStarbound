import star.json_extra;
import star.json;
import star.xx_hash;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.material_types;
import star.rpc_promise;
import star.vector;
import star.rect;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.poly;
import star.color;
import star.string;
import star.lua;
import star.weighted_pool;
import star.logging;
import star.parametric_function;
import star.perlin;
import star.strong_typedef;
import star.set;
import star.math_common;
import star.periodic;
import star.periodic_function;
import star.audio;
import star.map;
import star.ordered_set;
import star.either;
import star.io_device;
import star.ref_ptr;
import star.listener;
import std;
import star.memory;
import star.format;
import star.version;
import star.image;
import star.worker_pool;

import std;
import star.sector_array_2d;
import star.net_compatibility;
import star.btree;
import star.data_stream_devices;
import star.btree_database;

import star.utility_lua_bindings;
import star.lua_root;

import star.lua_actor_movement_component;
import star.item;
import star.lua_components;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.entity_rendering;
import star.object;
import star.container_entity;
import star.container_object;
import star.loungeable_object;
import star.farmable_object;
import star.item_database;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.tile_modification;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.item_descriptor;
import star.item_recipe;
import star.drawable;
import star.collision_block;
import star.platformer_astar;
import star.blocks_along_line;
import star.entity_rendering_types;
import star.lounging_entities;
import star.uuid;
import star.humanoid;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.ai_types;
import star.player_types;
import star.radio_message_database;
import star.player;
import star.aggressive_entity;
import star.monster_database;
import star.effect_emitter;
import star.monster;
import star.npc_database;
import star.npc;
import star.asset_source;
import star.assets;
import star.configuration;
import star.root;
import star.stagehand;
import star.projectile;
import star.tile_sector_array;
import star.liquid_types;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.chat_types;
import star.warping;
import star.plant;
import star.versioning_database;
import star.world_storage;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawner;
import star.world_server;
import star.ambient;
import star.weather;
import star.world_client;
import star.world_template;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.item_drop;
import star.lua_game_converters;
import star.universe_settings;
import star.plant_database;
import star.biome_placement;
import star.spawn_type_database;
import star.biome;
import star.vehicle;
import star.treasure;

import star.projectile_database;
import star.vehicle_database;
import star.stagehand_database;
import star.material_database;
import star.object_database;
import star.image_metadata_database;


import star.player_inventory;
import star.item_bag;

import star.sky;


import star.line;
import star.algorithm;
import star.pythonic;
namespace Star {

class World;
class WorldServer;
class WorldClient;
class Item;
class ScriptedEntity;

namespace LuaBindings {
  typedef std::function<Json(std::shared_ptr<ScriptedEntity> const& entity, String const& functionName, JsonArray const& args)> CallEntityScriptFunction;


  void addWorldDebugCallbacks(Root& root, LuaCallbacks& callbacks);
  void addWorldEntityCallbacks(LuaCallbacks& callbacks, World* world);
  void addWorldEnvironmentCallbacks(LuaCallbacks& callbacks, World* world);
  void addWorldGeometryCallbacks(LuaCallbacks& callbacks, World* world);
  void addWorldCollisionCallbacks(LuaCallbacks& callbacks, World* world);

  namespace WorldCallbacks {
    bool placeObject(World* world, String const& arg1, Vec2I const& arg2, std::optional<int> const& arg3, Json const& arg4);
    std::optional<EntityId> spawnItem(World* world, Json const& itemType, Vec2F const& worldPosition, std::optional<std::size_t> const& inputCount, Json const& inputParameters, std::optional<Vec2F> const& initialVelocity, std::optional<float> const& intangibleTime);
    List<EntityId> spawnTreasure(World* world, Vec2F const& position, String const& pool, float level, std::optional<std::uint64_t> seed);
    std::optional<EntityId> spawnMonster(World* world, String const& arg1, Vec2F const& arg2, std::optional<JsonObject> const& arg3);
    std::optional<EntityId> spawnNpc(World* world, Vec2F const& arg1, String const& arg2, String const& arg3, float arg4, std::optional<std::uint64_t> arg5, Json const& arg6);
    std::optional<EntityId> spawnStagehand(World* world, Vec2F const& spawnPosition, String const& typeName, Json const& overrides);
    std::optional<EntityId> spawnProjectile(World* world, String const& arg1, Vec2F const& arg2, std::optional<EntityId> const& arg3, std::optional<Vec2F> const& arg4, bool arg5, Json const& arg6);
    std::optional<EntityId> spawnVehicle(World* world, String const& vehicleName, Vec2F const& pos, Json const& extraConfig);
    Json getProperty(World* world, String const& arg1, Json const& arg2);
    void setProperty(World* world, String const& arg1, Json const& arg2);
    std::optional<PlatformerAStar::Path> findPlatformerPath(World* world, Vec2F const& start, Vec2F const& end, ActorMovementParameters actorMovementParameters, PlatformerAStar::Parameters searchParameters);
    PlatformerAStar::PathFinder platformerPathStart(World* world, Vec2F const& start, Vec2F const& end, ActorMovementParameters actorMovementParameters, PlatformerAStar::Parameters searchParameters);
  }

  namespace ClientWorldCallbacks {
    void resendEntity(WorldClient* world, EntityId arg1);
    RectI clientWindow(WorldClient* world);
  }

  namespace ServerWorldCallbacks {
    String id(WorldServer* world);
    bool breakObject(WorldServer* world, EntityId arg1, bool arg2);
    bool isVisibleToPlayer(WorldServer* world, RectF const& arg1);
    bool loadRegion(WorldServer* world, RectF const& arg1);
    bool regionActive(WorldServer* world, RectF const& arg1);
    void setTileProtection(WorldServer* world, DungeonId arg1, bool arg2);
    bool isPlayerModified(WorldServer* world, RectI const& region);
    std::optional<LiquidLevel> forceDestroyLiquid(WorldServer* world, Vec2F const& position);
    EntityId loadUniqueEntity(WorldServer* world, String const& uniqueId);
    void setUniqueId(WorldServer* world, EntityId entityId, std::optional<String> const& uniqueId);
    Json takeItemDrop(World* world, EntityId entityId, std::optional<EntityId> const& takenBy);
    void setPlayerStart(World* world, Vec2F const& playerStart, std::optional<bool> respawnInWorld);
    List<EntityId> players(World* world);
    LuaString fidelity(World* world, LuaEngine& engine);
    std::optional<LuaValue> callScriptContext(World* world, String const& contextName, String const& function, LuaVariadic<LuaValue> const& args);
    bool sendPacket(WorldServer* world, ConnectionId clientId, String const& packetType, Json const& packetData);
  }

  namespace WorldGeometryCallbacks {
    float magnitude(World* world, Vec2F pos1, std::optional<Vec2F> pos2);
    Vec2F distance(World* world, Vec2F const& arg1, Vec2F const& arg2);
    bool polyContains(World* world, PolyF const& poly, Vec2F const& pos);
    LuaValue xwrap(World* world, LuaEngine& engine, LuaValue const& positionOrX);
    LuaValue nearestTo(World* world, LuaEngine& engine, std::variant<Vec2F, float> const& sourcePositionOrX, std::variant<Vec2F, float> const& targetPositionOrX);
  }

  namespace WorldCollisionCallbacks {
    bool rectCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2);
    bool pointTileCollision(World* world, Vec2F const& arg1, std::optional<CollisionSet> const& arg2);
    bool lineTileCollision(World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3);
    std::optional<std::pair<Vec2F, Vec2I>> lineTileCollisionPoint(World* world, Vec2F const& start, Vec2F const& end, std::optional<CollisionSet> const& maybeCollisionSet);
    bool rectTileCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2);
    bool pointCollision(World* world, Vec2F const& point, std::optional<CollisionSet> const& collisionSet);
    LuaTupleReturn<std::optional<Vec2F>, std::optional<Vec2F>> lineCollision(World* world, Vec2F const& start, Vec2F const& end, std::optional<CollisionSet> const& maybeCollisionSet);
    bool polyCollision(World* world, PolyF const& arg1, std::optional<Vec2F> const& arg2, std::optional<CollisionSet> const& arg3);
    List<Vec2I> collisionBlocksAlongLine(World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3, std::optional<int> const& arg4);
    std::optional<Vec2F> resolvePolyCollision(World* world, PolyF poly, Vec2F const& position, float maximumCorrection, std::optional<CollisionSet> const& collisionSet);
  }

  namespace WorldDebugCallbacks {
    void debugPoint(Root& root, Vec2F const& arg1, Color const& arg2);
    void debugLine(Root& root, Vec2F const& arg1, Vec2F const& arg2, Color const& arg3);
    void debugPoly(Root& root, PolyF const& poly, Color const& color);
    void debugText(Root& root, LuaEngine& engine, LuaVariadic<LuaValue> const& args);
  }

  namespace WorldEntityCallbacks {
    LuaTable entityQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options);
    LuaTable monsterQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options);
    LuaTable npcQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options);
    LuaTable objectQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options);
    LuaTable itemDropQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options);
    LuaTable playerQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options);
    LuaTable loungeableQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options);
    LuaTable entityLineQuery(World* world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, std::optional<LuaTable> options);
    LuaTable objectLineQuery(World* world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, std::optional<LuaTable> options);
    LuaTable npcLineQuery(World* world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, std::optional<LuaTable> options);
    bool entityExists(World* world, EntityId entityId);
    bool entityCanDamage(World* world, EntityId sourceId, EntityId targetId);
    Json entityDamageTeam(World* world, EntityId entityId);
    bool entityAggressive(World* world, EntityId entityId);
    std::optional<LuaString> entityType(World* world, LuaEngine& engine, EntityId entityId);
    std::optional<Vec2F> entityPosition(World* world, EntityId entityId);
    std::optional<Vec2F> entityVelocity(World* world, EntityId entityId);
    std::optional<RectF> entityMetaBoundBox(World* world, EntityId entityId);
    std::optional<std::uint64_t> entityCurrency(World* world, EntityId entityId, String const& currencyType);
    std::optional<std::uint64_t> entityHasCountOfItem(World* world, EntityId entityId, Json descriptor, std::optional<bool> exactMatch);
    std::optional<Vec2F> entityHealth(World* world, EntityId entityId);
    std::optional<String> entitySpecies(World* world, EntityId entityId);
    std::optional<String> entityGender(World* world, EntityId entityId);
    std::optional<String> entityName(World* world, EntityId entityId);
    std::optional<Json> entityNametag(World* world, EntityId entityId);
    std::optional<String> entityDescription(World* world, EntityId entityId, std::optional<String> const& species);
    LuaNullTermWrapper<std::optional<List<Drawable>>> entityPortrait(World* world, EntityId entityId, String const& portraitMode);
    std::optional<String> entityHandItem(World* world, EntityId entityId, String const& handName);
    Json entityHandItemDescriptor(World* world, EntityId entityId, String const& handName);
    LuaNullTermWrapper<std::optional<String>> entityUniqueId(World* world, EntityId entityId);
    Json getObjectParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue);
    Json getNpcScriptParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue);
    List<Vec2I> objectSpaces(World* world, EntityId entityId);
    std::optional<int> farmableStage(World* world, EntityId entityId);
    std::optional<int> containerSize(World* world, EntityId entityId);
    bool containerClose(World* world, EntityId entityId);
    bool containerOpen(World* world, EntityId entityId);
    Json containerItems(World* world, EntityId entityId);
    Json containerItemAt(World* world, EntityId entityId, std::size_t offset);
    std::optional<bool> containerConsume(World* world, EntityId entityId, Json const& items);
    std::optional<bool> containerConsumeAt(World* world, EntityId entityId, std::size_t offset, int count);
    std::optional<std::size_t> containerAvailable(World* world, EntityId entityId, Json const& items);
    Json containerTakeAll(World* world, EntityId entityId);
    Json containerTakeAt(World* world, EntityId entityId, std::size_t offset);
    Json containerTakeNumItemsAt(World* world, EntityId entityId, std::size_t offset, int const& count);
    std::optional<std::size_t> containerItemsCanFit(World* world, EntityId entityId, Json const& items);
    Json containerItemsFitWhere(World* world, EntityId entityId, Json const& items);
    Json containerAddItems(World* world, EntityId entityId, Json const& items);
    Json containerStackItems(World* world, EntityId entityId, Json const& items);
    Json containerPutItemsAt(World* world, EntityId entityId, Json const& items, std::size_t offset);
    Json containerSwapItems(World* world, EntityId entityId, Json const& items, std::size_t offset);
    Json containerSwapItemsNoCombine(World* world, EntityId entityId, Json const& items, std::size_t offset);
    Json containerItemApply(World* world, EntityId entityId, Json const& items, std::size_t offset);
    std::optional<LuaValue> callScriptedEntity(World* world, EntityId entityId, String const& function, LuaVariadic<LuaValue> const& args);
    RpcPromise<Vec2F> findUniqueEntity(World* world, String const& uniqueId);
    RpcPromise<Json> sendEntityMessage(World* world, LuaEngine& engine, LuaValue entityId, String const& message, LuaVariadic<Json> args);
    std::optional<List<EntityId>> loungingEntities(World* world, EntityId entityId, std::optional<std::size_t> anchorIndex);
    std::optional<bool> loungeableOccupied(World* world, EntityId entityId, std::optional<std::size_t> anchorIndex);
    std::optional<std::size_t> loungeableAnchorCount(World* world, EntityId entityId);
    bool isMonster(World* world, EntityId entityId, std::optional<bool> const& aggressive);
    std::optional<String> monsterType(World* world, EntityId entityId);
    std::optional<String> npcType(World* world, EntityId entityId);
    std::optional<String> stagehandType(World* world, EntityId entityId);
    bool isNpc(World* world, EntityId entityId, std::optional<int> const& damageTeam);
  }

  namespace WorldEnvironmentCallbacks {
    float lightLevel(World* world, Vec2F const& position);
    float windLevel(World* world, Vec2F const& position);
    bool breathable(World* world, Vec2F const& position);
    bool underground(World* world, Vec2F const& position);
    LuaValue material(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName);
    LuaValue mod(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName);
    float materialHueShift(World* world, Vec2F const& position, String const& layerName);
    float modHueShift(World* world, Vec2F const& position, String const& layerName);
    MaterialColorVariant materialColor(World* world, Vec2F const& position, String const& layerName);
    void setMaterialColor(World* world, Vec2F const& position, String const& layerName, MaterialColorVariant color);
    bool damageTiles(World* world, List<Vec2I> const& arg1, String const& arg2, Vec2F const& arg3, String const& arg4, float arg5, std::optional<unsigned> const& arg6, std::optional<EntityId> sourceEntity);
    bool damageTileArea(World* world, Vec2F center, float radius, String layer, Vec2F sourcePosition, String damageType, float damage, std::optional<unsigned> const& harvestLevel, std::optional<EntityId> sourceEntity);
    bool placeMaterial(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5);
    bool replaceMaterials(World* world, List<Vec2I> const& tilePositions, String const& layer, String const& materialName, std::optional<int> const& hueShift, bool enableDrops);
    bool replaceMaterialArea(World* world, Vec2F center, float radius, String const& layer, String const& materialName, std::optional<int> const& hueShift, bool enableDrops);
    bool placeMod(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5);
    List<std::pair<Vec2I, LiquidLevel>> liquidAlongLine(World* world, Vec2F const& start, Vec2F const& end);
    bool tileIsOccupied(World* world, Vec2I const& arg1, std::optional<bool> const& arg2, std::optional<bool> const& arg3);
    double time(World* world);
    std::uint64_t day(World* world);
    double timeOfDay(World* world);
    float dayLength(World* world);
    std::optional<LiquidLevel> liquidAt(World* world, std::variant<RectF, Vec2I> boundBoxOrPoint);
    float gravity(World* world, Vec2F const& arg1);
    bool spawnLiquid(World* world, Vec2F const& arg1, LiquidId arg2, float arg3);
    std::optional<LiquidLevel> destroyLiquid(World* world, Vec2F const& position);
    bool isTileProtected(World* world, Vec2F const& position);
  }
}

}

namespace Star {
namespace LuaBindings {

  enum class EntityBoundMode { MetaBoundBox, CollisionArea, Position };

  EnumMap<EntityBoundMode> const EntityBoundModeNames = {
    {EntityBoundMode::MetaBoundBox, "MetaBoundBox"},
    {EntityBoundMode::CollisionArea, "CollisionArea"},
    {EntityBoundMode::Position, "Position"}
  };

  template <typename EntityT>
  using Selector = std::function<bool(std::shared_ptr<EntityT> const&)>;

  template <typename EntityT>
  LuaTable entityQueryImpl(World* world, LuaEngine& engine, LuaTable const& options, Selector<EntityT> selector) {
    std::optional<EntityId> withoutEntityId = options.get<std::optional<EntityId>>("withoutEntityId");
    std::optional<Set<EntityType>> includedTypes;
    if (auto types = options.get<std::optional<LuaTable>>("includedTypes")) {
      includedTypes = Set<EntityType>();
      types->iterate([&includedTypes](LuaValue const&, LuaString const& type) {
        if (type == "mobile") {
          includedTypes->add(EntityType::Player);
          includedTypes->add(EntityType::Monster);
          includedTypes->add(EntityType::Npc);
          includedTypes->add(EntityType::Projectile);
          includedTypes->add(EntityType::ItemDrop);
          includedTypes->add(EntityType::PlantDrop);
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

    std::optional<String> callScript = options.get<std::optional<String>>("callScript");
    List<LuaValue> callScriptArgs = options.get<std::optional<List<LuaValue>>>("callScriptArgs").value_or(List<LuaValue>{});
    LuaValue callScriptResult = options.get<std::optional<LuaValue>>("callScriptResult").value_or(LuaBoolean(true));

    std::optional<Line2F> lineQuery = options.get<std::optional<Line2F>>("line");
    std::optional<PolyF> polyQuery = options.get<std::optional<PolyF>>("poly");
    std::optional<RectF> rectQuery = options.get<std::optional<RectF>>("rect");
    std::optional<std::pair<Vec2F, float>> radiusQuery;
    if (auto radius = options.get<std::optional<float>>("radius"))
      radiusQuery = std::make_pair(options.get<Vec2F>("center"), *radius);
    
    bool globalQuery = !lineQuery && !polyQuery && !rectQuery && !radiusQuery;

    EntityBoundMode boundMode = EntityBoundModeNames.getLeft(options.get<std::optional<String>>("boundMode").value_or("CollisionArea"));
    std::optional<LuaString> order = options.get<std::optional<LuaString>>("order");

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
      if (globalQuery) {
        return true;
      } else if (boundMode == EntityBoundMode::MetaBoundBox) {
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
    } else {
      // global query
      world->forAllEntities([&](std::shared_ptr<Entity> const& entity) {
          if (auto ofType = as<EntityT>(entity)) {
            if (innerSelector(ofType))
              entities.emplace_back(ofType);
          }
      });
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
            [world, nearestPosition](std::shared_ptr<EntityT> const& entity) {
              return world->geometry().diff(entity->position(), nearestPosition).magnitude();
            });
      } else if (*order == "random") {
        world->root().random().shuffle(entities);
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
  LuaTable entityQuery(World* world,
      LuaEngine& engine,
      Vec2F const& pos1,
      LuaValue const& pos2,
      std::optional<LuaTable> options,
      Selector<EntityT> selector = {}) {
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
  LuaTable entityLineQuery(World* world,
      LuaEngine& engine,
      Vec2F const& point1,
      Vec2F const& point2,
      std::optional<LuaTable> options,
      Selector<EntityT> selector = {}) {
    Line2F line(point1, point2);

    if (!options)
      options = engine.createTable();

    options->set("line", line);

    return entityQueryImpl<EntityT>(world, engine, *options, selector);
  }
  
  // Thread-safe callbacks that can be used from any thread under the world
  LuaCallbacks makeWorldThreadCallbacks(Root& root, World* world) {
    LuaCallbacks callbacks;
    
    addWorldDebugCallbacks(root, callbacks);
    addWorldGeometryCallbacks(callbacks, world);
    
    return callbacks;
  }

  LuaCallbacks makeWorldCallbacks(Root& root, World* world) {
    LuaCallbacks callbacks = makeWorldThreadCallbacks(root, world);

    addWorldEnvironmentCallbacks(callbacks, world);
    addWorldEntityCallbacks(callbacks, world);
    addWorldCollisionCallbacks(callbacks, world);

    callbacks.registerCallbackWithSignature<bool, String, Vec2I, std::optional<int>, Json>("placeObject", std::bind(WorldCallbacks::placeObject, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<std::optional<EntityId>, Json, Vec2F, std::optional<std::size_t>, Json, std::optional<Vec2F>, std::optional<float>>("spawnItem", std::bind(WorldCallbacks::spawnItem, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    callbacks.registerCallbackWithSignature<List<EntityId>, Vec2F, String, float, std::optional<std::uint64_t>>("spawnTreasure", std::bind(WorldCallbacks::spawnTreasure, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<std::optional<EntityId>, String, Vec2F, std::optional<JsonObject>>("spawnMonster", std::bind(WorldCallbacks::spawnMonster, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<EntityId>, Vec2F, String, String, float, std::optional<std::uint64_t>, Json>("spawnNpc", std::bind(WorldCallbacks::spawnNpc, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    callbacks.registerCallbackWithSignature<std::optional<EntityId>, Vec2F, String, Json>("spawnStagehand", std::bind(WorldCallbacks::spawnStagehand, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<EntityId>, String, Vec2F, std::optional<EntityId>, std::optional<Vec2F>, bool, Json>("spawnProjectile", std::bind(WorldCallbacks::spawnProjectile, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    callbacks.registerCallbackWithSignature<std::optional<EntityId>, String, Vec2F, Json>("spawnVehicle", std::bind(WorldCallbacks::spawnVehicle, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<Json, String, Json>("getProperty", std::bind(WorldCallbacks::getProperty, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<void, String, Json>("setProperty", std::bind(WorldCallbacks::setProperty, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<PlatformerAStar::Path>, Vec2F, Vec2F, ActorMovementParameters, PlatformerAStar::Parameters>("findPlatformerPath", std::bind(WorldCallbacks::findPlatformerPath, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<PlatformerAStar::PathFinder, Vec2F, Vec2F, ActorMovementParameters, PlatformerAStar::Parameters>("platformerPathStart", std::bind(WorldCallbacks::platformerPathStart, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

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

    callbacks.registerCallback("biomeBlocksAt", [world](Vec2I position) -> std::optional<List<MaterialId>> {
        std::shared_ptr<WorldTemplate const> worldTemplate;
        if (auto worldClient = as<WorldClient>(world))
          worldTemplate = worldClient->currentTemplate();
        else if (auto worldServer = as<WorldServer>(world))
          worldTemplate = worldServer->worldTemplate();

        if (worldTemplate) {
          WorldTemplate::BlockInfo block = worldTemplate->blockInfo(position[0], position[1]);
          if (auto biome = worldTemplate->biome(block.blockBiomeIndex)) {
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

    callbacks.registerCallback("biomeAt", [world](Vec2I position) -> Json {
        std::shared_ptr<WorldTemplate const> worldTemplate;
        if (auto worldClient = as<WorldClient>(world))
          worldTemplate = worldClient->currentTemplate();
        else if (auto worldServer = as<WorldServer>(world))
          worldTemplate = worldServer->worldTemplate();

        if (worldTemplate) {
          WorldTemplate::BlockInfo block = worldTemplate->blockInfo(position[0], position[1]);
          if (auto biome = worldTemplate->biome(block.blockBiomeIndex)) {
            return biome->toJson();
          }
        }
        return {};
      });
    callbacks.registerCallback("blockInfoAt", [world](Vec2I position) -> Json {
        std::shared_ptr<WorldTemplate const> worldTemplate;
        if (auto worldClient = as<WorldClient>(world))
          worldTemplate = worldClient->currentTemplate();
        else if (auto worldServer = as<WorldServer>(world))
          worldTemplate = worldServer->worldTemplate();

        if (worldTemplate) {
          return worldTemplate->blockInfo(position[0], position[1]).toJson();
        }
        return {};
      });

    if (auto clientWorld = as<WorldClient>(world)) {
      callbacks.registerCallback("inWorld", [clientWorld]() { return clientWorld->inWorld(); });
      callbacks.registerCallback("mainPlayer", [clientWorld]() -> std::optional<int> {
        if (clientWorld->isHeadless()) {
          return {};
        } else {
          return clientWorld->clientState().playerId(); 
        }
      });
      callbacks.registerCallback("headless", [clientWorld]() { return clientWorld->isHeadless();  });
      callbacks.registerCallback("subWorldId", [clientWorld]() { return clientWorld->subWorldId();  });
      callbacks.registerCallback("isClient", []() { return true;  });
      callbacks.registerCallback("isServer", []() { return false; });
      callbacks.registerCallback("latency", [clientWorld]() { return clientWorld->latency(); });
      callbacks.registerCallbackWithSignature<void, EntityId>("resendEntity", std::bind(ClientWorldCallbacks::resendEntity, clientWorld, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<RectI>("clientWindow", std::bind(ClientWorldCallbacks::clientWindow, clientWorld));
      callbacks.registerCallback("players", [clientWorld]() {
        List<EntityId> playerIds;

        clientWorld->forAllEntities([&](std::shared_ptr<Entity> const& entity) {
          if (entity->entityType() == EntityType::Player)
            playerIds.emplace_back(entity->entityId());
        });

        return playerIds;
      });
      callbacks.registerCallback("callScriptContext", [clientWorld](String const& contextName, String const& function, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> {
        auto context = clientWorld->scriptContext(contextName);
        if (!context)
          throw StarException::format("Context {} does not exist", contextName);
        return context->invoke(function, args);
      });
      callbacks.registerCallback("unload", [clientWorld]() {
        if (!clientWorld->isHeadless())
          throw StarException::format("Non-headless worlds cannot be unloaded.");
        clientWorld->requestDestroy();
      });
      callbacks.registerCallback("template", [clientWorld]() {
        return clientWorld->currentTemplate()->store();
      });
      callbacks.registerCallback("setTemplate", [clientWorld](Json worldTemplate) {
        clientWorld->setTemplate(worldTemplate);
      });
      callbacks.registerCallback("wire", [clientWorld](Vec2I outputPosition, std::size_t outputIndex, Vec2I inputPosition, std::size_t inputIndex) {
        clientWorld->wire(outputPosition, outputIndex, inputPosition, inputIndex);
      });
    }

    if (auto serverWorld = as<WorldServer>(world)) {
      callbacks.registerCallback("isClient", []() { return false; });
      callbacks.registerCallback("isServer", []() { return true;  });

      callbacks.registerCallbackWithSignature<String>("id", std::bind(ServerWorldCallbacks::id, serverWorld));
      callbacks.registerCallbackWithSignature<bool, EntityId, bool>("breakObject", std::bind(ServerWorldCallbacks::breakObject, serverWorld, std::placeholders::_1, std::placeholders::_2));
      callbacks.registerCallbackWithSignature<bool, RectF>("isVisibleToPlayer", std::bind(ServerWorldCallbacks::isVisibleToPlayer, serverWorld, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<bool, RectF>("loadRegion", std::bind(ServerWorldCallbacks::loadRegion, serverWorld, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<bool, RectF>("regionActive", std::bind(ServerWorldCallbacks::regionActive, serverWorld, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<void, DungeonId, bool>("setTileProtection", std::bind(ServerWorldCallbacks::setTileProtection, serverWorld, std::placeholders::_1, std::placeholders::_2));
      callbacks.registerCallbackWithSignature<bool, RectI>("isPlayerModified", std::bind(ServerWorldCallbacks::isPlayerModified, serverWorld, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<std::optional<LiquidLevel>, Vec2F>("forceDestroyLiquid", std::bind(ServerWorldCallbacks::forceDestroyLiquid, serverWorld, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<EntityId, String>("loadUniqueEntity", std::bind(ServerWorldCallbacks::loadUniqueEntity, serverWorld, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<void, EntityId, String>("setUniqueId", std::bind(ServerWorldCallbacks::setUniqueId, serverWorld, std::placeholders::_1, std::placeholders::_2));
      callbacks.registerCallbackWithSignature<Json, EntityId, std::optional<EntityId>>("takeItemDrop", std::bind(ServerWorldCallbacks::takeItemDrop, world, std::placeholders::_1, std::placeholders::_2));
      callbacks.registerCallbackWithSignature<void, Vec2F, std::optional<bool>>("setPlayerStart", std::bind(ServerWorldCallbacks::setPlayerStart, world, std::placeholders::_1, std::placeholders::_2));
      callbacks.registerCallbackWithSignature<List<EntityId>>("players", std::bind(ServerWorldCallbacks::players, world));
      callbacks.registerCallbackWithSignature<LuaString, LuaEngine&>("fidelity", std::bind(ServerWorldCallbacks::fidelity, world, std::placeholders::_1));
      callbacks.registerCallbackWithSignature<std::optional<LuaValue>, String, String, LuaVariadic<LuaValue>>("callScriptContext", std::bind(ServerWorldCallbacks::callScriptContext, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      callbacks.registerCallbackWithSignature<bool, ConnectionId, String, Json>("sendPacket", std::bind(ServerWorldCallbacks::sendPacket, serverWorld, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

      callbacks.registerCallbackWithSignature<double>("skyTime", [serverWorld]() {
          return serverWorld->sky()->epochTime();
        });
      callbacks.registerCallbackWithSignature<void, double>("setSkyTime", [serverWorld](double skyTime) {
          return serverWorld->sky()->setEpochTime(skyTime);
        });

      callbacks.registerCallback("expiryTime", [serverWorld]() -> float { return serverWorld->expiryTime(); });
      callbacks.registerCallback("setExpiryTime", [serverWorld](float expiryTime) { serverWorld->setExpiryTime(expiryTime); });

      callbacks.registerCallback("wire", [serverWorld](Vec2I outputPosition, std::size_t outputIndex, Vec2I inputPosition, std::size_t inputIndex) {
        serverWorld->wire(outputPosition, outputIndex, inputPosition, inputIndex);
      });

      callbacks.registerCallback("flyingType", [serverWorld]() -> String { return FlyingTypeNames.getRight(serverWorld->sky()->flyingType()); });
      callbacks.registerCallback("warpPhase", [serverWorld]() -> String { return WarpPhaseNames.getRight(serverWorld->sky()->warpPhase()); });
      callbacks.registerCallback("setUniverseFlag", [serverWorld](String flagName) { return serverWorld->universeSettings()->setFlag(flagName); });
      callbacks.registerCallback("universeFlags", [serverWorld]() { return serverWorld->universeSettings()->flags(); });
      callbacks.registerCallback("universeFlagSet", [serverWorld](String const& flagName) { return serverWorld->universeSettings()->flags().contains(flagName); });
      callbacks.registerCallback("placeDungeon", [serverWorld](String dungeonName, Vec2I position, std::optional<DungeonId> dungeonId) -> bool {
          return serverWorld->placeDungeon(dungeonName, position, dungeonId);
        });
      callbacks.registerCallback("tryPlaceDungeon", [serverWorld](String dungeonName, Vec2I position, std::optional<DungeonId> dungeonId) -> bool {
          return serverWorld->placeDungeon(dungeonName, position, dungeonId, false);
        });

      callbacks.registerCallback("addBiomeRegion", [serverWorld](Vec2I position, String biomeName, String subBlockSelector, int width) {
          serverWorld->addBiomeRegion(position, biomeName, subBlockSelector, width);
        });
      callbacks.registerCallback("expandBiomeRegion", [serverWorld](Vec2I position, int width) {
          serverWorld->expandBiomeRegion(position, width);
        });

      callbacks.registerCallback("pregenerateAddBiome", [serverWorld](Vec2I position, int width) {
          return serverWorld->pregenerateAddBiome(position, width);
        });
      callbacks.registerCallback("pregenerateExpandBiome", [serverWorld](Vec2I position, int width) {
          return serverWorld->pregenerateExpandBiome(position, width);
        });

      callbacks.registerCallback("setLayerEnvironmentBiome", [serverWorld](Vec2I position) {
          serverWorld->setLayerEnvironmentBiome(position);
        });

      callbacks.registerCallback("setPlanetType", [serverWorld](String planetType, String primaryBiomeName) {
          serverWorld->setPlanetType(planetType, primaryBiomeName);
        });

      callbacks.registerCallback("setDungeonGravity", [serverWorld](DungeonId dungeonId, std::optional<float> gravity) {
          serverWorld->setDungeonGravity(dungeonId, gravity);
        });

      callbacks.registerCallback("setDungeonBreathable", [serverWorld](DungeonId dungeonId, std::optional<bool> breathable) {
          serverWorld->setDungeonBreathable(dungeonId, breathable);
        });

      callbacks.registerCallback("setDungeonId", [serverWorld](RectI tileRegion, DungeonId dungeonId) {
          serverWorld->setDungeonId(tileRegion, dungeonId);
        });

      callbacks.registerCallback("enqueuePlacement", [serverWorld](List<Json> distributionConfigs, std::optional<DungeonId> id) {
          auto distributions = distributionConfigs.transformed([serverWorld](Json const& config) {
            return BiomeItemDistribution(serverWorld->root(), config, serverWorld->root().random().randu64());
          });
          return serverWorld->enqueuePlacement(std::move(distributions), id);
        });
      callbacks.registerCallback("template", [serverWorld]() {
        return serverWorld->worldTemplate()->store();
      });
      callbacks.registerCallback("setTemplate", [serverWorld](Json worldTemplate) {
        auto newTemplate = std::make_shared<WorldTemplate>(serverWorld->root(), worldTemplate);
        serverWorld->setTemplate(newTemplate);
      });
    }

    return callbacks;
  }

  void addWorldDebugCallbacks(Root& root, LuaCallbacks& callbacks) {
    callbacks.registerCallbackWithSignature<void, Vec2F, Color>("debugPoint", std::bind(WorldDebugCallbacks::debugPoint, std::ref(root), std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<void, Vec2F, Vec2F, Color>("debugLine", std::bind(WorldDebugCallbacks::debugLine, std::ref(root), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<void, PolyF, Color>("debugPoly", std::bind(WorldDebugCallbacks::debugPoly, std::ref(root), std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<void, LuaEngine&, LuaVariadic<LuaValue>>("debugText", std::bind(WorldDebugCallbacks::debugText, std::ref(root), std::placeholders::_1, std::placeholders::_2));
  }

  void addWorldEntityCallbacks(LuaCallbacks& callbacks, World* world) {
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("entityQuery", std::bind(WorldEntityCallbacks::entityQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("monsterQuery", std::bind(WorldEntityCallbacks::monsterQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("npcQuery", std::bind(WorldEntityCallbacks::npcQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("objectQuery", std::bind(WorldEntityCallbacks::objectQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("itemDropQuery", std::bind(WorldEntityCallbacks::itemDropQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("playerQuery", std::bind(WorldEntityCallbacks::playerQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, LuaValue, std::optional<LuaTable>>("loungeableQuery", std::bind(WorldEntityCallbacks::loungeableQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, Vec2F, std::optional<LuaTable>>("entityLineQuery", std::bind(WorldEntityCallbacks::entityLineQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, Vec2F, std::optional<LuaTable>>("objectLineQuery", std::bind(WorldEntityCallbacks::objectLineQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&, Vec2F, Vec2F, std::optional<LuaTable>>("npcLineQuery", std::bind(WorldEntityCallbacks::npcLineQuery, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallback("objectAt",
        [world](Vec2I const& tilePosition) -> std::optional<int> {
          if (auto object = world->findEntityAtTile(tilePosition, [](std::shared_ptr<TileEntity> const& entity) { return is<Object>(entity); }))
            return object->entityId();
          else
            return {};
        });

    callbacks.registerCallbackWithSignature<std::optional<std::shared_ptr<Entity>>, EntityId>("entity", [world](EntityId entityId) -> std::optional<std::shared_ptr<Entity>> {
      if (auto entity = world->entity(entityId)) {
        return entity;
      } else {
        return {};
      }
    });
    
    callbacks.registerCallback("entities", [world](LuaEngine& engine, std::optional<LuaTable> options) {
      if (!options) {
        options = engine.createTable();
      }
      return entityQueryImpl<Entity>(world, engine, *options, {});
    });

    callbacks.registerCallbackWithSignature<bool, int>("entityExists", std::bind(WorldEntityCallbacks::entityExists, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, int, int>("entityCanDamage", std::bind(WorldEntityCallbacks::entityCanDamage, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId>("entityDamageTeam", std::bind(WorldEntityCallbacks::entityDamageTeam, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<Json, EntityId>("entityAggressive", std::bind(WorldEntityCallbacks::entityAggressive, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<LuaString>, LuaEngine&, int>("entityType", std::bind(WorldEntityCallbacks::entityType, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<Vec2F>, int>("entityPosition", std::bind(WorldEntityCallbacks::entityPosition, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<Vec2F>, int>("entityVelocity", std::bind(WorldEntityCallbacks::entityVelocity, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<RectF>, int>("entityMetaBoundBox", std::bind(WorldEntityCallbacks::entityMetaBoundBox, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<std::uint64_t>, EntityId, String>("entityCurrency", std::bind(WorldEntityCallbacks::entityCurrency, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<std::uint64_t>, EntityId, Json, std::optional<bool>>("entityHasCountOfItem", std::bind(WorldEntityCallbacks::entityHasCountOfItem, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<Vec2F>, EntityId>("entityHealth", std::bind(WorldEntityCallbacks::entityHealth, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("entitySpecies", std::bind(WorldEntityCallbacks::entitySpecies, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("entityGender", std::bind(WorldEntityCallbacks::entityGender, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("entityName", std::bind(WorldEntityCallbacks::entityName, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<Json>, EntityId>("entityNametag", std::bind(WorldEntityCallbacks::entityNametag, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId, std::optional<String>>("entityDescription", std::bind(WorldEntityCallbacks::entityDescription, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<LuaNullTermWrapper<std::optional<List<Drawable>>>, EntityId, String>("entityPortrait", std::bind(WorldEntityCallbacks::entityPortrait, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId, String>("entityHandItem", std::bind(WorldEntityCallbacks::entityHandItem, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId, String>("entityHandItemDescriptor", std::bind(WorldEntityCallbacks::entityHandItemDescriptor, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<LuaNullTermWrapper<std::optional<String>>, EntityId>("entityUniqueId", std::bind(WorldEntityCallbacks::entityUniqueId, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<Json, EntityId, String, std::optional<Json>>("getObjectParameter", std::bind(WorldEntityCallbacks::getObjectParameter, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<Json, EntityId, String, std::optional<Json>>("getNpcScriptParameter", std::bind(WorldEntityCallbacks::getNpcScriptParameter, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<List<Vec2I>, EntityId>("objectSpaces", std::bind(WorldEntityCallbacks::objectSpaces, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<int>, EntityId>("farmableStage", std::bind(WorldEntityCallbacks::farmableStage, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<int>, EntityId>("containerSize", std::bind(WorldEntityCallbacks::containerSize, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, EntityId>("containerClose", std::bind(WorldEntityCallbacks::containerClose, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, EntityId>("containerOpen", std::bind(WorldEntityCallbacks::containerOpen, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<Json, EntityId>("containerItems", std::bind(WorldEntityCallbacks::containerItems, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<Json, EntityId, std::size_t>("containerItemAt", std::bind(WorldEntityCallbacks::containerItemAt, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<bool>, EntityId, Json>("containerConsume", std::bind(WorldEntityCallbacks::containerConsume, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<bool>, EntityId, std::size_t, int>("containerConsumeAt", std::bind(WorldEntityCallbacks::containerConsumeAt, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<std::size_t>, EntityId, Json>("containerAvailable", std::bind(WorldEntityCallbacks::containerAvailable, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId>("containerTakeAll", std::bind(WorldEntityCallbacks::containerTakeAll, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<Json, EntityId, std::size_t>("containerTakeAt", std::bind(WorldEntityCallbacks::containerTakeAt, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId, std::size_t, int>("containerTakeNumItemsAt", std::bind(WorldEntityCallbacks::containerTakeNumItemsAt, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<std::size_t>, EntityId, Json>("containerItemsCanFit", std::bind(WorldEntityCallbacks::containerItemsCanFit, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId, Json>("containerItemsFitWhere", std::bind(WorldEntityCallbacks::containerItemsFitWhere, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId, Json>("containerAddItems", std::bind(WorldEntityCallbacks::containerAddItems, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId, Json>("containerStackItems", std::bind(WorldEntityCallbacks::containerStackItems, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Json, EntityId, Json, std::size_t>("containerPutItemsAt", std::bind(WorldEntityCallbacks::containerPutItemsAt, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<Json, EntityId, Json, std::size_t>("containerSwapItems", std::bind(WorldEntityCallbacks::containerSwapItems, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<Json, EntityId, Json, std::size_t>("containerSwapItemsNoCombine", std::bind(WorldEntityCallbacks::containerSwapItemsNoCombine, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<Json, EntityId, Json, std::size_t>("containerItemApply", std::bind(WorldEntityCallbacks::containerItemApply, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<LuaValue>, EntityId, String, LuaVariadic<LuaValue>>("callScriptedEntity", std::bind(WorldEntityCallbacks::callScriptedEntity, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<RpcPromise<Vec2F>, String>("findUniqueEntity", std::bind(WorldEntityCallbacks::findUniqueEntity, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<RpcPromise<Json>, LuaEngine&, LuaValue, String, LuaVariadic<Json>>("sendEntityMessage", std::bind(WorldEntityCallbacks::sendEntityMessage, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<std::optional<List<EntityId>>, EntityId, std::optional<std::size_t>>("loungingEntities", std::bind(WorldEntityCallbacks::loungingEntities, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<bool>, EntityId, std::optional<std::size_t>>("loungeableOccupied", std::bind(WorldEntityCallbacks::loungeableOccupied, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<std::size_t>, EntityId>("loungeableAnchorCount", std::bind(WorldEntityCallbacks::loungeableAnchorCount, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, EntityId, std::optional<bool>>("isMonster", std::bind(WorldEntityCallbacks::isMonster, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("monsterType", std::bind(WorldEntityCallbacks::monsterType, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("npcType", std::bind(WorldEntityCallbacks::npcType, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<std::optional<String>, EntityId>("stagehandType", std::bind(WorldEntityCallbacks::stagehandType, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, EntityId, std::optional<int>>("isNpc", std::bind(WorldEntityCallbacks::isNpc, world, std::placeholders::_1, std::placeholders::_2));
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
      callbacks.registerCallback("itemDropItem", [world](EntityId const& entityId) -> Json {
          if (auto itemDrop = world->get<ItemDrop>(entityId))
            return itemDrop->item()->descriptor().toJson();
          return {};
        });
  }

  void addWorldEnvironmentCallbacks(LuaCallbacks& callbacks, World* world) {
    callbacks.registerCallbackWithSignature<float, Vec2F>("lightLevel", std::bind(WorldEnvironmentCallbacks::lightLevel, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<float, Vec2F>("windLevel", std::bind(WorldEnvironmentCallbacks::windLevel, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, Vec2F>("breathable", std::bind(WorldEnvironmentCallbacks::breathable, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, Vec2F>("underground", std::bind(WorldEnvironmentCallbacks::underground, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, Vec2F, String>("material", std::bind(WorldEnvironmentCallbacks::material, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, Vec2F, String>("mod", std::bind(WorldEnvironmentCallbacks::mod, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<float, Vec2F, String>("materialHueShift", std::bind(WorldEnvironmentCallbacks::materialHueShift, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<float, Vec2F, String>("modHueShift", std::bind(WorldEnvironmentCallbacks::modHueShift, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<MaterialColorVariant, Vec2F, String>("materialColor", std::bind(WorldEnvironmentCallbacks::materialColor, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<void, Vec2F, String, MaterialColorVariant>("setMaterialColor", std::bind(WorldEnvironmentCallbacks::setMaterialColor, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    callbacks.registerCallback("oceanLevel", [world](Vec2I position) -> int {
        if (auto serverWorld = as<WorldServer>(world)) {
          return serverWorld->worldTemplate()->blockInfo(position[0], position[1]).oceanLiquidLevel;
        } else {
          auto clientWorld = as<WorldClient>(world);
          return clientWorld->currentTemplate()->blockInfo(position[0], position[1]).oceanLiquidLevel;
        }
      });

    callbacks.registerCallback("environmentStatusEffects", [world](Vec2F const& position) {
        return world->environmentStatusEffects(position);
      });
    callbacks.registerCallback("weatherStatusEffects", [world](Vec2F const& position) {
        return world->weatherStatusEffects(position);
      });
    callbacks.registerCallback("exposedToWeather", [world](Vec2F const& position) {
        return world->exposedToWeather(position);
      });
    callbacks.registerCallback("activeWeather", [world](Vec2F const& position) {
        return world->activeWeather(position);
      });

    callbacks.registerCallbackWithSignature<bool, List<Vec2I>, String, Vec2F, String, float, std::optional<unsigned>, std::optional<EntityId>>("damageTiles", std::bind(WorldEnvironmentCallbacks::damageTiles, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7));
    callbacks.registerCallbackWithSignature<bool, Vec2F, float, String, Vec2F, String, float, std::optional<unsigned>, std::optional<EntityId>>("damageTileArea", std::bind(WorldEnvironmentCallbacks::damageTileArea, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8));
    callbacks.registerCallbackWithSignature<bool, Vec2I, String, String, std::optional<int>, bool>("placeMaterial", std::bind(WorldEnvironmentCallbacks::placeMaterial, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    callbacks.registerCallbackWithSignature<bool, List<Vec2I>, String, String, std::optional<int>, bool>("replaceMaterials", std::bind(WorldEnvironmentCallbacks::replaceMaterials, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    callbacks.registerCallbackWithSignature<bool, Vec2F, float, String, String, std::optional<int>, bool>("replaceMaterialArea", std::bind(WorldEnvironmentCallbacks::replaceMaterialArea, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    callbacks.registerCallbackWithSignature<bool, Vec2I, String, String, std::optional<int>, bool>("placeMod", std::bind(WorldEnvironmentCallbacks::placeMod, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    callbacks.registerCallback("radialTileQuery", [world](Vec2F center, float radius, String layerName) -> List<Vec2I> {
      auto layer = TileLayerNames.getLeft(layerName);
      return tileAreaBrush(radius, center, false).filtered([&](Vec2I const& t) -> bool {
        return world->material(t, layer) != EmptyMaterialId;
      });
    });
    
    callbacks.registerCallbackWithSignature<List<std::pair<Vec2I, LiquidLevel>>, Vec2F, Vec2F>("liquidAlongLine", std::bind(WorldEnvironmentCallbacks::liquidAlongLine, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<bool, Vec2I, std::optional<bool>, std::optional<bool>>("tileIsOccupied", std::bind(WorldEnvironmentCallbacks::tileIsOccupied, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<float>("threatLevel", std::bind(&World::threatLevel, world));
    callbacks.registerCallbackWithSignature<double>("time", std::bind(WorldEnvironmentCallbacks::time, world));
    callbacks.registerCallbackWithSignature<std::uint64_t>("day", std::bind(WorldEnvironmentCallbacks::day, world));
    callbacks.registerCallbackWithSignature<double>("timeOfDay", std::bind(WorldEnvironmentCallbacks::timeOfDay, world));
    callbacks.registerCallbackWithSignature<float>("dayLength", std::bind(WorldEnvironmentCallbacks::dayLength, world));
    callbacks.registerCallbackWithSignature<std::optional<LiquidLevel>, std::variant<RectF, Vec2I>>("liquidAt", std::bind(WorldEnvironmentCallbacks::liquidAt, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<float, Vec2F>("gravity", std::bind(WorldEnvironmentCallbacks::gravity, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, Vec2F, LiquidId, float>("spawnLiquid", std::bind(WorldEnvironmentCallbacks::spawnLiquid, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<LiquidLevel>, Vec2F>("destroyLiquid", std::bind(WorldEnvironmentCallbacks::destroyLiquid, world, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, Vec2F>("isTileProtected", std::bind(WorldEnvironmentCallbacks::isTileProtected, world, std::placeholders::_1));
  }
  
  void addWorldGeometryCallbacks(LuaCallbacks& callbacks, World* world) {

    callbacks.registerCallback("size", [world]() -> Vec2U {
        return world->geometry().size();
      });
    callbacks.registerCallbackWithSignature<float, Vec2F, std::optional<Vec2F>>("magnitude", std::bind(&WorldGeometryCallbacks::magnitude, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<Vec2F, Vec2F, Vec2F>("distance", std::bind(WorldGeometryCallbacks::distance, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<bool, PolyF, Vec2F>("polyContains", std::bind(WorldGeometryCallbacks::polyContains, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, LuaValue>("xwrap", std::bind(WorldGeometryCallbacks::xwrap, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<LuaValue, LuaEngine&, std::variant<Vec2F, float>, std::variant<Vec2F, float>>("nearestTo", std::bind(WorldGeometryCallbacks::nearestTo, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }
  
  void addWorldCollisionCallbacks(LuaCallbacks& callbacks, World* world) {
    callbacks.registerCallbackWithSignature<bool, RectF, std::optional<CollisionSet>>("rectCollision", std::bind(WorldCollisionCallbacks::rectCollision, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<bool, Vec2F, std::optional<CollisionSet>>("pointTileCollision", std::bind(WorldCollisionCallbacks::pointTileCollision, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<bool, Vec2F, Vec2F, std::optional<CollisionSet>>("lineTileCollision", std::bind(WorldCollisionCallbacks::lineTileCollision, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<std::optional<std::pair<Vec2F, Vec2I>>, Vec2F, Vec2F, std::optional<CollisionSet>>("lineTileCollisionPoint", std::bind(WorldCollisionCallbacks::lineTileCollisionPoint, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<bool, RectF, std::optional<CollisionSet>>("rectTileCollision", std::bind(WorldCollisionCallbacks::rectTileCollision, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<bool, Vec2F, std::optional<CollisionSet>>("pointCollision", std::bind(WorldCollisionCallbacks::pointCollision, world, std::placeholders::_1, std::placeholders::_2));
    callbacks.registerCallbackWithSignature<LuaTupleReturn<std::optional<Vec2F>, std::optional<Vec2F>>, Vec2F, Vec2F, std::optional<CollisionSet>>("lineCollision", std::bind(WorldCollisionCallbacks::lineCollision, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<bool, PolyF, std::optional<Vec2F>, std::optional<CollisionSet>>("polyCollision", std::bind(WorldCollisionCallbacks::polyCollision, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    callbacks.registerCallbackWithSignature<List<Vec2I>, Vec2F, Vec2F, std::optional<CollisionSet>, std::optional<int>>("collisionBlocksAlongLine", std::bind(WorldCollisionCallbacks::collisionBlocksAlongLine, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    callbacks.registerCallbackWithSignature<std::optional<Vec2F>, PolyF, Vec2F, float, std::optional<CollisionSet>>("resolvePolyCollision", std::bind(WorldCollisionCallbacks::resolvePolyCollision, world, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  }

  float WorldGeometryCallbacks::magnitude(World* world, Vec2F pos1, std::optional<Vec2F> pos2) {
    if (pos2)
      return world->geometry().diff(pos1, *pos2).magnitude();
    else
      return pos1.magnitude();
  }

  Vec2F WorldGeometryCallbacks::distance(World* world, Vec2F const& arg1, Vec2F const& arg2) {
    return world->geometry().diff(arg1, arg2);
  }

  bool WorldGeometryCallbacks::polyContains(World* world, PolyF const& poly, Vec2F const& pos) {
    return world->geometry().polyContains(poly, pos);
  }

  LuaValue WorldGeometryCallbacks::xwrap(World* world, LuaEngine& engine, LuaValue const& positionOrX) {
    if (auto x = engine.luaMaybeTo<float>(positionOrX))
      return LuaFloat(world->geometry().xwrap(*x));
    return engine.luaFrom<Vec2F>(world->geometry().xwrap(engine.luaTo<Vec2F>(positionOrX)));
  }

  LuaValue WorldGeometryCallbacks::nearestTo(World* world, LuaEngine& engine, std::variant<Vec2F, float> const& sourcePositionOrX, std::variant<Vec2F, float> const& targetPositionOrX) {
    if (std::holds_alternative<Vec2F>(targetPositionOrX)) {
      Vec2F targetPosition = std::get<Vec2F>(targetPositionOrX);
      Vec2F sourcePosition;
      if (std::holds_alternative<Vec2F>(sourcePositionOrX))
        sourcePosition = std::get<Vec2F>(sourcePositionOrX);
      else
        sourcePosition[0] = std::get<float>(sourcePositionOrX);

      return engine.luaFrom<Vec2F>(world->geometry().nearestTo(sourcePosition, targetPosition));

    } else {
      float targetX = std::get<float>(targetPositionOrX);
      float sourceX;
      if (std::holds_alternative<Vec2F>(sourcePositionOrX))
        sourceX = std::get<Vec2F>(sourcePositionOrX)[0];
      else
        sourceX = std::get<float>(sourcePositionOrX);

      return LuaFloat(world->geometry().nearestTo(sourceX, targetX));
    }
  }

  bool WorldCollisionCallbacks::rectCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2) {
    PolyF body = PolyF(arg1);

    if (arg2)
      return world->polyCollision(body, *arg2);
    else
      return world->polyCollision(body);
  }

  bool WorldCollisionCallbacks::pointTileCollision(World* world, Vec2F const& arg1, std::optional<CollisionSet> const& arg2) {
    if (arg2)
      return world->pointTileCollision(arg1, *arg2);
    else
      return world->pointTileCollision(arg1);
  }

  bool WorldCollisionCallbacks::lineTileCollision(
      World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3) {
    Vec2F const begin = arg1;
    Vec2F const end = arg2;

    if (arg3)
      return world->lineTileCollision(begin, end, *arg3);
    else
      return world->lineTileCollision(begin, end);
  }

  std::optional<std::pair<Vec2F, Vec2I>> WorldCollisionCallbacks::lineTileCollisionPoint(World* world, Vec2F const& begin, Vec2F const& end, std::optional<CollisionSet> const& collisionSet) {
    if (collisionSet)
      return world->lineTileCollisionPoint(begin, end, *collisionSet);
    else
      return world->lineTileCollisionPoint(begin, end);
  }

  bool WorldCollisionCallbacks::rectTileCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2) {
    RectI const region = RectI::integral(arg1);

    if (arg2)
      return world->rectTileCollision(region, *arg2);
    else
      return world->rectTileCollision(region);
  }

  bool WorldCollisionCallbacks::pointCollision(World* world, Vec2F const& point, std::optional<CollisionSet> const& collisionSet) {
    return world->pointCollision(point, collisionSet.value_or(DefaultCollisionSet));
  }

  LuaTupleReturn<std::optional<Vec2F>, std::optional<Vec2F>> WorldCollisionCallbacks::lineCollision(World* world, Vec2F const& start, Vec2F const& end, std::optional<CollisionSet> const& collisionSet) {
    std::optional<Vec2F> point;
    std::optional<Vec2F> normal;
    auto collision = world->lineCollision(Line2F(start, end), collisionSet.value_or(DefaultCollisionSet));
    if (collision) {
      point = collision->first;
      normal = collision->second;
    }
    return luaTupleReturn(point, normal);
  }

  bool WorldCollisionCallbacks::polyCollision(
      World* world, PolyF const& arg1, std::optional<Vec2F> const& arg2, std::optional<CollisionSet> const& arg3) {
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

  List<Vec2I> WorldCollisionCallbacks::collisionBlocksAlongLine(
      World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3, std::optional<int> const& arg4) {
    Vec2F const begin = arg1;
    Vec2F const end = arg2;

    CollisionSet collisionSet = arg3.value_or(DefaultCollisionSet);
    int const maxSize = arg4 ? *arg4 : -1;
    return world->collidingTilesAlongLine(begin, end, collisionSet, maxSize);
  }

  std::optional<Vec2F> WorldCollisionCallbacks::resolvePolyCollision(
      World* world, PolyF poly, Vec2F const& position, float maximumCorrection, std::optional<CollisionSet> const& maybeCollisionSet) {
    struct CollisionPoly {
      PolyF poly;
      Vec2F center;
      float sortingDistance;
    };

    poly.translate(position);
    List<CollisionPoly> collisions;
    CollisionSet collisionSet = maybeCollisionSet.value_or(DefaultCollisionSet);
    world->forEachCollisionBlock(RectI::integral(poly.boundBox().padded(maximumCorrection + 1.0f)),
        [&](auto const& block) {
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
        std::sort(collisions.begin(), collisions.end(), [](auto const& a, auto const& b) { return a.sortingDistance < b.sortingDistance; });

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
      float angle = i * std::numbers::pi / 8;
      Vec2F dir = Vec2F::withAngle(angle, 1.0f);
      if (auto resolution = resolveCollision(dir, maximumCorrection, 1))
        return position + *resolution;
    }

    return {};
  }

  List<std::pair<Vec2I, LiquidLevel>> WorldEnvironmentCallbacks::liquidAlongLine(World* world, Vec2F const& start, Vec2F const& end) {
    List<std::pair<Vec2I, LiquidLevel>> levels;
    forBlocksAlongLine<float>(start, world->geometry().diff(end, start), [&](int x, int y) {
        auto liquidLevel = world->liquidLevel(RectF::withSize(Vec2F(x, y), Vec2F(1, 1)));
        if (liquidLevel.liquid != EmptyLiquidId)
          levels.append(std::pair<Vec2I, LiquidLevel>(Vec2I(x, y), liquidLevel));
        return true;
      });
    return levels;
  }

  bool WorldEnvironmentCallbacks::tileIsOccupied(
      World* world, Vec2I const& arg1, std::optional<bool> const& arg2, std::optional<bool> const& arg3) {
    Vec2I const tile = arg1;
    bool const tileLayerBool = arg2.value_or(true);
    bool const includeEphemeral = arg3.value_or(false);

    TileLayer const tileLayer = tileLayerBool ? TileLayer::Foreground : TileLayer::Background;

    return world->tileIsOccupied(tile, tileLayer, includeEphemeral);
  }

  bool WorldCallbacks::placeObject(World* world,
      String const& objectType,
      Vec2I const& worldPosition,
      std::optional<int> const& objectDirection,
      Json const& objectParameters) {
    auto objectDatabase = world->root().objectDatabase();

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
      world->root().logger().warn("Could not create placable object of kind '{}', exception caught: {}",
          objectType,
          outputException(exception, false));
    }

    return false;
  }

  std::optional<EntityId> WorldCallbacks::spawnItem(World* world,
      Json const& itemType,
      Vec2F const& worldPosition,
      std::optional<std::size_t> const& inputCount,
      Json const& inputParameters,
      std::optional<Vec2F> const& initialVelocity,
      std::optional<float> const& intangibleTime) {
    Vec2F const position = worldPosition;

    try {
      ItemDescriptor descriptor;
      if (itemType.isType(Json::Type::String)) {
        std::size_t count = inputCount.value_or(1);

        Json parameters = inputParameters ? inputParameters : JsonObject();

        descriptor = ItemDescriptor(itemType.toString(), count, parameters);
      } else {
        descriptor = ItemDescriptor(itemType);
      }

      if (auto itemDrop = ItemDrop::createRandomizedDrop(world->root(), descriptor, position)) {
        if (initialVelocity)
          itemDrop->setVelocity(*initialVelocity);
        if (intangibleTime)
          itemDrop->setIntangibleTime(*intangibleTime);
        world->addEntity(itemDrop);
        return itemDrop->inWorld() ? itemDrop->entityId() : std::optional<EntityId>();
      }

      world->root().logger().warn("Could not spawn item, item empty in WorldCallbacks::spawnItem");
    } catch (StarException const& exception) {
      world->root().logger().warn("Could not spawn Item of kind '{}', exception caught: {}", itemType, outputException(exception, false));
    }

    return {};
  }

  List<EntityId> WorldCallbacks::spawnTreasure(
      World* world, Vec2F const& position, String const& pool, float level, std::optional<std::uint64_t> seed) {
    List<EntityId> entities;
    auto treasureDatabase = world->root().treasureDatabase();
    try {
      for (auto const& treasureItem : treasureDatabase->createTreasure(pool, level, seed.value_or(world->root().random().randu64()))) {
        std::shared_ptr<ItemDrop> entity = ItemDrop::createRandomizedDrop(world->root(), treasureItem, position);
        entities.append(entity->entityId());
        world->addEntity(entity);
      }
    } catch (StarException const& exception) {
      world->root().logger().warn(
          "Could not spawn treasure from pool '{}', exception caught: {}", pool, outputException(exception, false));
    }
    return entities;
  }

  std::optional<EntityId> WorldCallbacks::spawnMonster(
      World* world, String const& arg1, Vec2F const& arg2, std::optional<JsonObject> const& arg3) {
    Vec2F const spawnPosition = arg2;
    auto monsterDatabase = world->root().monsterDatabase();

    try {
      JsonObject parameters;
      parameters["aggressive"] = world->root().random().randb();
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
      world->root().logger().warn(
          "Could not spawn Monster of type '{}', exception caught: {}", arg1, outputException(exception, false));
      return {};
    }
  }

  std::optional<EntityId> WorldCallbacks::spawnNpc(World* world,
      Vec2F const& arg1,
      String const& arg2,
      String const& arg3,
      float arg4,
      std::optional<std::uint64_t> arg5,
      Json const& arg6) {
    Vec2F const spawnPosition = arg1;

    String typeName = arg3;
    float level = arg4;

    std::uint64_t seed;
    if (arg5)
      seed = *arg5;
    else
      seed = world->root().random().randu64();

    Json overrides = arg6 ? arg6 : JsonObject();

    auto npcDatabase = world->root().npcDatabase();
    try {
      auto npc = npcDatabase->createNpc(npcDatabase->generateNpcVariant(arg2, typeName, level, seed, overrides));
      npc->setPosition(spawnPosition);
      world->addEntity(npc);
      return npc->inWorld() ? npc->entityId() : std::optional<EntityId>();
    } catch (StarException const& exception) {
      world->root().logger().warn("Could not spawn NPC of species '{}' and type '{}', exception caught: {}",
          arg2,
          typeName,
          outputException(exception, false));
      return {};
    }
  }

  std::optional<EntityId> WorldCallbacks::spawnStagehand(
      World* world, Vec2F const& spawnPosition, String const& typeName, Json const& overrides) {
    auto stagehandDatabase = world->root().stagehandDatabase();
    try {
      auto stagehand = stagehandDatabase->createStagehand(typeName, overrides);
      stagehand->setPosition(spawnPosition);
      world->addEntity(stagehand);
      return stagehand->inWorld() ? stagehand->entityId() : std::optional<EntityId>();
    } catch (StarException const& exception) {
      world->root().logger().warn(
          "Could not spawn Stagehand of type '{}', exception caught: {}", typeName, outputException(exception, false));
      return {};
    }
  }

  std::optional<EntityId> WorldCallbacks::spawnProjectile(World* world,
      String const& projectileType,
      Vec2F const& spawnPosition,
      std::optional<EntityId> const& sourceEntityId,
      std::optional<Vec2F> const& projectileDirection,
      bool trackSourceEntity,
      Json const& projectileParameters) {

    try {
      auto projectileDatabase = world->root().projectileDatabase();
      auto projectile = projectileDatabase->createProjectile(projectileType, projectileParameters ? projectileParameters : JsonObject());
      projectile->setInitialPosition(spawnPosition);
      projectile->setInitialDirection(projectileDirection.value_or(Vec2F{}));
      projectile->setSourceEntity(sourceEntityId.value_or(NullEntityId), trackSourceEntity);
      world->addEntity(projectile);
      return projectile->inWorld() ? projectile->entityId() : std::optional<EntityId>();
    } catch (StarException const& exception) {
      world->root().logger().warn(
          "Could not spawn Projectile of type '{}', exception caught: {}", projectileType, outputException(exception, false));
      return {};
    }
  }

  std::optional<EntityId> WorldCallbacks::spawnVehicle(
      World* world, String const& vehicleName, Vec2F const& pos, Json const& extraConfig) {
    auto vehicleDatabase = world->root().vehicleDatabase();
    auto vehicle = vehicleDatabase->create(vehicleName, extraConfig);
    vehicle->setPosition(pos);
    world->addEntity(vehicle);
    if (vehicle->inWorld())
      return vehicle->entityId();
    return {};
  }

  double WorldEnvironmentCallbacks::time(World* world) {
    return world->epochTime();
  }

  std::uint64_t WorldEnvironmentCallbacks::day(World* world) {
    return world->day();
  }

  double WorldEnvironmentCallbacks::timeOfDay(World* world) {
    return world->timeOfDay() / world->dayLength();
  }

  float WorldEnvironmentCallbacks::dayLength(World* world) {
    return world->dayLength();
  }

  Json WorldCallbacks::getProperty(World* world, String const& arg1, Json const& arg2) {
    return world->getProperty(arg1, arg2);
  }

  void WorldCallbacks::setProperty(World* world, String const& arg1, Json const& arg2) {
    world->setProperty(arg1, arg2);
  }

  std::optional<LiquidLevel> WorldEnvironmentCallbacks::liquidAt(World* world, std::variant<RectF, Vec2I> boundBoxOrPoint) {
    LiquidLevel liquidLevel = std::visit([world](auto const& bbop) { return world->liquidLevel(bbop); }, boundBoxOrPoint);
    if (liquidLevel.liquid != EmptyLiquidId)
      return liquidLevel;
    return {};
  }

  float WorldEnvironmentCallbacks::gravity(World* world, Vec2F const& arg1) {
    return world->gravity(arg1);
  }

  bool WorldEnvironmentCallbacks::spawnLiquid(World* world, Vec2F const& position, LiquidId liquid, float quantity) {
    return world->modifyTile(Vec2I::floor(position), PlaceLiquid{liquid, quantity}, true);
  }

  std::optional<LiquidLevel> WorldEnvironmentCallbacks::destroyLiquid(World* world, Vec2F const& position) {
    auto liquidLevel = world->liquidLevel(Vec2I::floor(position));
    if (liquidLevel.liquid != EmptyLiquidId) {
      if (world->modifyTile(Vec2I::floor(position), PlaceLiquid{EmptyLiquidId, 0}, true))
        return liquidLevel;
    }
    return {};
  }

  bool WorldEnvironmentCallbacks::isTileProtected(World* world, Vec2F const& position) {
    return world->isTileProtected(Vec2I::floor(position));
  }

  std::optional<PlatformerAStar::Path> WorldCallbacks::findPlatformerPath(World* world,
      Vec2F const& start,
      Vec2F const& end,
      ActorMovementParameters actorMovementParameters,
      PlatformerAStar::Parameters searchParameters) {
    PlatformerAStar::PathFinder pathFinder(world, start, end, std::move(actorMovementParameters), std::move(searchParameters));
    pathFinder.explore({});
    return pathFinder.result();
  }

  PlatformerAStar::PathFinder WorldCallbacks::platformerPathStart(World* world,
      Vec2F const& start,
      Vec2F const& end,
      ActorMovementParameters actorMovementParameters,
      PlatformerAStar::Parameters searchParameters) {
    return PlatformerAStar::PathFinder(world, start, end, std::move(actorMovementParameters), std::move(searchParameters));
  }

  void ClientWorldCallbacks::resendEntity(WorldClient* world, EntityId arg1) {
    return world->resendEntity(arg1);
  }

  RectI ClientWorldCallbacks::clientWindow(WorldClient* world) {
    return world->clientWindow();
  }

  String ServerWorldCallbacks::id(WorldServer* world) {
    return world->worldId();
  }

  bool ServerWorldCallbacks::breakObject(WorldServer* world, EntityId arg1, bool arg2) {
    if (auto entity = world->get<Object>(arg1)) {
      bool smash = arg2;
      entity->breakObject(smash);
      return true;
    }
    return false;
  }

  bool ServerWorldCallbacks::isVisibleToPlayer(WorldServer* world, RectF const& arg1) {
    return world->isVisibleToPlayer(arg1);
  }

  bool ServerWorldCallbacks::loadRegion(WorldServer* world, RectF const& arg1) {
    return world->signalRegion(RectI::integral(arg1));
  }

  bool ServerWorldCallbacks::regionActive(WorldServer* world, RectF const& arg1) {
    return world->regionActive(RectI::integral(arg1));
  }

  void ServerWorldCallbacks::setTileProtection(WorldServer* world, DungeonId arg1, bool arg2) {
    DungeonId dungeonId = arg1;
    bool isProtected = arg2;
    world->setTileProtection(dungeonId, isProtected);
  }

  bool ServerWorldCallbacks::isPlayerModified(WorldServer* world, RectI const& region) {
    return world->isPlayerModified(region);
  }

  std::optional<LiquidLevel> ServerWorldCallbacks::forceDestroyLiquid(WorldServer* world, Vec2F const& position) {
    auto liquidLevel = world->liquidLevel(Vec2I::floor(position));
    if (liquidLevel.liquid != EmptyLiquidId) {
      if (world->forceModifyTile(Vec2I::floor(position), PlaceLiquid{EmptyLiquidId, 0}, true))
        return liquidLevel;
    }
    return {};
  }

  EntityId ServerWorldCallbacks::loadUniqueEntity(WorldServer* world, String const& uniqueId) {
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

  Json ServerWorldCallbacks::takeItemDrop(World* world, EntityId entityId, std::optional<EntityId> const& takenBy) {
    auto itemDrop = world->get<ItemDrop>(entityId);
    if (itemDrop && itemDrop->canTake() && itemDrop->isMaster()) {
      std::shared_ptr<Item> item;
      if (takenBy)
        item = itemDrop->takeBy(*takenBy);
      else
        item = itemDrop->take();

      if (item)
        return item->descriptor().toJson();
    }

    return Json();
  }

  void ServerWorldCallbacks::setPlayerStart(World* world, Vec2F const& playerStart, std::optional<bool> respawnInWorld) {
    as<WorldServer>(world)->setPlayerStart(playerStart, respawnInWorld.has_value() && respawnInWorld.value());
  }

  List<EntityId> ServerWorldCallbacks::players(World* world) {
    return as<WorldServer>(world)->players();
  }

  LuaString ServerWorldCallbacks::fidelity(World* world, LuaEngine& engine) {
    return engine.createString(WorldServerFidelityNames.getRight(as<WorldServer>(world)->fidelity()));
  }

  std::optional<LuaValue> ServerWorldCallbacks::callScriptContext(World* world, String const& contextName, String const& function, LuaVariadic<LuaValue> const& args) {
    auto context = as<WorldServer>(world)->scriptContext(contextName);
    if (!context)
      throw StarException::format("Context {} does not exist", contextName);
    return context->invoke(function, args);
  }

  bool ServerWorldCallbacks::sendPacket(WorldServer* world, ConnectionId clientId, String const& packetType, Json const& packetData) {
    PacketType type = PacketTypeNames.getLeft(packetType);
    auto packet = createPacket(world->root().random(), type, packetData);
    return world->sendPacket(clientId, packet);
  }

  void WorldDebugCallbacks::debugPoint(Root& root, Vec2F const& arg1, Color const& arg2) {
    root.spatialLogger().logPoint("world", arg1, arg2.toRgba());
  }

  void WorldDebugCallbacks::debugLine(Root& root, Vec2F const& arg1, Vec2F const& arg2, Color const& arg3) {
    root.spatialLogger().logLine("world", arg1, arg2, arg3.toRgba());
  }

  void WorldDebugCallbacks::debugPoly(Root& root, PolyF const& poly, Color const& color) {
    root.spatialLogger().logPoly("world", poly, color.toRgba());
  }

  void WorldDebugCallbacks::debugText(Root& root, LuaEngine& engine, LuaVariadic<LuaValue> const& args) {
    if (args.size() < 3)
      throw StarException(strf("Too few arguments to debugText: {}", args.size()));

    Vec2F const position = engine.luaTo<Vec2F>(args.at(args.size() - 2));
    Vec4B const color = engine.luaTo<Color>(args.at(args.size() - 1)).toRgba();

    String text = formatLua(engine.luaTo<String>(args.at(0)), slice<List<LuaValue>>(args, 1, args.size() - 2));
    root.spatialLogger().logText("world", text, position, color);
  }

  LuaTable WorldEntityCallbacks::entityQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) {
    return LuaBindings::entityQuery<Entity>(world, engine, pos1, pos2, std::move(options));
  }

  LuaTable WorldEntityCallbacks::monsterQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) {
    return LuaBindings::entityQuery<Monster>(world, engine, pos1, pos2, std::move(options));
  }

  LuaTable WorldEntityCallbacks::npcQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) {
    return LuaBindings::entityQuery<Npc>(world, engine, pos1, pos2, std::move(options));
  }

  LuaTable WorldEntityCallbacks::objectQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) {
    String objectName;
    if (options)
      objectName = options->get<std::optional<String>>("name").value_or(String{});

    return LuaBindings::entityQuery<Object>(world,
        engine,
        pos1,
        pos2,
        std::move(options),
        [&objectName](std::shared_ptr<Object> const& entity) -> bool {
          return objectName.empty() || entity->name() == objectName;
        });
  }

  LuaTable WorldEntityCallbacks::itemDropQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) {
    return LuaBindings::entityQuery<ItemDrop>(world, engine, pos1, pos2, std::move(options));
  }

  LuaTable WorldEntityCallbacks::playerQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) {
    return LuaBindings::entityQuery<Player>(world, engine, pos1, pos2, std::move(options));
  }

  LuaTable WorldEntityCallbacks::loungeableQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) {
    String orientationName;
    if (options)
      orientationName = options->get<std::optional<String>>("orientation").value_or(String{});

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

  LuaTable WorldEntityCallbacks::entityLineQuery(World* world, LuaEngine& engine, Vec2F const& point1, Vec2F const& point2, std::optional<LuaTable> options) {
    return LuaBindings::entityLineQuery<Entity>(world, engine, point1, point2, std::move(options));
  }

  LuaTable WorldEntityCallbacks::objectLineQuery(World* world, LuaEngine& engine, Vec2F const& point1, Vec2F const& point2, std::optional<LuaTable> options) {
    return LuaBindings::entityLineQuery<Object>(world, engine, point1, point2, std::move(options));
  }

  LuaTable WorldEntityCallbacks::npcLineQuery(World* world, LuaEngine& engine, Vec2F const& point1, Vec2F const& point2, std::optional<LuaTable> options) {
    return LuaBindings::entityLineQuery<Npc>(world, engine, point1, point2, std::move(options));
  }

  bool WorldEntityCallbacks::entityExists(World* world, EntityId entityId) {
    return (bool)world->entity(entityId);
  }

  bool WorldEntityCallbacks::entityCanDamage(World* world, EntityId sourceId, EntityId targetId) {
    auto source = world->entity(sourceId);
    auto target = world->entity(targetId);

    if (!source || !target || !source->getTeam().canDamage(target->getTeam(), false))
      return false;

    return true;
  }

  Json WorldEntityCallbacks::entityDamageTeam(World* world, EntityId entityId) {
    if (auto entity = world->entity(entityId))
      return entity->getTeam().toJson();
    return {};
  }

  bool WorldEntityCallbacks::entityAggressive(World* world, EntityId entityId) {
    auto entity = world->entity(entityId);
    if (auto monster = as<Monster>(entity))
      return monster->aggressive();
    if (auto npc = as<Npc>(entity))
      return npc->aggressive();
    return false;
  }

  std::optional<LuaString> WorldEntityCallbacks::entityType(World* world, LuaEngine& engine, EntityId entityId) {
    if (auto entity = world->entity(entityId))
      return engine.createString(EntityTypeNames.getRight(entity->entityType()));
    return {};
  }

  std::optional<Vec2F> WorldEntityCallbacks::entityPosition(World* world, EntityId entityId) {
    if (auto entity = world->entity(entityId)) {
      return entity->position();
    } else {
      return {};
    }
  }

  std::optional<RectF> WorldEntityCallbacks::entityMetaBoundBox(World* world, EntityId entityId) {
    if (auto entity = world->entity(entityId)) {
      return entity->metaBoundBox();
    } else {
      return {};
    }
  }

  std::optional<Vec2F> WorldEntityCallbacks::entityVelocity(World* world, EntityId entityId) {
    auto entity = world->entity(entityId);

    if (auto mobileEntity = as<MobileEntity>(entity))
        return mobileEntity->movementController()->velocity();

    return {};
  }

  std::optional<std::uint64_t> WorldEntityCallbacks::entityCurrency(World* world, EntityId entityId, String const& currencyType) {
    if (auto player = world->get<Player>(entityId)) {
      return player->currency(currencyType);
    } else {
      return {};
    }
  }

  std::optional<std::uint64_t> WorldEntityCallbacks::entityHasCountOfItem(World* world, EntityId entityId, Json descriptor, std::optional<bool> exactMatch) {
    if (auto player = world->get<Player>(entityId)) {
      return player->inventory()->hasCountOfItem(ItemDescriptor(descriptor), exactMatch.value_or(false));
    } else {
      return {};
    }
  }

  std::optional<Vec2F> WorldEntityCallbacks::entityHealth(World* world, EntityId entityId) {
    if (auto entity = world->get<DamageBarEntity>(entityId)) {
      return Vec2F(entity->health(), entity->maxHealth());
    } else {
      return {};
    }
  }

  std::optional<String> WorldEntityCallbacks::entitySpecies(World* world, EntityId entityId) {
    if (auto player = world->get<Player>(entityId)) {
      return player->species();
    } else if (auto npc = world->get<Npc>(entityId)) {
      return npc->species();
    } else {
      return {};
    }
  }

  std::optional<String> WorldEntityCallbacks::entityGender(World* world, EntityId entityId) {
    if (auto player = world->get<Player>(entityId)) {
      return GenderNames.getRight(player->gender());
    } else if (auto npc = world->get<Npc>(entityId)) {
      return GenderNames.getRight(npc->gender());
    } else {
      return {};
    }
  }

  std::optional<String> WorldEntityCallbacks::entityName(World* world, EntityId entityId) {
    if (auto entity = world->entity(entityId))
      return entity->name();

    return {};
  }

  std::optional<Json> WorldEntityCallbacks::entityNametag(World* world, EntityId entityId) {
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
        result.set("status", *status);
    }

    return result;
  }

  std::optional<String> WorldEntityCallbacks::entityDescription(World* world, EntityId entityId, std::optional<String> const& species) {
    if (auto entity = world->entity(entityId)) {
      if (auto inspectableEntity = as<InspectableEntity>(entity)) {
        if (species)
          return inspectableEntity->inspectionDescription(*species);
      }

      return entity->description();
    }

    return {};
  }

  LuaNullTermWrapper<std::optional<List<Drawable>>> WorldEntityCallbacks::entityPortrait(World* world, EntityId entityId, String const& portraitMode) {
    if (auto portraitEntity = as<PortraitEntity>(world->entity(entityId)))
      return portraitEntity->portrait(PortraitModeNames.getLeft(portraitMode));

    return {};
  }

  std::optional<String> WorldEntityCallbacks::entityHandItem(World* world, EntityId entityId, String const& handName) {
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

    return {};
  }

  Json WorldEntityCallbacks::entityHandItemDescriptor(World* world, EntityId entityId, String const& handName) {
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

    return Json();
  }

  LuaNullTermWrapper<std::optional<String>> WorldEntityCallbacks::entityUniqueId(World* world, EntityId entityId) {
    if (auto entity = world->entity(entityId))
      return entity->uniqueId();
    return {};
  }

  Json WorldEntityCallbacks::getObjectParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue) {
    Json val = Json();

    if (auto objectEntity = as<Object>(world->entity(entityId))) {
      val = objectEntity->configValue(parameterName);
      if (!val && defaultValue)
        val = *defaultValue;
    }

    return val;
  }

  Json WorldEntityCallbacks::getNpcScriptParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue) {
    Json val = Json();

    if (auto npcEntity = as<Npc>(world->entity(entityId))) {
      val = npcEntity->scriptConfigParameter(parameterName);
      if (!val && defaultValue)
        val = *defaultValue;
    }

    return val;
  }

  List<Vec2I> WorldEntityCallbacks::objectSpaces(World* world, EntityId entityId) {
    if (auto tileEntity = as<TileEntity>(world->entity(entityId)))
      return tileEntity->spaces();
    return {};
  }

  std::optional<int> WorldEntityCallbacks::farmableStage(World* world, EntityId entityId) {
    if (auto farmable = world->get<FarmableObject>(entityId)) {
      return farmable->stage();
    }

    return {};
  }

  std::optional<int> WorldEntityCallbacks::containerSize(World* world, EntityId entityId) {
    if (auto container = world->get<ContainerObject>(entityId))
      return container->containerSize();

    return {};
  }

  bool WorldEntityCallbacks::containerClose(World* world, EntityId entityId) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      container->containerClose();
      return true;
    }

    return false;
  }

  bool WorldEntityCallbacks::containerOpen(World* world, EntityId entityId) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      container->containerOpen();
      return true;
    }

    return false;
  }

  Json WorldEntityCallbacks::containerItems(World* world, EntityId entityId) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      JsonArray res;
      auto itemDb = world->root().itemDatabase();
      for (auto const& item : container->itemBag()->items())
        res.append(itemDb->toJson(item));
      return res;
    }

    return Json();
  }

  Json WorldEntityCallbacks::containerItemAt(World* world, EntityId entityId, std::size_t offset) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto items = container->itemBag()->items();
      if (offset < items.size()) {
        return itemDb->toJson(items.at(offset));
      }
    }

    return Json();
  }

  std::optional<bool> WorldEntityCallbacks::containerConsume(World* world, EntityId entityId, Json const& items) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto toConsume = ItemDescriptor(items);
      return container->consumeItems(toConsume).result();
    }

    return {};
  }

  std::optional<bool> WorldEntityCallbacks::containerConsumeAt(World* world, EntityId entityId, std::size_t offset, int count) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      if (offset < container->containerSize()) {
        return container->consumeItems(offset, count).result();
      }
    }

    return {};
  }

  std::optional<std::size_t> WorldEntityCallbacks::containerAvailable(World* world, EntityId entityId, Json const& items) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemBag = container->itemBag();
      auto toCheck = ItemDescriptor(items);
      return itemBag->available(toCheck);
    }

    return {};
  }

  Json WorldEntityCallbacks::containerTakeAll(World* world, EntityId entityId) {
    auto itemDb = world->root().itemDatabase();
    if (auto container = world->get<ContainerObject>(entityId)) {
      if (auto itemList = container->clearContainer().result()) {
        JsonArray res;
        for (auto item : *itemList)
          res.append(itemDb->toJson(item));
        return res;
      }
    }

    return Json();
  }

  Json WorldEntityCallbacks::containerTakeAt(World* world, EntityId entityId, std::size_t offset) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      if (offset < container->containerSize()) {
        if (auto res = container->takeItems(offset).result())
          return itemDb->toJson(*res);
      }
    }

    return Json();
  }

  Json WorldEntityCallbacks::containerTakeNumItemsAt(World* world, EntityId entityId, std::size_t offset, int const& count) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      if (offset < container->containerSize()) {
        if (auto res = container->takeItems(offset, count).result())
          return itemDb->toJson(*res);
      }
    }

    return Json();
  }

  std::optional<std::size_t> WorldEntityCallbacks::containerItemsCanFit(World* world, EntityId entityId, Json const& items) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto itemBag = container->itemBag();
      auto toSearch = itemDb->fromJson(items);
      return itemBag->itemsCanFit(toSearch);
    }

    return {};
  }

  Json WorldEntityCallbacks::containerItemsFitWhere(World* world, EntityId entityId, Json const& items) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto itemBag = container->itemBag();
      auto toSearch = itemDb->fromJson(items);
      auto res = itemBag->itemsFitWhere(toSearch);
      return JsonObject{
        {"leftover", res.leftover},
        {"slots", jsonFromList<std::size_t>(res.slots)}
      };
    }

    return Json();
  }

  Json WorldEntityCallbacks::containerAddItems(World* world, EntityId entityId, Json const& items) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto toInsert = itemDb->fromJson(items);
      if (auto res = container->addItems(toInsert).result())
        return itemDb->toJson(*res);
    }

    return items;
  }

  Json WorldEntityCallbacks::containerStackItems(World* world, EntityId entityId, Json const& items) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto toInsert = itemDb->fromJson(items);
      if (auto res = container->addItems(toInsert).result())
        return itemDb->toJson(*res);
    }

    return items;
  }

  Json WorldEntityCallbacks::containerPutItemsAt(World* world, EntityId entityId, Json const& items, std::size_t offset) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto toInsert = itemDb->fromJson(items);
      if (offset < container->containerSize()) {
        if (auto res = container->putItems(offset, toInsert).result())
          return itemDb->toJson(*res);
      }
    }

    return items;
  }

  Json WorldEntityCallbacks::containerSwapItems(World* world, EntityId entityId, Json const& items, std::size_t offset) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto toSwap = itemDb->fromJson(items);
      if (offset < container->containerSize()) {
        if (auto res = container->swapItems(offset, toSwap, true).result())
          return itemDb->toJson(*res);
      }
    }

    return items;
  }

  Json WorldEntityCallbacks::containerSwapItemsNoCombine(World* world, EntityId entityId, Json const& items, std::size_t offset) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto toSwap = itemDb->fromJson(items);
      if (offset < container->containerSize()) {
        if (auto res = container->swapItems(offset, toSwap, false).result())
          return itemDb->toJson(*res);
      }
    }

    return items;
  }

  Json WorldEntityCallbacks::containerItemApply(World* world, EntityId entityId, Json const& items, std::size_t offset) {
    if (auto container = world->get<ContainerObject>(entityId)) {
      auto itemDb = world->root().itemDatabase();
      auto toSwap = itemDb->fromJson(items);
      if (offset < container->containerSize()) {
        if (auto res = container->swapItems(offset, toSwap, false).result())
          return itemDb->toJson(*res);
      }
    }

    return items;
  }

  std::optional<LuaValue> WorldEntityCallbacks::callScriptedEntity(World* world, EntityId entityId, String const& function, LuaVariadic<LuaValue> const& args) {
    auto entity = as<ScriptedEntity>(world->entity(entityId));
    if (!entity || !entity->isMaster())
      throw StarException::format("Entity {} does not exist or is not a local master scripted entity", entityId);
    return entity->callScript(function, args);
  }

  RpcPromise<Vec2F> WorldEntityCallbacks::findUniqueEntity(World* world, String const& uniqueId) {
    return world->findUniqueEntity(uniqueId);
  }

  RpcPromise<Json> WorldEntityCallbacks::sendEntityMessage(World* world, LuaEngine& engine, LuaValue entityId, String const& message, LuaVariadic<Json> args) {
    if (std::holds_alternative<LuaString>(entityId))
      return world->sendEntityMessage(engine.luaTo<String>(entityId), message, JsonArray::from(std::move(args)));
    else
      return world->sendEntityMessage(engine.luaTo<EntityId>(entityId), message, JsonArray::from(std::move(args)));
  }

  std::optional<List<EntityId>> WorldEntityCallbacks::loungingEntities(World* world, EntityId entityId, std::optional<std::size_t> anchorIndex) {
    if (auto entity = world->get<LoungeableEntity>(entityId))
      return entity->entitiesLoungingIn(anchorIndex.value_or(std::size_t{})).values();
    return {};
  }

  std::optional<bool> WorldEntityCallbacks::loungeableOccupied(World* world, EntityId entityId, std::optional<std::size_t> anchorIndex) {
    auto entity = world->get<LoungeableEntity>(entityId);
    std::size_t anchor = anchorIndex.value_or(std::size_t{});
    if (entity && entity->anchorCount() > anchor)
      return !entity->entitiesLoungingIn(anchor).empty();
    return {};
  }

  std::optional<std::size_t> WorldEntityCallbacks::loungeableAnchorCount(World* world, EntityId entityId) {
    if (auto entity = world->get<LoungeableEntity>(entityId))
      return entity->anchorCount();
    return {};
  }

  bool WorldEntityCallbacks::isMonster(World* world, EntityId entityId, std::optional<bool> const& aggressive) {
    if (auto entity = world->get<Monster>(entityId))
      return !aggressive || *aggressive == entity->aggressive();

    return false;
  }

  std::optional<String> WorldEntityCallbacks::monsterType(World* world, EntityId entityId) {
    if (auto monster = world->get<Monster>(entityId))
      return monster->typeName();

    return {};
  }

  std::optional<String> WorldEntityCallbacks::npcType(World* world, EntityId entityId) {
    if (auto npc = world->get<Npc>(entityId))
      return npc->npcType();

    return {};
  }

  std::optional<String> WorldEntityCallbacks::stagehandType(World* world, EntityId entityId) {
    if (auto stagehand = world->get<Stagehand>(entityId))
      return stagehand->typeName();

    return {};
  }

  bool WorldEntityCallbacks::isNpc(World* world, EntityId entityId, std::optional<int> const& damageTeam) {
    if (auto entity = world->get<Npc>(entityId)) {
      return !damageTeam || *damageTeam == entity->getTeam().team;
    }

    return false;
  }

  float WorldEnvironmentCallbacks::lightLevel(World* world, Vec2F const& position) {
    return world->lightLevel(position);
  }

  float WorldEnvironmentCallbacks::windLevel(World* world, Vec2F const& position) {
    return world->windLevel(position);
  }

  bool WorldEnvironmentCallbacks::breathable(World* world, Vec2F const& position) {
    return world->breathable(position);
  }

  bool WorldEnvironmentCallbacks::underground(World* world, Vec2F const& position) {
    return world->isUnderground(position);
  }

  LuaValue WorldEnvironmentCallbacks::material(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName) {
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
      auto materialDatabase = world->root().materialDatabase();
      return engine.createString(materialDatabase->materialName(materialId));
    }
  }

  LuaValue WorldEnvironmentCallbacks::mod(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName) {
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
      auto materialDatabase = world->root().materialDatabase();
      return engine.createString(materialDatabase->modName(modId));
    }

    return LuaNil;
  }

  float WorldEnvironmentCallbacks::materialHueShift(World* world, Vec2F const& position, String const& layerName) {
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

  float WorldEnvironmentCallbacks::modHueShift(World* world, Vec2F const& position, String const& layerName) {
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

  MaterialColorVariant WorldEnvironmentCallbacks::materialColor(World* world, Vec2F const& position, String const& layerName) {
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

    world->modifyTile(Vec2I::floor(position), PlaceMaterialColor{layer, color}, true);
  }

  bool WorldEnvironmentCallbacks::damageTiles(World* world,
      List<Vec2I> const& arg1,
      String const& arg2,
      Vec2F const& arg3,
      String const& arg4,
      float arg5,
      std::optional<unsigned> const& arg6,
      std::optional<EntityId> sourceEntity) {
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

  bool WorldEnvironmentCallbacks::damageTileArea(World* world,
      Vec2F center,
      float radius,
      String layer,
      Vec2F sourcePosition,
      String damageType,
      float damage,
      std::optional<unsigned> const& harvestLevel,
      std::optional<EntityId> sourceEntity) {
    auto tiles = tileAreaBrush(radius, center, false);
    return damageTiles(world, tiles, layer, sourcePosition, damageType, damage, harvestLevel, sourceEntity);
  }

  bool WorldEnvironmentCallbacks::placeMaterial(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5) {
    auto tilePosition = arg1;

    PlaceMaterial placeMaterial;

    std::string layerName = arg2.utf8();
    auto split = layerName.find_first_of('+');
    if (split != NPos) {
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
    auto materialDatabase = world->root().materialDatabase();
    if (!materialDatabase->materialNames().contains(materialName))
      throw StarException(strf("Unknown material name {}", materialName));
    placeMaterial.material = materialDatabase->materialId(materialName);

    if (arg4)
      placeMaterial.materialHueShift = (MaterialHue)*arg4;

    bool allowOverlap = arg5;

    return world->modifyTile(tilePosition, placeMaterial, allowOverlap);
  }

  bool WorldEnvironmentCallbacks::replaceMaterials(World* world,
      List<Vec2I> const& tilePositions,
      String const& layer,
      String const& materialName,
      std::optional<int> const& hueShift,
      bool enableDrops) {
    PlaceMaterial placeMaterial;

    std::string layerName = layer.utf8();
    auto split = layerName.find_first_of('+');
    if (split != NPos) {
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

    auto materialDatabase = world->root().materialDatabase();
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

    return world->replaceTiles(modifications, damage).empty();;
  }

  bool WorldEnvironmentCallbacks::replaceMaterialArea(World* world,
      Vec2F center,
      float radius,
      String const& layer,
      String const& materialName,
      std::optional<int> const& hueShift,
      bool enableDrops) {
    auto tiles = tileAreaBrush(radius, center, false);
    return replaceMaterials(world, tiles, layer, materialName, hueShift, enableDrops);
  }

  bool WorldEnvironmentCallbacks::placeMod(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5) {
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
    auto materialDatabase = world->root().materialDatabase();
    if (!materialDatabase->modNames().contains(modName))
      throw StarException(strf("Unknown mod name {}", modName));
    placeMod.mod = materialDatabase->modId(modName);

    if (arg4)
      placeMod.modHueShift = (MaterialHue)*arg4;

    bool allowOverlap = arg5;

    return world->modifyTile(tilePosition, placeMod, allowOverlap);
  }
}

}
