#pragma once

#include "StarCellularLighting.hpp"
#include "StarCellularLiquid.hpp"
#include "StarCollisionGenerator.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarInterpolationTracker.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaRoot.hpp"
#include "StarNetPackets.hpp"
#include "StarSpawner.hpp"
#include "StarWarping.hpp"
#include "StarWeather.hpp"
#include "StarWorld.hpp"
#include "StarWorldClientState.hpp"
#include "StarWorldStructure.hpp"

import std;

namespace Star {

class WorldTemplate;
class UniverseSettings;
class UniverseServer;
class Player;
class Sky;
class TileEntity;
class WireProcessor;
class FallingBlocksAgent;

using WorldServerException = ExceptionDerived<"WorldServerException">;

// Describes the amount of optional processing that a call to update() in
// WorldServer performs for things like liquid simulation, wiring, sector
// generation etc.
enum class WorldServerFidelity {
  Minimum,
  Low,
  Medium,
  High
};
extern EnumMap<WorldServerFidelity> const WorldServerFidelityNames;

class WorldServer : public World {
public:
  using ScriptComponent = LuaMessageHandlingComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>;
  using ScriptComponentPtr = std::shared_ptr<ScriptComponent>;
  using WorldPropertyListener = std::function<void(Json const&)>;

  // Create a new world with the given template, writing new storage file.
  WorldServer(Ptr<WorldTemplate> const& worldTemplate, Ptr<IODevice> storage);
  // Synonym for WorldServer(make_shared<WorldTemplate>(size), storage);
  WorldServer(Vec2U const& size, Ptr<IODevice> storage);
  // Load an existing world from the given storage files
  WorldServer(Ptr<IODevice> const& storage);
  // Load an existing world from the given in-memory chunks
  WorldServer(WorldChunks const& chunks);
  // Load an existing world from an in-memory representation
  ~WorldServer() override;

  void setWorldId(String worldId);
  auto worldId() const -> String const&;

  void setUniverseSettings(Ptr<UniverseSettings> universeSettings);
  auto universeSettings() const -> Ptr<UniverseSettings>;

  void setReferenceClock(Ptr<Clock> clock);

  void initLua(UniverseServer* universe);

  // Give this world a central structure.  If there is a previous central
  // structure it is removed first.  Returns the structure with transformed
  // coordinates.
  auto setCentralStructure(WorldStructure centralStructure) -> WorldStructure;
  auto centralStructure() const -> WorldStructure const&;
  // If there is an active central structure, it is removed and all unmodified
  // objects and blocks associated with the structure are removed.
  void removeCentralStructure();

  void setPlayerStart(Vec2F const& startPosition, bool respawnInWorld = false);

  auto spawnTargetValid(SpawnTarget const& spawnTarget) const -> bool;

  // Returns false if the client id already exists, or the spawn target is
  // invalid.
  auto addClient(ConnectionId clientId, SpawnTarget const& spawnTarget, bool isLocal, bool isAdmin = false, NetCompatibilityRules netRules = {}) -> bool;

  // Removes client, sends the WorldStopPacket, and returns any pending packets
  // for that client
  auto removeClient(ConnectionId clientId) -> List<Ptr<Packet>>;

  auto clientIds() const -> List<ConnectionId>;
  auto hasClient(ConnectionId clientId) const -> bool;
  auto clientWindow(ConnectionId clientId) const -> RectF;
  // May return null if a Player is not available or if the client id is not
  // valid.
  auto clientPlayer(ConnectionId clientId) const -> Ptr<Player>;

  auto players() const -> List<EntityId>;

  void handleIncomingPackets(ConnectionId clientId, List<Ptr<Packet>> const& packets);
  auto getOutgoingPackets(ConnectionId clientId) -> List<Ptr<Packet>>;
  auto sendPacket(ConnectionId clientId, Ptr<Packet> const& packet) -> bool;

