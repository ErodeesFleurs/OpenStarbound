#pragma once

#include "StarBiMap.hpp"
#include "StarRect.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarDrawable.hpp"
#include "StarGameTypes.hpp"
#include "StarCollisionBlock.hpp"
#include "StarLua.hpp"
#include "StarPlatformerAStar.hpp"

namespace Star {

class World;
class WorldServer;
class WorldClient;
class Item;
using ItemPtr = SharedPtr<Item>;
class ScriptedEntity;
using ScriptedEntityPtr = SharedPtr<ScriptedEntity>;

namespace LuaBindings {
  using CallEntityScriptFunction = function<Json(ScriptedEntityPtr const& entity, String const& functionName, JsonArray const& args)>;

  [[nodiscard]] LuaCallbacks makeWorldCallbacks(World& world);

  void addWorldDebugCallbacks(LuaCallbacks& callbacks);
  void addWorldEntityCallbacks(LuaCallbacks& callbacks, World& world);
  void addWorldEnvironmentCallbacks(LuaCallbacks& callbacks, World& world);

  namespace WorldCallbacks {
    [[nodiscard]] float magnitude(World& world, Vec2F pos1, Maybe<Vec2F> pos2);
    [[nodiscard]] Vec2F distance(World& world, Vec2F const& arg1, Vec2F const& arg2);
    [[nodiscard]] bool polyContains(World& world, PolyF const& poly, Vec2F const& pos);
    [[nodiscard]] LuaValue xwrap(World& world, LuaEngine& engine, LuaValue const& positionOrX);
    [[nodiscard]] LuaValue nearestTo(World& world, LuaEngine& engine, Variant<Vec2F, float> const& sourcePositionOrX, Variant<Vec2F, float> const& targetPositionOrX);
    [[nodiscard]] bool rectCollision(World& world, RectF const& arg1, Maybe<CollisionSet> const& arg2);
    [[nodiscard]] bool pointTileCollision(World& world, Vec2F const& arg1, Maybe<CollisionSet> const& arg2);
    [[nodiscard]] bool lineTileCollision(World& world, Vec2F const& arg1, Vec2F const& arg2, Maybe<CollisionSet> const& arg3);
    [[nodiscard]] Maybe<pair<Vec2F, Vec2I>> lineTileCollisionPoint(World& world, Vec2F const& start, Vec2F const& end, Maybe<CollisionSet> const& maybeCollisionSet);
    [[nodiscard]] bool rectTileCollision(World& world, RectF const& arg1, Maybe<CollisionSet> const& arg2);
    [[nodiscard]] bool pointCollision(World& world, Vec2F const& point, Maybe<CollisionSet> const& collisionSet);
    [[nodiscard]] LuaTupleReturn<Maybe<Vec2F>, Maybe<Vec2F>> lineCollision(World& world, Vec2F const& start, Vec2F const& end, Maybe<CollisionSet> const& maybeCollisionSet);
    [[nodiscard]] bool polyCollision(World& world, PolyF const& arg1, Maybe<Vec2F> const& arg2, Maybe<CollisionSet> const& arg3);
    [[nodiscard]] List<Vec2I> collisionBlocksAlongLine(World& world, Vec2F const& arg1, Vec2F const& arg2, Maybe<CollisionSet> const& arg3, Maybe<int> const& arg4);
    [[nodiscard]] List<pair<Vec2I, LiquidLevel>> liquidAlongLine(World& world, Vec2F const& start, Vec2F const& end);
    [[nodiscard]] Maybe<Vec2F> resolvePolyCollision(World& world, PolyF poly, Vec2F const& position, float maximumCorrection, Maybe<CollisionSet> const& collisionSet);
    [[nodiscard]] bool tileIsOccupied(World& world, Vec2I const& arg1, Maybe<bool> const& arg2, Maybe<bool> const& arg3);
    [[nodiscard]] bool placeObject(World& world, String const& arg1, Vec2I const& arg2, Maybe<int> const& arg3, Json const& arg4);
    [[nodiscard]] Maybe<EntityId> spawnItem(World& world, Json const& itemType, Vec2F const& worldPosition, Maybe<size_t> const& inputCount, Json const& inputParameters, Maybe<Vec2F> const& initialVelocity, Maybe<float> const& intangibleTime);
    [[nodiscard]] List<EntityId> spawnTreasure(World& world, Vec2F const& position, String const& pool, float level, Maybe<uint64_t> seed);
    [[nodiscard]] Maybe<EntityId> spawnMonster(World& world, String const& arg1, Vec2F const& arg2, Maybe<JsonObject> const& arg3);
    [[nodiscard]] Maybe<EntityId> spawnNpc(World& world, Vec2F const& arg1, String const& arg2, String const& arg3, float arg4, Maybe<uint64_t> arg5, Json const& arg6);
    [[nodiscard]] Maybe<EntityId> spawnStagehand(World& world, Vec2F const& spawnPosition, String const& typeName, Json const& overrides);
    [[nodiscard]] Maybe<EntityId> spawnProjectile(World& world, String const& arg1, Vec2F const& arg2, Maybe<EntityId> const& arg3, Maybe<Vec2F> const& arg4, bool arg5, Json const& arg6);
    [[nodiscard]] Maybe<EntityId> spawnVehicle(World& world, String const& vehicleName, Vec2F const& pos, Json const& extraConfig);
    [[nodiscard]] double time(World& world);
    [[nodiscard]] uint64_t day(World& world);
    [[nodiscard]] double timeOfDay(World& world);
    [[nodiscard]] float dayLength(World& world);
    [[nodiscard]] Json getProperty(World& world, String const& arg1, Json const& arg2);
    void setProperty(World& world, String const& arg1, Json const& arg2);
    [[nodiscard]] Maybe<LiquidLevel> liquidAt(World& world, Variant<RectF, Vec2I> boundBoxOrPoint);
    [[nodiscard]] float gravity(World& world, Vec2F const& arg1);
    [[nodiscard]] bool spawnLiquid(World& world, Vec2F const& arg1, LiquidId arg2, float arg3);
    [[nodiscard]] Maybe<LiquidLevel> destroyLiquid(World& world, Vec2F const& position);
    [[nodiscard]] bool isTileProtected(World& world, Vec2F const& position);
    Maybe<PlatformerAStar::Path> findPlatformerPath(World& world, Vec2F const& start, Vec2F const& end, ActorMovementParameters actorMovementParameters, PlatformerAStar::Parameters searchParameters);
    PlatformerAStar::PathFinder platformerPathStart(World& world, Vec2F const& start, Vec2F const& end, ActorMovementParameters actorMovementParameters, PlatformerAStar::Parameters searchParameters);
  }

