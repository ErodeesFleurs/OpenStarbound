#pragma once

#include "StarColor.hpp"
#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarGameTypes.hpp"
#include "StarLua.hpp"
#include "StarPlatformerAStar.hpp"
#include "StarPoly.hpp"
#include "StarRect.hpp"

import std;

namespace Star {

class ScriptedEntity;
class WorldClient;
class WorldServer;

namespace LuaBindings {
using CallEntityScriptFunction = std::function<Json(Ptr<ScriptedEntity> const& entity, String const& functionName, JsonArray const& args)>;

auto makeWorldCallbacks(World* world) -> LuaCallbacks;

void addWorldDebugCallbacks(LuaCallbacks& callbacks);
void addWorldEntityCallbacks(LuaCallbacks& callbacks, World* world);
void addWorldEnvironmentCallbacks(LuaCallbacks& callbacks, World* world);

namespace WorldCallbacks {
auto magnitude(World* world, Vec2F pos1, std::optional<Vec2F> pos2) -> float;
auto distance(World* world, Vec2F const& arg1, Vec2F const& arg2) -> Vec2F;
auto polyContains(World* world, PolyF const& poly, Vec2F const& pos) -> bool;
auto xwrap(World* world, LuaEngine& engine, LuaValue const& positionOrX) -> LuaValue;
auto nearestTo(World* world, LuaEngine& engine, Variant<Vec2F, float> const& sourcePositionOrX, Variant<Vec2F, float> const& targetPositionOrX) -> LuaValue;
auto rectCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2) -> bool;
auto pointTileCollision(World* world, Vec2F const& arg1, std::optional<CollisionSet> const& arg2) -> bool;
auto lineTileCollision(World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3) -> bool;
auto lineTileCollisionPoint(World* world, Vec2F const& start, Vec2F const& end, std::optional<CollisionSet> const& maybeCollisionSet) -> std::optional<std::pair<Vec2F, Vec2I>>;
auto rectTileCollision(World* world, RectF const& arg1, std::optional<CollisionSet> const& arg2) -> bool;
auto pointCollision(World* world, Vec2F const& point, std::optional<CollisionSet> const& collisionSet) -> bool;
auto lineCollision(World* world, Vec2F const& start, Vec2F const& end, std::optional<CollisionSet> const& maybeCollisionSet) -> LuaTupleReturn<std::optional<Vec2F>, std::optional<Vec2F>>;
auto polyCollision(World* world, PolyF const& arg1, std::optional<Vec2F> const& arg2, std::optional<CollisionSet> const& arg3) -> bool;
auto collisionBlocksAlongLine(World* world, Vec2F const& arg1, Vec2F const& arg2, std::optional<CollisionSet> const& arg3, std::optional<int> const& arg4) -> List<Vec2I>;
auto liquidAlongLine(World* world, Vec2F const& start, Vec2F const& end) -> List<std::pair<Vec2I, LiquidLevel>>;
auto resolvePolyCollision(World* world, PolyF poly, Vec2F const& position, float maximumCorrection, std::optional<CollisionSet> const& collisionSet) -> std::optional<Vec2F>;
auto tileIsOccupied(World* world, Vec2I const& arg1, std::optional<bool> const& arg2, std::optional<bool> const& arg3) -> bool;
auto placeObject(World* world, String const& arg1, Vec2I const& arg2, std::optional<int> const& arg3, Json const& arg4) -> bool;
auto spawnItem(World* world, Json const& itemType, Vec2F const& worldPosition, std::optional<size_t> const& inputCount, Json const& inputParameters, std::optional<Vec2F> const& initialVelocity, std::optional<float> const& intangibleTime) -> std::optional<EntityId>;
auto spawnTreasure(World* world, Vec2F const& position, String const& pool, float level, std::optional<std::uint64_t> seed) -> List<EntityId>;
auto spawnMonster(World* world, String const& arg1, Vec2F const& arg2, std::optional<JsonObject> const& arg3) -> std::optional<EntityId>;
auto spawnNpc(World* world, Vec2F const& arg1, String const& arg2, String const& arg3, float arg4, std::optional<std::uint64_t> arg5, Json const& arg6) -> std::optional<EntityId>;
auto spawnStagehand(World* world, Vec2F const& spawnPosition, String const& typeName, Json const& overrides) -> std::optional<EntityId>;
auto spawnProjectile(World* world, String const& arg1, Vec2F const& arg2, std::optional<EntityId> const& arg3, std::optional<Vec2F> const& arg4, bool arg5, Json const& arg6) -> std::optional<EntityId>;
auto spawnVehicle(World* world, String const& vehicleName, Vec2F const& pos, Json const& extraConfig) -> std::optional<EntityId>;
auto time(World* world) -> double;
auto day(World* world) -> std::uint64_t;
auto timeOfDay(World* world) -> double;
auto dayLength(World* world) -> float;
auto getProperty(World* world, String const& arg1, Json const& arg2) -> Json;
void setProperty(World* world, String const& arg1, Json const& arg2);
auto liquidAt(World* world, Variant<RectF, Vec2I> boundBoxOrPoint) -> std::optional<LiquidLevel>;
auto gravity(World* world, Vec2F const& arg1) -> float;
auto spawnLiquid(World* world, Vec2F const& arg1, LiquidId arg2, float arg3) -> bool;
auto destroyLiquid(World* world, Vec2F const& position) -> std::optional<LiquidLevel>;
auto isTileProtected(World* world, Vec2F const& position) -> bool;
auto findPlatformerPath(World* world, Vec2F const& start, Vec2F const& end, ActorMovementParameters actorMovementParameters, PlatformerAStar::Parameters searchParameters) -> std::optional<PlatformerAStar::Path>;
auto platformerPathStart(World* world, Vec2F const& start, Vec2F const& end, ActorMovementParameters actorMovementParameters, PlatformerAStar::Parameters searchParameters) -> PlatformerAStar::PathFinder;
}// namespace WorldCallbacks

namespace ClientWorldCallbacks {
void resendEntity(WorldClient* world, EntityId arg1);
auto clientWindow(WorldClient* world) -> RectI;
}// namespace ClientWorldCallbacks

namespace ServerWorldCallbacks {
auto id(WorldServer* world) -> String;
auto breakObject(WorldServer* world, EntityId arg1, bool arg2) -> bool;
auto isVisibleToPlayer(WorldServer* world, RectF const& arg1) -> bool;
auto loadRegion(WorldServer* world, RectF const& arg1) -> bool;
auto regionActive(WorldServer* world, RectF const& arg1) -> bool;
void setTileProtection(WorldServer* world, DungeonId arg1, bool arg2);
auto isPlayerModified(WorldServer* world, RectI const& region) -> bool;
auto forceDestroyLiquid(WorldServer* world, Vec2F const& position) -> std::optional<LiquidLevel>;
auto loadUniqueEntity(WorldServer* world, String const& uniqueId) -> EntityId;
void setUniqueId(WorldServer* world, EntityId entityId, std::optional<String> const& uniqueId);
auto takeItemDrop(World* world, EntityId entityId, std::optional<EntityId> const& takenBy) -> Json;
void setPlayerStart(World* world, Vec2F const& playerStart, std::optional<bool> respawnInWorld);
auto players(World* world) -> List<EntityId>;
auto fidelity(World* world, LuaEngine& engine) -> LuaString;
auto callScriptContext(World* world, String const& contextName, String const& function, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue>;
auto sendPacket(WorldServer* world, ConnectionId clientId, String const& packetType, Json const& packetData) -> bool;
}// namespace ServerWorldCallbacks

namespace WorldDebugCallbacks {
void debugPoint(Vec2F const& arg1, Color const& arg2);
void debugLine(Vec2F const& arg1, Vec2F const& arg2, Color const& arg3);
void debugPoly(PolyF const& poly, Color const& color);
void debugText(LuaEngine& engine, LuaVariadic<LuaValue> const& args);
}// namespace WorldDebugCallbacks

namespace WorldEntityCallbacks {
auto entityQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto monsterQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto npcQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto objectQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto itemDropQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto playerQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto loungeableQuery(World* world, LuaEngine& engine, Vec2F const& pos1, LuaValue const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto entityLineQuery(World* world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto objectLineQuery(World* world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto npcLineQuery(World* world, LuaEngine& engine, Vec2F const& pos1, Vec2F const& pos2, std::optional<LuaTable> options) -> LuaTable;
auto entityExists(World* world, EntityId entityId) -> bool;
auto entityCanDamage(World* world, EntityId sourceId, EntityId targetId) -> bool;
auto entityDamageTeam(World* world, EntityId entityId) -> Json;
auto entityAggressive(World* world, EntityId entityId) -> bool;
auto entityType(World* world, LuaEngine& engine, EntityId entityId) -> std::optional<LuaString>;
auto entityPosition(World* world, EntityId entityId) -> std::optional<Vec2F>;
auto entityVelocity(World* world, EntityId entityId) -> std::optional<Vec2F>;
auto entityMetaBoundBox(World* world, EntityId entityId) -> std::optional<RectF>;
auto entityCurrency(World* world, EntityId entityId, String const& currencyType) -> std::optional<std::uint64_t>;
auto entityHasCountOfItem(World* world, EntityId entityId, Json descriptor, std::optional<bool> exactMatch) -> std::optional<std::uint64_t>;
auto entityHealth(World* world, EntityId entityId) -> std::optional<Vec2F>;
auto entitySpecies(World* world, EntityId entityId) -> std::optional<String>;
auto entityGender(World* world, EntityId entityId) -> std::optional<String>;
auto entityName(World* world, EntityId entityId) -> std::optional<String>;
auto entityNametag(World* world, EntityId entityId) -> std::optional<Json>;
auto entityDescription(World* world, EntityId entityId, std::optional<String> const& species) -> std::optional<String>;
auto entityPortrait(World* world, EntityId entityId, String const& portraitMode) -> LuaNullTermWrapper<std::optional<List<Drawable>>>;
auto entityHandItem(World* world, EntityId entityId, String const& handName) -> std::optional<String>;
auto entityHandItemDescriptor(World* world, EntityId entityId, String const& handName) -> Json;
auto entityUniqueId(World* world, EntityId entityId) -> LuaNullTermWrapper<std::optional<String>>;
auto getObjectParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue) -> Json;
auto getNpcScriptParameter(World* world, EntityId entityId, String const& parameterName, std::optional<Json> const& defaultValue) -> Json;
auto objectSpaces(World* world, EntityId entityId) -> List<Vec2I>;
auto farmableStage(World* world, EntityId entityId) -> std::optional<int>;
auto containerSize(World* world, EntityId entityId) -> std::optional<int>;
auto containerClose(World* world, EntityId entityId) -> bool;
auto containerOpen(World* world, EntityId entityId) -> bool;
auto containerItems(World* world, EntityId entityId) -> Json;
auto containerItemAt(World* world, EntityId entityId, size_t offset) -> Json;
auto containerConsume(World* world, EntityId entityId, Json const& items) -> std::optional<bool>;
auto containerConsumeAt(World* world, EntityId entityId, size_t offset, int count) -> std::optional<bool>;
auto containerAvailable(World* world, EntityId entityId, Json const& items) -> std::optional<size_t>;
auto containerTakeAll(World* world, EntityId entityId) -> Json;
auto containerTakeAt(World* world, EntityId entityId, size_t offset) -> Json;
auto containerTakeNumItemsAt(World* world, EntityId entityId, size_t offset, int const& count) -> Json;
auto containerItemsCanFit(World* world, EntityId entityId, Json const& items) -> std::optional<size_t>;
auto containerItemsFitWhere(World* world, EntityId entityId, Json const& items) -> Json;
auto containerAddItems(World* world, EntityId entityId, Json const& items) -> Json;
auto containerStackItems(World* world, EntityId entityId, Json const& items) -> Json;
auto containerPutItemsAt(World* world, EntityId entityId, Json const& items, size_t offset) -> Json;
auto containerSwapItems(World* world, EntityId entityId, Json const& items, size_t offset) -> Json;
auto containerSwapItemsNoCombine(World* world, EntityId entityId, Json const& items, size_t offset) -> Json;
auto containerItemApply(World* world, EntityId entityId, Json const& items, size_t offset) -> Json;
auto callScriptedEntity(World* world, EntityId entityId, String const& function, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue>;
auto findUniqueEntity(World* world, String const& uniqueId) -> RpcPromise<Vec2F>;
auto sendEntityMessage(World* world, LuaEngine& engine, LuaValue entityId, String const& message, LuaVariadic<Json> args) -> RpcPromise<Json>;
auto loungingEntities(World* world, EntityId entityId, std::optional<size_t> anchorIndex) -> std::optional<List<EntityId>>;
auto loungeableOccupied(World* world, EntityId entityId, std::optional<size_t> anchorIndex) -> std::optional<bool>;
auto loungeableAnchorCount(World* world, EntityId entityId) -> std::optional<size_t>;
auto isMonster(World* world, EntityId entityId, std::optional<bool> const& aggressive) -> bool;
auto monsterType(World* world, EntityId entityId) -> std::optional<String>;
auto npcType(World* world, EntityId entityId) -> std::optional<String>;
auto stagehandType(World* world, EntityId entityId) -> std::optional<String>;
auto isNpc(World* world, EntityId entityId, std::optional<int> const& damageTeam) -> bool;
}// namespace WorldEntityCallbacks

namespace WorldEnvironmentCallbacks {
auto lightLevel(World* world, Vec2F const& position) -> float;
auto windLevel(World* world, Vec2F const& position) -> float;
auto breathable(World* world, Vec2F const& position) -> bool;
auto underground(World* world, Vec2F const& position) -> bool;
auto material(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName) -> LuaValue;
auto mod(World* world, LuaEngine& engine, Vec2F const& position, String const& layerName) -> LuaValue;
auto materialHueShift(World* world, Vec2F const& position, String const& layerName) -> float;
auto modHueShift(World* world, Vec2F const& position, String const& layerName) -> float;
auto materialColor(World* world, Vec2F const& position, String const& layerName) -> MaterialColorVariant;
void setMaterialColor(World* world, Vec2F const& position, String const& layerName, MaterialColorVariant color);
auto damageTiles(World* world, List<Vec2I> const& arg1, String const& arg2, Vec2F const& arg3, String const& arg4, float arg5, std::optional<unsigned> const& arg6, std::optional<EntityId> sourceEntity) -> bool;
auto damageTileArea(World* world, Vec2F center, float radius, String layer, Vec2F sourcePosition, String damageType, float damage, std::optional<unsigned> const& harvestLevel, std::optional<EntityId> sourceEntity) -> bool;
auto placeMaterial(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5) -> bool;
auto replaceMaterials(World* world, List<Vec2I> const& tilePositions, String const& layer, String const& materialName, std::optional<int> const& hueShift, bool enableDrops) -> bool;
auto replaceMaterialArea(World* world, Vec2F center, float radius, String const& layer, String const& materialName, std::optional<int> const& hueShift, bool enableDrops) -> bool;
auto placeMod(World* world, Vec2I const& arg1, String const& arg2, String const& arg3, std::optional<int> const& arg4, bool arg5) -> bool;
}// namespace WorldEnvironmentCallbacks
}// namespace LuaBindings

}// namespace Star