  auto receiveMessage(ConnectionId fromConnection, String const& message, JsonArray const& args) -> std::optional<Json>;

  void startFlyingSky(bool enterHyperspace, bool startInWarp, Json settings = {});
  void stopFlyingSkyAt(SkyParameters const& destination);
  void setOrbitalSky(SkyParameters const& destination);

  // Defaults to Medium
  auto fidelity() const -> WorldServerFidelity;
  void setFidelity(WorldServerFidelity fidelity);

  auto shouldExpire() -> bool;
  void setExpiryTime(float expiryTime);
  auto expiryTime() -> float;

  void update(float dt);

  auto connection() const -> ConnectionId override;
  auto geometry() const -> WorldGeometry override;
  auto currentStep() const -> uint64_t override;
  auto material(Vec2I const& position, TileLayer layer) const -> MaterialId override;
  auto materialHueShift(Vec2I const& position, TileLayer layer) const -> MaterialHue override;
  auto mod(Vec2I const& position, TileLayer layer) const -> ModId override;
  auto modHueShift(Vec2I const& position, TileLayer layer) const -> MaterialHue override;
  auto colorVariant(Vec2I const& position, TileLayer layer) const -> MaterialColorVariant override;
  auto liquidLevel(Vec2I const& pos) const -> LiquidLevel override;
  auto liquidLevel(RectF const& region) const -> LiquidLevel override;