  namespace ClientWorldCallbacks {
    void resendEntity(WorldClient* world, EntityId arg1);
    [[nodiscard]] RectI clientWindow(WorldClient* world);
  }

  namespace ServerWorldCallbacks {
    [[nodiscard]] String id(WorldServer* world);
    [[nodiscard]] bool breakObject(WorldServer* world, EntityId arg1, bool arg2);
    [[nodiscard]] bool isVisibleToPlayer(WorldServer* world, RectF const& arg1);
    [[nodiscard]] bool loadRegion(WorldServer* world, RectF const& arg1);
    [[nodiscard]] bool regionActive(WorldServer* world, RectF const& arg1);
    void setTileProtection(WorldServer* world, DungeonId arg1, bool arg2);
    [[nodiscard]] bool isPlayerModified(WorldServer* world, RectI const& region);
    [[nodiscard]] Maybe<LiquidLevel> forceDestroyLiquid(WorldServer* world, Vec2F const& position);
    [[nodiscard]] EntityId loadUniqueEntity(WorldServer* world, String const& uniqueId);
    void setUniqueId(WorldServer* world, EntityId entityId, Maybe<String> const& uniqueId);
    [[nodiscard]] Json takeItemDrop(World& world, EntityId entityId, Maybe<EntityId> const& takenBy);
    void setPlayerStart(World& world, Vec2F const& playerStart, Maybe<bool> respawnInWorld);
    [[nodiscard]] List<EntityId> players(World& world);
    [[nodiscard]] LuaString fidelity(World& world, LuaEngine& engine);
    [[nodiscard]] Maybe<LuaValue> callScriptContext(World& world, String const& contextName, String const& function, LuaVariadic<LuaValue> const& args);
    [[nodiscard]] bool sendPacket(WorldServer* world, ConnectionId clientId, String const& packetType, Json const& packetData);
  }

  namespace WorldDebugCallbacks {
    void debugPoint(Vec2F const& arg1, Color const& arg2);
    void debugLine(Vec2F const& arg1, Vec2F const& arg2, Color const& arg3);
    void debugPoly(PolyF const& poly, Color const& color);
    void debugText(LuaEngine& engine, LuaVariadic<LuaValue> const& args);
  }

  namespace WorldEntityCallbacks {
    [[nodiscard]] LuaTable entityQuery(World& world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable monsterQuery(World& world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable npcQuery(World& world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable objectQuery(World& world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable itemDropQuery(World& world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable playerQuery(World& world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable loungeableQuery(World& world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable entityLineQuery(World& world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable objectLineQuery(World& world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] LuaTable npcLineQuery(World& world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, Maybe<LuaTable> options);
    [[nodiscard]] bool entityExists(World& world, EntityId entityId);
    [[nodiscard]] bool entityCanDamage(World& world, EntityId sourceId, EntityId targetId);
    [[nodiscard]] Json entityDamageTeam(World& world, EntityId entityId);
    [[nodiscard]] bool entityAggressive(World& world, EntityId entityId);
    [[nodiscard]] Maybe<LuaString> entityType(World& world, LuaEngine& engine, EntityId entityId);
    [[nodiscard]] Maybe<Vec2F> entityPosition(World& world, EntityId entityId);
    [[nodiscard]] Maybe<Vec2F> entityVelocity(World& world, EntityId entityId);
    [[nodiscard]] Maybe<RectF> entityMetaBoundBox(World& world, EntityId entityId);
    [[nodiscard]] Maybe<uint64_t> entityCurrency(World& world, EntityId entityId, String const& currencyType);
    [[nodiscard]] Maybe<uint64_t> entityHasCountOfItem(World& world, EntityId entityId, Json descriptor, Maybe<bool> exactMatch);
    [[nodiscard]] Maybe<Vec2F> entityHealth(World& world, EntityId entityId);
    [[nodiscard]] Maybe<String> entitySpecies(World& world, EntityId entityId);
    [[nodiscard]] Maybe<String> entityGender(World& world, EntityId entityId);
    [[nodiscard]] Maybe<String> entityName(World& world, EntityId entityId);
    [[nodiscard]] Maybe<Json> entityNametag(World& world, EntityId entityId);
    [[nodiscard]] Maybe<String> entityDescription(World& world, EntityId entityId, Maybe<String> const& species);
    [[nodiscard]] LuaNullTermWrapper<Maybe<List<Drawable>>> entityPortrait(World& world, EntityId entityId, String const& portraitMode);
    [[nodiscard]] Maybe<String> entityHandItem(World& world, EntityId entityId, String const& handName);
    [[nodiscard]] Json entityHandItemDescriptor(World& world, EntityId entityId, String const& handName);
    [[nodiscard]] LuaNullTermWrapper<Maybe<String>> entityUniqueId(World& world, EntityId entityId);
    [[nodiscard]] Json getObjectParameter(World& world, EntityId entityId, String const& parameterName, Maybe<Json> const& defaultValue);
    [[nodiscard]] Json getNpcScriptParameter(World& world, EntityId entityId, String const& parameterName, Maybe<Json> const& defaultValue);
    [[nodiscard]] List<Vec2I> objectSpaces(World& world, EntityId entityId);
    [[nodiscard]] Maybe<int> farmableStage(World& world, EntityId entityId);
    [[nodiscard]] Maybe<int> containerSize(World& world, EntityId entityId);
    [[nodiscard]] bool containerClose(World& world, EntityId entityId);
    [[nodiscard]] bool containerOpen(World& world, EntityId entityId);
    [[nodiscard]] Json containerItems(World& world, EntityId entityId);
    [[nodiscard]] Json containerItemAt(World& world, EntityId entityId, size_t offset);
    [[nodiscard]] Maybe<bool> containerConsume(World& world, EntityId entityId, Json const& items);
    [[nodiscard]] Maybe<bool> containerConsumeAt(World& world, EntityId entityId, size_t offset, int count);
    [[nodiscard]] Maybe<size_t> containerAvailable(World& world, EntityId entityId, Json const& items);
    [[nodiscard]] Json containerTakeAll(World& world, EntityId entityId);
    [[nodiscard]] Json containerTakeAt(World& world, EntityId entityId, size_t offset);
    [[nodiscard]] Json containerTakeNumItemsAt(World& world, EntityId entityId, size_t offset, int const& count);
    [[nodiscard]] Maybe<size_t> containerItemsCanFit(World& world, EntityId entityId, Json const& items);
    [[nodiscard]] Json containerItemsFitWhere(World& world, EntityId entityId, Json const& items);
    [[nodiscard]] Json containerAddItems(World& world, EntityId entityId, Json const& items);
    [[nodiscard]] Json containerStackItems(World& world, EntityId entityId, Json const& items);
    [[nodiscard]] Json containerPutItemsAt(World& world, EntityId entityId, Json const& items, size_t offset);
    [[nodiscard]] Json containerSwapItems(World& world, EntityId entityId, Json const& items, size_t offset);
    [[nodiscard]] Json containerSwapItemsNoCombine(World& world, EntityId entityId, Json const& items, size_t offset);
    [[nodiscard]] Json containerItemApply(World& world, EntityId entityId, Json const& items, size_t offset);
    [[nodiscard]] Maybe<LuaValue> callScriptedEntity(World& world, EntityId entityId, String const& function, LuaVariadic<LuaValue> const& args);
    [[nodiscard]] RpcPromise<Vec2F> findUniqueEntity(World& world, String const& uniqueId);
    [[nodiscard]] RpcPromise<Json> sendEntityMessage(World& world, LuaEngine& engine, LuaValue entityId, String const& message, LuaVariadic<Json> args);
    [[nodiscard]] Maybe<List<EntityId>> loungingEntities(World& world, EntityId entityId, Maybe<size_t> anchorIndex);
    [[nodiscard]] Maybe<bool> loungeableOccupied(World& world, EntityId entityId, Maybe<size_t> anchorIndex);
    [[nodiscard]] Maybe<size_t> loungeableAnchorCount(World& world, EntityId entityId);
    [[nodiscard]] bool isMonster(World& world, EntityId entityId, Maybe<bool> const& aggressive);
    [[nodiscard]] Maybe<String> monsterType(World& world, EntityId entityId);
    [[nodiscard]] Maybe<String> npcType(World& world, EntityId entityId);
    [[nodiscard]] Maybe<String> stagehandType(World& world, EntityId entityId);
    [[nodiscard]] bool isNpc(World& world, EntityId entityId, Maybe<int> const& damageTeam);
  }

  namespace WorldEnvironmentCallbacks {
    [[nodiscard]] float lightLevel(World& world, Vec2F const& position);
    [[nodiscard]] float windLevel(World& world, Vec2F const& position);
    [[nodiscard]] bool breathable(World& world, Vec2F const& position);
    [[nodiscard]] bool underground(World& world, Vec2F const& position);
    [[nodiscard]] LuaValue material(World& world, LuaEngine& engine, Vec2F const& position, String const& layerName);
    [[nodiscard]] LuaValue mod(World& world, LuaEngine& engine, Vec2F const& position, String const& layerName);
    [[nodiscard]] float materialHueShift(World& world, Vec2F const& position, String const& layerName);
    [[nodiscard]] float modHueShift(World& world, Vec2F const& position, String const& layerName);
    [[nodiscard]] MaterialColorVariant materialColor(World& world, Vec2F const& position, String const& layerName);
    void setMaterialColor(World& world, Vec2F const& position, String const& layerName, MaterialColorVariant color);
    [[nodiscard]] bool damageTiles(World& world, List<Vec2I> const& arg1, String const& arg2, Vec2F const& arg3, String const& arg4, float arg5, Maybe<unsigned> const& arg6, Maybe<EntityId> sourceEntity);
    [[nodiscard]] bool damageTileArea(World& world, Vec2F center, float radius, String layer, Vec2F sourcePosition, String damageType, float damage, Maybe<unsigned> const& harvestLevel, Maybe<EntityId> sourceEntity);
    [[nodiscard]] bool placeMaterial(World& world, Vec2I const& arg1, String const& arg2, String const& arg3, Maybe<int> const& arg4, bool arg5);
    [[nodiscard]] bool replaceMaterials(World& world, List<Vec2I> const& tilePositions, String const& layer, String const& materialName, Maybe<int> const& hueShift, bool enableDrops);
    [[nodiscard]] bool replaceMaterialArea(World& world, Vec2F center, float radius, String const& layer, String const& materialName, Maybe<int> const& hueShift, bool enableDrops);
    [[nodiscard]] bool placeMod(World& world, Vec2I const& arg1, String const& arg2, String const& arg3, Maybe<int> const& arg4, bool arg5);
  }
}

}