  auto validTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) const -> TileModificationList override;
  auto applyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) -> TileModificationList override;
  auto replaceTile(Vec2I const& pos, TileModification const& modification, TileDamage const& tileDamage) -> bool;
  auto replaceTiles(TileModificationList const& modificationList, TileDamage const& tileDamage, bool applyDamage = false) -> TileModificationList override;
  auto damageWouldDestroy(Vec2I const& pos, TileLayer layer, TileDamage const& tileDamage) const -> bool override;
  auto entity(EntityId entityId) const -> Ptr<Entity> override;
  void addEntity(Ptr<Entity> const& entity, EntityId entityId = NullEntityId) override;
  auto closestEntity(Vec2F const& center, float radius, EntityFilter selector = EntityFilter()) const -> Ptr<Entity> override;
  void forAllEntities(EntityCallback entityCallback) const override;
  void forEachEntity(RectF const& boundBox, EntityCallback callback) const override;
  void forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback callback) const override;
  void forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> entityCallback) const override;
  auto findEntity(RectF const& boundBox, EntityFilter entityFilter) const -> Ptr<Entity> override;
  auto findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter entityFilter) const -> Ptr<Entity> override;
  auto findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> entityFilter) const -> Ptr<Entity> override;
  auto tileIsOccupied(Vec2I const& pos, TileLayer layer, bool includeEphemeral = false, bool checkCollision = false) const -> bool override;
  auto tileCollisionKind(Vec2I const& pos) const -> CollisionKind override;
  void forEachCollisionBlock(RectI const& region, std::function<void(CollisionBlock const&)> const& iterator) const override;
  auto isTileConnectable(Vec2I const& pos, TileLayer layer, bool tilesOnly = false) const -> bool override;
  auto pointTileCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool override;
  auto lineTileCollision(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool override;
  auto lineTileCollisionPoint(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const -> std::optional<std::pair<Vec2F, Vec2I>> override;
  auto collidingTilesAlongLine(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet, int maxSize = -1, bool includeEdges = true) const -> List<Vec2I> override;
  auto rectTileCollision(RectI const& region, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool override;
  auto damageTiles(List<Vec2I> const& pos, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, std::optional<EntityId> sourceEntity = std::nullopt) -> TileDamageResult override;
  auto getInteractiveInRange(Vec2F const& targetPosition, Vec2F const& sourcePosition, float maxRange) const -> Ptr<InteractiveEntity> override;
  auto canReachEntity(Vec2F const& position, float radius, EntityId targetEntity, bool preferInteractive = true) const -> bool override;
  auto interact(InteractRequest const& request) -> RpcPromise<InteractAction> override;
  auto gravity(Vec2F const& pos) const -> float override;
  auto windLevel(Vec2F const& pos) const -> float override;
  auto lightLevel(Vec2F const& pos) const -> float override;
  auto breathable(Vec2F const& pos) const -> bool override;
  auto threatLevel() const -> float override;
  auto environmentStatusEffects(Vec2F const& pos) const -> StringList override;
  auto weatherStatusEffects(Vec2F const& pos) const -> StringList override;
  auto exposedToWeather(Vec2F const& pos) const -> bool override;
  auto isUnderground(Vec2F const& pos) const -> bool override;
  auto disableDeathDrops() const -> bool override;
  auto forceRegions() const -> List<PhysicsForceRegion> override;
  auto getProperty(String const& propertyName, Json const& def = Json()) const -> Json override;
  void setProperty(String const& propertyName, Json const& property) override;
  void timer(float delay, WorldAction worldAction) override;
  auto epochTime() const -> double override;
  auto day() const -> uint32_t override;
  auto dayLength() const -> float override;
  auto timeOfDay() const -> float override;
  auto luaRoot() -> Ptr<LuaRoot> override;
  auto findUniqueEntity(String const& uniqueId) -> RpcPromise<Vec2F> override;
  auto sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) -> RpcPromise<Json> override;
  auto isTileProtected(Vec2I const& pos) const -> bool override;
  void wire(Vec2I const& outputPosition, size_t outputIndex, Vec2I const& inputPosition, size_t inputIndex);

  auto getTileProtection(DungeonId dungeonId) const -> bool;
  void setTileProtection(DungeonId dungeonId, bool isProtected);
  // sets a provided list of DungeonIds all at once and returns how many were changed
  auto setTileProtection(List<DungeonId> const& dungeonIds, bool isProtected) -> size_t;
  // used to globally, temporarily disable protection for certain operations
  void setTileProtectionEnabled(bool enabled);

  void setDungeonGravity(DungeonId dungeonId, std::optional<float> gravity);
  void setDungeonBreathable(DungeonId dungeonId, std::optional<bool> breathable);

  void setDungeonId(RectI const& tileRegion, DungeonId dungeonId);

  // Signal a region to load / generate, returns true if it is now fully loaded
  // and generated
  auto signalRegion(RectI const& region) -> bool;
  // Immediately generate a given region
  void generateRegion(RectI const& region);
  // Returns true if a region is fully active without signaling it.
  auto regionActive(RectI const& region) -> bool;

  auto scriptContext(String const& contextName) -> ScriptComponentPtr;

  // Queues a microdungeon for placement
  auto enqueuePlacement(List<BiomeItemDistribution> distributions, std::optional<DungeonId> id) -> RpcPromise<Vec2I>;

  auto getServerTile(Vec2I const& position, bool withSignal = false) -> ServerTile const&;
  // Gets mutable pointer to server tile and marks it as needing updates to all
  // clients.
  auto modifyServerTile(Vec2I const& position, bool withSignal = false) -> ServerTile*;

  auto loadUniqueEntity(String const& uniqueId) -> EntityId;

  auto worldTemplate() const -> Ptr<WorldTemplate>;
  void setTemplate(Ptr<WorldTemplate> newTemplate);
  auto sky() const -> Ptr<Sky>;
  void modifyLiquid(Vec2I const& pos, LiquidId liquid, float quantity, bool additive = false);
  void setLiquid(Vec2I const& pos, LiquidId liquid, float level, float pressure);
  auto destroyBlock(TileLayer layer, Vec2I const& pos, bool genItems, bool destroyModFirst, bool updateNeighbors = true) -> List<ItemDescriptor>;
  void removeEntity(EntityId entityId, bool andDie);

  void updateTileEntityTiles(Ptr<TileEntity> const& object, bool removing = false, bool checkBreaks = true);

  auto isVisibleToPlayer(RectF const& region) const -> bool;
  void activateLiquidRegion(RectI const& region);
  void activateLiquidLocation(Vec2I const& location);

  // if blocks cascade, we'll need to do a break check across all tile entities
  // when the timer next ticks
  void requestGlobalBreakCheck();

  void setSpawningEnabled(bool spawningEnabled);

  void setPropertyListener(String const& propertyName, WorldPropertyListener listener);

  // Write all active sectors to disk without unloading them
  void sync();
  // Unload all sectors
  void unloadAll(bool force = false);
  // Copy full world to in memory representation
  auto readChunks() -> WorldChunks;

  auto forceModifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap) -> bool;
  auto forceApplyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) -> TileModificationList;

  auto dungeonId(Vec2I const& pos) const -> DungeonId;

  auto isPlayerModified(RectI const& region) const -> bool;

  auto collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId) -> ItemDescriptor;

  auto placeDungeon(String const& dungeonName, Vec2I const& position, std::optional<DungeonId> dungeonId = {}, bool forcePlacement = true) -> bool;

  void addBiomeRegion(Vec2I const& position, String const& biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  // queue generation of the sectors that will be needed to insert or
  // expand a biome region in order to spread processing over time
  auto pregenerateAddBiome(Vec2I const& position, int width) -> bool;
  auto pregenerateExpandBiome(Vec2I const& position, int newWidth) -> bool;

  // set the biome at the given position to be the environment biome for the layer
  void setLayerEnvironmentBiome(Vec2I const& position);

  // for terrestrial worlds only. updates the planet type in the celestial as well as local
  // world parameters along with the primary biome and the weather pool
  void setPlanetType(String const& planetType, String const& primaryBiomeName);

  // Force the current weather to a specific index from the world's weather pool
  void setWeatherIndex(size_t weatherIndex, bool force = false);
  // Force the current weather to a specific weather type by name
  void setWeather(String const& weatherName, bool force = false);

  // Returns the list of weather names available in this world
  auto weatherList() const -> StringList;

  // used to notify the universe server that the celestial planet type has changed
  auto pullNewPlanetType() -> std::optional<std::pair<String, String>>;

private:
  struct ClientInfo {
    ClientInfo(ConnectionId clientId, InterpolationTracker const trackerInit);

    auto monitoringRegions(Ptr<EntityMap> const& entityMap) const -> List<RectI>;

    auto needsDamageNotification(RemoteDamageNotification const& rdn) const -> bool;

    ConnectionId clientId;
    uint64_t skyNetVersion;
    uint64_t weatherNetVersion;
    WorldClientState clientState;
    bool pendingForward;
    bool started;
    bool local;
    bool admin;

    List<Ptr<Packet>> outgoingPackets;

    // All slave entities for which the player should be knowledgable about.
    HashMap<EntityId, uint64_t> clientSlavesNetVersion;

    // Batch send tile updates
    HashSet<Vec2I> pendingTileUpdates;
    HashSet<Vec2I> pendingLiquidUpdates;
    HashSet<std::pair<Vec2I, TileLayer>> pendingTileDamageUpdates;
    HashSet<ServerTileSectorArray::Sector> pendingSectors;
    HashSet<ServerTileSectorArray::Sector> activeSectors;

    InterpolationTracker interpolationTracker;
  };

  struct TileEntitySpaces {
    List<MaterialSpace> materials;
    List<Vec2I> roots;
  };

  using ServerTileGetter = std::function<ServerTile const&(Vec2I)>;

  void init(bool firstTime);

  // Returns nothing if the processing defined by the given configuration entry
  // should not run this tick, if it should run this tick, returns the number
  // of ticks since the last run.
  auto shouldRunThisStep(String const& timingConfiguration) -> std::optional<unsigned>;

  auto doApplyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap, bool ignoreTileProtection = false, bool updateNeighbors = true) -> TileModificationList;

  // Queues pending (step based) updates to the given player
  void queueUpdatePackets(ConnectionId clientId, bool sendRemoteUpdates);
  void updateDamage(float dt);

  void updateDamagedBlocks(float dt);

  // Check for any newly broken entities in this rect
  void checkEntityBreaks(RectF const& rect);
  // Push modified tile data to each client.
  void queueTileUpdates(Vec2I const& pos);
  void queueTileDamageUpdates(Vec2I const& pos, TileLayer layer);
  void writeNetTile(Vec2I const& pos, NetTile& netTile) const;

  void dirtyCollision(RectI const& region);
  void freshenCollision(RectI const& region);

  auto findPlayerStart(std::optional<Vec2F> firstTry = {}) -> Vec2F;
  auto findPlayerSpaceStart(float targetX) -> Vec2F;
  void readMetadata();
  void writeMetadata();
  auto gravityFromTile(ServerTile const& tile) const -> float;

  auto isFloatingDungeonWorld() const -> bool;

  void setupForceRegions();

  Json m_serverConfig;

  Ptr<WorldTemplate> m_worldTemplate;
  WorldStructure m_centralStructure;
  Vec2F m_playerStart;
  bool m_adjustPlayerStart;
  bool m_respawnInWorld;
  JsonObject m_worldProperties;
  StringMap<WorldPropertyListener> m_worldPropertyListeners;

  std::optional<std::pair<String, String>> m_newPlanetType;

  Ptr<UniverseSettings> m_universeSettings;

  Ptr<EntityMap> m_entityMap;
  Ptr<ServerTileSectorArray> m_tileArray;
  ServerTileGetter m_tileGetterFunction;
  Ptr<WorldStorage> m_worldStorage;
  WorldServerFidelity m_fidelity;
  Json m_fidelityConfig;

  HashSet<Vec2I> m_damagedBlocks;
  Ptr<DamageManager> m_damageManager;
  Ptr<WireProcessor> m_wireProcessor;
  Ptr<LuaRoot> m_luaRoot;

  StringMap<ScriptComponentPtr> m_scriptContexts;

  WorldGeometry m_geometry;
  double m_currentTime;
  uint64_t m_currentStep;
  mutable CellularLightIntensityCalculator m_lightIntensityCalculator;
  Ptr<Sky> m_sky;

  ServerWeather m_weather;

  CollisionGenerator m_collisionGenerator;
  List<CollisionBlock> m_workingCollisionBlocks;

  HashMap<NetCompatibilityRules, HashMap<std::pair<EntityId, uint64_t>, std::pair<ByteArray, uint64_t>>> m_netStateCache;
  OrderedHashMap<ConnectionId, std::shared_ptr<ClientInfo>> m_clientInfo;

  GameTimer m_entityUpdateTimer;
  GameTimer m_tileEntityBreakCheckTimer;

  std::shared_ptr<LiquidCellEngine<LiquidId>> m_liquidEngine;
  Ptr<FallingBlocksAgent> m_fallingBlocksAgent;
  Spawner m_spawner;

  // Keep track of material spaces and roots registered by tile entities to
  // make sure we can cleanly remove them when they change or when the entity
  // is removed / uninitialized
  HashMap<EntityId, TileEntitySpaces> m_tileEntitySpaces;

  List<std::pair<float, WorldAction>> m_timers;

  bool m_needsGlobalBreakCheck;

  bool m_generatingDungeon;
  HashMap<DungeonId, float> m_dungeonIdGravity;
  HashMap<DungeonId, bool> m_dungeonIdBreathable;
  StableHashSet<DungeonId> m_protectedDungeonIds;
  bool m_tileProtectionEnabled;

  HashMap<Uuid, std::pair<ConnectionId, MVariant<ConnectionId, RpcPromiseKeeper<Json>>>> m_entityMessageResponses;

  List<PhysicsForceRegion> m_forceRegions;

  String m_worldId;

  GameTimer m_expiryTimer;
};

}// namespace Star
