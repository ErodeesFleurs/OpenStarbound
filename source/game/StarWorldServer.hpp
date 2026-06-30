#pragma once

#include "StarAssets.hpp"
#include "StarBiomeDatabase.hpp"
#include "StarCellularLighting.hpp"
#include "StarCellularLiquid.hpp"
#include "StarCollisionGenerator.hpp"
#include "StarConfiguration.hpp"
#include "StarEntityFactory.hpp"
#include "StarInterpolationTracker.hpp"
#include "StarItemDatabase.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaRoot.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarMonsterDatabase.hpp"
#include "StarNetPackets.hpp"
#include "StarNpcDatabase.hpp"
#include "StarPlantDatabase.hpp"
#include "StarProjectileDatabase.hpp"
#include "StarRpcThreadPromise.hpp"
#include "StarSpawnTypeDatabase.hpp"
#include "StarSpawner.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarStagehandDatabase.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarTreasure.hpp"
#include "StarVehicleDatabase.hpp"
#include "StarVersioningDatabase.hpp"
#include "StarWarping.hpp"
#include "StarWeather.hpp"
#include "StarWorld.hpp"
#include "StarWorldClientState.hpp"
#include "StarWorldRenderData.hpp"
#include "StarWorldServerCollision.hpp"
#include "StarWorldServerDungeonProtection.hpp"
#include "StarWorldServerLiquid.hpp"
#include "StarWorldServerProperties.hpp"
#include "StarWorldServerSpawnFinder.hpp"
#include "StarWorldStructure.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class WorldTemplate;
using WorldTemplatePtr = SharedPtr<WorldTemplate>;
class DungeonDefinitions;
using DungeonDefinitionsConstPtr = SharedPtr<DungeonDefinitions const>;

// Aggregate of all service dependencies required by WorldServer.
// Construct via Root::makeWorldServerServices().
struct WorldServerServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  MaterialDatabaseConstPtr materialDatabase;
  ItemDatabaseConstPtr itemDatabase;
  ObjectDatabaseConstPtr objectDatabase;
  ProjectileDatabaseConstPtr projectileDatabase;
  PlantDatabaseConstPtr plantDatabase;
  TreasureDatabaseConstPtr treasureDatabase;
  NpcDatabaseConstPtr npcDatabase;
  MonsterDatabaseConstPtr monsterDatabase;
  SpawnTypeDatabaseConstPtr spawnTypeDatabase;
  StagehandDatabaseConstPtr stagehandDatabase;
  VehicleDatabaseConstPtr vehicleDatabase;
  SpeciesDatabaseConstPtr speciesDatabase;
  EntityFactoryConstPtr entityFactory;
  LiquidsDatabaseConstPtr liquidsDatabase;
  TerrainDatabaseConstPtr terrainDatabase;
  BiomeDatabaseConstPtr biomeDatabase;
  VersioningDatabaseConstPtr versioningDatabase;
  FunctionDatabaseConstPtr functionDatabase;
  EffectSourceDatabaseConstPtr effectSourceDatabase;
  ParticleDatabaseConstPtr particleDatabase;
  TechDatabaseConstPtr techDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  ImageMetadataDatabaseConstPtr imageMetadataDatabase;
  DungeonDefinitionsConstPtr dungeonDefinitions;
  BehaviorDatabaseConstPtr behaviorDatabase;
  LuaRootServices luaRootServices;
};
class Sky;
using SkyPtr = SharedPtr<Sky>;
struct SkyParameters;
class DamageManager;
using DamageManagerPtr = SharedPtr<DamageManager>;
class WireProcessor;
using WireProcessorPtr = SharedPtr<WireProcessor>;
class EntityMap;
using EntityMapPtr = SharedPtr<EntityMap>;
class WorldStorage;
using WorldStoragePtr = SharedPtr<WorldStorage>;
class FallingBlocksAgent;
using FallingBlocksAgentPtr = SharedPtr<FallingBlocksAgent>;
class DungeonDefinition;
class WorldServer;
using WorldServerPtr = SharedPtr<WorldServer>;
class TileEntity;
using TileEntityPtr = SharedPtr<TileEntity>;
class UniverseSettings;
using UniverseSettingsPtr = SharedPtr<UniverseSettings>;
class UniverseServer;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class VehicleDatabase;
using VehicleDatabaseConstPtr = SharedPtr<VehicleDatabase const>;

struct WorldServerExceptionTag {
  static constexpr char const* typeName = "WorldServerException";
};
using WorldServerException = TypedException<StarException, WorldServerExceptionTag>;

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
  using ScriptComponentPtr = shared_ptr<ScriptComponent>;
  using WorldPropertyListener = function<void(Json const&)>;

  // Create a new world with the given template, writing new storage file.
  WorldServer(WorldTemplatePtr const& worldTemplate,
              IODevicePtr storage,
              WorldServerServices services);
  // Synonym for WorldServer(make_shared<WorldTemplate>(services.assets, size), storage, services);
  WorldServer(Vec2U const& size,
              IODevicePtr storage,
              WorldServerServices services);
  // Load an existing world from the given storage files
  WorldServer(IODevicePtr const& storage,
              WorldServerServices services);
  // Load an existing world from the given in-memory chunks
  WorldServer(WorldChunks const& chunks,
              WorldServerServices services);
  // Load an existing world from an in-memory representation
  ~WorldServer();

  void setWorldId(String worldId);
  [[nodiscard]] String const& worldId() const;

  void setUniverseSettings(UniverseSettingsPtr universeSettings);
  [[nodiscard]] UniverseSettingsPtr universeSettings() const;
  [[nodiscard]] AssetsConstPtr assets() const override;
  [[nodiscard]] MaterialDatabaseConstPtr materialDatabase() const override;
  [[nodiscard]] ItemDatabaseConstPtr itemDatabase() const override;
  [[nodiscard]] ObjectDatabaseConstPtr objectDatabase() const override;
  [[nodiscard]] PlantDatabaseConstPtr plantDatabase() const override;
  [[nodiscard]] TreasureDatabaseConstPtr treasureDatabase() const override;
  [[nodiscard]] ImageMetadataDatabaseConstPtr imageMetadataDatabase() const override;
  [[nodiscard]] FunctionDatabaseConstPtr functionDatabase() const override;
  [[nodiscard]] BehaviorDatabaseConstPtr behaviorDatabase() const override;
  [[nodiscard]] NpcDatabaseConstPtr npcDatabase() const;
  [[nodiscard]] MonsterDatabaseConstPtr monsterDatabase() const;
  [[nodiscard]] ProjectileDatabaseConstPtr projectileDatabase() const override;
  [[nodiscard]] SpawnTypeDatabaseConstPtr spawnTypeDatabase() const;
  [[nodiscard]] StagehandDatabaseConstPtr stagehandDatabase() const;
  [[nodiscard]] VehicleDatabaseConstPtr vehicleDatabase() const;
  [[nodiscard]] TerrainDatabaseConstPtr terrainDatabase() const;
  [[nodiscard]] BiomeDatabaseConstPtr biomeDatabase() const;
  [[nodiscard]] DungeonDefinitionsConstPtr dungeonDefinitions() const;
  [[nodiscard]] LiquidsDatabaseConstPtr liquidsDatabase() const override;
  [[nodiscard]] EffectSourceDatabaseConstPtr effectSourceDatabase() const override;
  [[nodiscard]] ParticleDatabaseConstPtr particleDatabase() const override;
  [[nodiscard]] TechDatabaseConstPtr techDatabase() const override;
  [[nodiscard]] StatusEffectDatabaseConstPtr statusEffectDatabase() const override;

  void setPause(bool pause);
  void setReferenceClock(ClockPtr clock);

  void initLua(UniverseServer& universe);

  // Give this world a central structure.  If there is a previous central
  // structure it is removed first.  Returns the structure with transformed
  // coordinates.
  [[nodiscard]] WorldStructure setCentralStructure(WorldStructure centralStructure);
  [[nodiscard]] WorldStructure const& centralStructure() const;
  // If there is an active central structure, it is removed and all unmodified
  // objects and blocks associated with the structure are removed.
  void removeCentralStructure();

  void setPlayerStart(Vec2F const& startPosition, bool respawnInWorld = false);

  [[nodiscard]] bool spawnTargetValid(SpawnTarget const& spawnTarget) const;

  // Returns false if the client id already exists, or the spawn target is
  // invalid.
  [[nodiscard]] bool addClient(ConnectionId clientId, SpawnTarget const& spawnTarget, bool isLocal, bool isAdmin = false, NetCompatibilityRules netRules = {});

  // Removes client, sends the WorldStopPacket, and returns any pending packets
  // for that client
  [[nodiscard]] List<PacketPtr> removeClient(ConnectionId clientId);

  [[nodiscard]] List<ConnectionId> clientIds() const;
  [[nodiscard]] bool hasClient(ConnectionId clientId) const;
  [[nodiscard]] RectF clientWindow(ConnectionId clientId) const;
  // May return null if a Player is not available or if the client id is not
  // valid.
  [[nodiscard]] PlayerPtr clientPlayer(ConnectionId clientId) const;

  [[nodiscard]] List<EntityId> players() const;

  void handleIncomingPackets(ConnectionId clientId, List<PacketPtr> const& packets);
  [[nodiscard]] List<PacketPtr> getOutgoingPackets(ConnectionId clientId);
  [[nodiscard]] bool sendPacket(ConnectionId clientId, PacketPtr const& packet);

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId fromConnection, String const& message, JsonArray const& args);

  void startFlyingSky(bool enterHyperspace, bool startInWarp, Json settings = {});
  void stopFlyingSkyAt(SkyParameters const& destination);
  void setOrbitalSky(SkyParameters const& destination);

  // Defaults to Medium
  [[nodiscard]] WorldServerFidelity fidelity() const;
  void setFidelity(WorldServerFidelity fidelity);

  [[nodiscard]] bool shouldExpire();
  void setExpiryTime(float expiryTime);
  [[nodiscard]] float expiryTime();

  void update(float dt);

  [[nodiscard]] ConnectionId connection() const override;
  [[nodiscard]] WorldGeometry geometry() const override;
  [[nodiscard]] uint64_t currentStep() const override;
  [[nodiscard]] MaterialId material(Vec2I const& position, TileLayer layer) const override;
  std::tuple<MaterialId, ModId> materialAndMod(Vec2I const& position, TileLayer layer) const override;
  [[nodiscard]] MaterialHue materialHueShift(Vec2I const& position, TileLayer layer) const override;
  [[nodiscard]] ModId mod(Vec2I const& position, TileLayer layer) const override;
  [[nodiscard]] MaterialHue modHueShift(Vec2I const& position, TileLayer layer) const override;
  [[nodiscard]] MaterialColorVariant colorVariant(Vec2I const& position, TileLayer layer) const override;
  [[nodiscard]] LiquidLevel liquidLevel(Vec2I const& pos) const override;
  [[nodiscard]] LiquidLevel liquidLevel(RectF const& region) const override;

  [[nodiscard]] TileModificationList validTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) const override;
  [[nodiscard]] TileModificationList applyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) override;
  [[nodiscard]] bool replaceTile(Vec2I const& pos, TileModification const& modification, TileDamage const& tileDamage);
  [[nodiscard]] TileModificationList replaceTiles(TileModificationList const& modificationList, TileDamage const& tileDamage, bool applyDamage = false) override;
  [[nodiscard]] bool damageWouldDestroy(Vec2I const& pos, TileLayer layer, TileDamage const& tileDamage) const override;
  [[nodiscard]] EntityPtr entity(EntityId entityId) const override;
  void addEntity(EntityPtr const& entity, EntityId entityId = NullEntityId) override;
  [[nodiscard]] EntityPtr closestEntity(Vec2F const& center, float radius, EntityFilter selector = EntityFilter()) const override;
  void forAllEntities(EntityCallback entityCallback) const override;
  void forEachEntity(RectF const& boundBox, EntityCallback callback) const override;
  void forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback callback) const override;
  void forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> entityCallback) const override;
  [[nodiscard]] EntityPtr findEntity(RectF const& boundBox, EntityFilter entityFilter) const override;
  [[nodiscard]] EntityPtr findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter entityFilter) const override;
  [[nodiscard]] EntityPtr findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> entityFilter) const override;
  [[nodiscard]] bool tileIsOccupied(Vec2I const& pos, TileLayer layer, bool includeEphemeral = false, bool checkCollision = false) const override;
  [[nodiscard]] CollisionKind tileCollisionKind(Vec2I const& pos) const override;
  void forEachCollisionBlock(RectI const& region, function<void(CollisionBlock const&)> const& iterator) const override;
  [[nodiscard]] bool isTileConnectable(Vec2I const& pos, TileLayer layer, bool tilesOnly = false) const override;
  [[nodiscard]] bool pointTileCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  [[nodiscard]] bool lineTileCollision(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  [[nodiscard]] Maybe<pair<Vec2F, Vec2I>> lineTileCollisionPoint(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  [[nodiscard]] List<Vec2I> collidingTilesAlongLine(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet, int maxSize = -1, bool includeEdges = true) const override;
  [[nodiscard]] bool rectTileCollision(RectI const& region, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  [[nodiscard]] TileDamageResult damageTiles(List<Vec2I> const& pos, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, Maybe<EntityId> sourceEntity = {}) override;
  [[nodiscard]] InteractiveEntityPtr getInteractiveInRange(Vec2F const& targetPosition, Vec2F const& sourcePosition, float maxRange) const override;
  [[nodiscard]] bool canReachEntity(Vec2F const& position, float radius, EntityId targetEntity, bool preferInteractive = true) const override;
  [[nodiscard]] RpcPromise<InteractAction> interact(InteractRequest const& request) override;
  [[nodiscard]] float gravity(Vec2F const& pos) const override;
  [[nodiscard]] float windLevel(Vec2F const& pos) const override;
  [[nodiscard]] float lightLevel(Vec2F const& pos) const override;
  [[nodiscard]] bool breathable(Vec2F const& pos) const override;
  [[nodiscard]] float threatLevel() const override;
  [[nodiscard]] StringList environmentStatusEffects(Vec2F const& pos) const override;
  [[nodiscard]] StringList weatherStatusEffects(Vec2F const& pos) const override;
  [[nodiscard]] bool exposedToWeather(Vec2F const& pos) const override;
  [[nodiscard]] bool isUnderground(Vec2F const& pos) const override;
  [[nodiscard]] bool disableDeathDrops() const override;
  [[nodiscard]] List<PhysicsForceRegion> forceRegions() const override;
  [[nodiscard]] Json getProperty(String const& propertyName, Json const& def = Json()) const override;
  void setProperty(String const& propertyName, Json const& property) override;
  void timer(float delay, WorldAction worldAction) override;
  [[nodiscard]] double epochTime() const override;
  [[nodiscard]] uint32_t day() const override;
  [[nodiscard]] float dayLength() const override;
  [[nodiscard]] float timeOfDay() const override;
  [[nodiscard]] LuaRootPtr luaRoot() override;
  [[nodiscard]] RpcPromise<Vec2F> findUniqueEntity(String const& uniqueId) override;
  [[nodiscard]] RpcPromise<Json> sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) override;
  [[nodiscard]] bool isTileProtected(Vec2I const& pos) const override;
  void wire(Vec2I const& outputPosition, size_t outputIndex, Vec2I const& inputPosition, size_t inputIndex);

  [[nodiscard]] bool getTileProtection(DungeonId dungeonId) const;
  void setTileProtection(DungeonId dungeonId, bool isProtected);
  // sets a provided list of DungeonIds all at once and returns how many were changed
  [[nodiscard]] size_t setTileProtection(List<DungeonId> const& dungeonIds, bool isProtected);
  // used to globally, temporarily disable protection for certain operations
  void setTileProtectionEnabled(bool enabled);

  void setDungeonGravity(DungeonId dungeonId, Maybe<float> gravity);
  void setDungeonBreathable(DungeonId dungeonId, Maybe<bool> breathable);

  void setDungeonId(RectI const& tileRegion, DungeonId dungeonId);

  // Signal a region to load / generate, returns true if it is now fully loaded
  // and generated
  [[nodiscard]] bool signalRegion(RectI const& region);
  // Immediately generate a given region
  void generateRegion(RectI const& region);
  // Returns true if a region is fully active without signaling it.
  [[nodiscard]] bool regionActive(RectI const& region);

  [[nodiscard]] ScriptComponentPtr scriptContext(String const& contextName);

  // Queues a microdungeon for placement
  [[nodiscard]] RpcPromise<Vec2I> enqueuePlacement(List<BiomeItemDistribution> distributions, Maybe<DungeonId> id);

  [[nodiscard]] ServerTile const& getServerTile(Vec2I const& position, bool withSignal = false);
  // Gets mutable pointer to server tile and marks it as needing updates to all
  // clients.
  [[nodiscard]] ServerTile* modifyServerTile(Vec2I const& position, bool withSignal = false);

  [[nodiscard]] EntityId loadUniqueEntity(String const& uniqueId);

  [[nodiscard]] WorldTemplatePtr worldTemplate() const;
  void setTemplate(WorldTemplatePtr newTemplate);
  [[nodiscard]] SkyPtr sky() const;
  void modifyLiquid(Vec2I const& pos, LiquidId liquid, float quantity, bool additive = false);
  void setLiquid(Vec2I const& pos, LiquidId liquid, float level, float pressure);
  [[nodiscard]] List<ItemDescriptor> destroyBlock(TileLayer layer, Vec2I const& pos, bool genItems, bool destroyModFirst, bool updateNeighbors = true);
  void removeEntity(EntityId entityId, bool andDie);

  void updateTileEntityTiles(TileEntityPtr const& object, bool removing = false, bool checkBreaks = true);

  [[nodiscard]] bool isVisibleToPlayer(RectF const& region) const;
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
  [[nodiscard]] WorldChunks readChunks();

  [[nodiscard]] bool forceModifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap);
  [[nodiscard]] TileModificationList forceApplyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap);

  [[nodiscard]] DungeonId dungeonId(Vec2I const& pos) const;

  [[nodiscard]] bool isPlayerModified(RectI const& region) const;

  [[nodiscard]] ItemDescriptor collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId);

  [[nodiscard]] bool placeDungeon(String const& dungeonName, Vec2I const& position, Maybe<DungeonId> dungeonId = {}, bool forcePlacement = true);

  void addBiomeRegion(Vec2I const& position, String const& biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  // queue generation of the sectors that will be needed to insert or
  // expand a biome region in order to spread processing over time
  [[nodiscard]] bool pregenerateAddBiome(Vec2I const& position, int width);
  [[nodiscard]] bool pregenerateExpandBiome(Vec2I const& position, int newWidth);

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
  [[nodiscard]] StringList weatherList() const;

  // used to notify the universe server that the celestial planet type has changed
  [[nodiscard]] Maybe<pair<String, String>> pullNewPlanetType();

private:
  struct ClientInfo {
    ClientInfo(AssetsConstPtr assets, ConnectionId clientId, InterpolationTracker const trackerInit);

    [[nodiscard]] List<RectI> monitoringRegions(EntityMapPtr const& entityMap) const;

    [[nodiscard]] bool needsDamageNotification(RemoteDamageNotification const& rdn) const;

    ConnectionId clientId;
    uint64_t skyNetVersion;
    uint64_t weatherNetVersion;
    WorldClientState clientState;
    bool pendingForward;
    bool started;
    bool local;
    bool admin;

    List<PacketPtr> outgoingPackets;

    // All slave entities for which the player should be knowledgable about.
    HashMap<EntityId, uint64_t> clientSlavesNetVersion;

    // Batch send tile updates
    HashSet<Vec2I> pendingTileUpdates;
    HashSet<Vec2I> pendingLiquidUpdates;
    HashSet<pair<Vec2I, TileLayer>> pendingTileDamageUpdates;
    HashSet<ServerTileSectorArray::Sector> pendingSectors;
    HashSet<ServerTileSectorArray::Sector> activeSectors;

    InterpolationTracker interpolationTracker;
  };

  struct TileEntitySpaces {
    List<MaterialSpace> materials;
    List<Vec2I> roots;
  };

  using ServerTileGetter = function<ServerTile const&(Vec2I)>;

  void init(bool firstTime);

  // Returns nothing if the processing defined by the given configuration entry
  // should not run this tick, if it should run this tick, returns the number
  // of ticks since the last run.
  [[nodiscard]] Maybe<unsigned> shouldRunThisStep(String const& timingConfiguration);

  [[nodiscard]] TileModificationList doApplyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap, bool ignoreTileProtection = false, bool updateNeighbors = true);

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

  [[nodiscard]] Vec2F findPlayerStart(Maybe<Vec2F> firstTry = {});
  [[nodiscard]] Vec2F findPlayerSpaceStart(float targetX);
  void readMetadata();
  void writeMetadata();
  [[nodiscard]] float gravityFromTile(ServerTile const& tile) const;

  [[nodiscard]] bool isFloatingDungeonWorld() const;

  void setupForceRegions();

  Json m_serverConfig;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  LuaRootServices m_luaRootServices;
  MaterialDatabaseConstPtr m_materialDatabase;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  ProjectileDatabaseConstPtr m_projectileDatabase;
  PlantDatabaseConstPtr m_plantDatabase;
  TreasureDatabaseConstPtr m_treasureDatabase;
  NpcDatabaseConstPtr m_npcDatabase;
  MonsterDatabaseConstPtr m_monsterDatabase;
  SpawnTypeDatabaseConstPtr m_spawnTypeDatabase;
  StagehandDatabaseConstPtr m_stagehandDatabase;
  VehicleDatabaseConstPtr m_vehicleDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  EntityFactoryConstPtr m_entityFactory;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  TerrainDatabaseConstPtr m_terrainDatabase;
  BiomeDatabaseConstPtr m_biomeDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;
  FunctionDatabaseConstPtr m_functionDatabase;
  BehaviorDatabaseConstPtr m_behaviorDatabase;
  EffectSourceDatabaseConstPtr m_effectSourceDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  TechDatabaseConstPtr m_techDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  DungeonDefinitionsConstPtr m_dungeonDefinitions;

  friend class WorldServerCollision;
  friend class WorldServerDungeonProtection;
  friend class WorldServerLiquid;
  friend class WorldServerSpawnFinder;

  WorldTemplatePtr m_worldTemplate;
  WorldStructure m_centralStructure;
  WorldServerSpawnFinder m_spawnFinder{*this};
  WorldServerProperties m_worldProperties;

  struct NewPlanetType {
    String planetType;
    String primaryBiomeName;
  };

  Maybe<NewPlanetType> m_newPlanetType;

  UniverseSettingsPtr m_universeSettings;

  EntityMapPtr m_entityMap;
  ServerTileSectorArrayPtr m_tileArray;
  ServerTileGetter m_tileGetterFunction;
  WorldStoragePtr m_worldStorage;
  WorldServerFidelity m_fidelity;
  Json m_fidelityConfig;

  HashSet<Vec2I> m_damagedBlocks;
  DamageManagerPtr m_damageManager;
  WireProcessorPtr m_wireProcessor;
  LuaRootPtr m_luaRoot;

  StringMap<ScriptComponentPtr> m_scriptContexts;

  WorldGeometry m_geometry;
  double m_currentTime;
  uint64_t m_currentStep;
  mutable CellularLightIntensityCalculator m_lightIntensityCalculator;
  SkyPtr m_sky;

  ServerWeather m_weather;

  ClockPtr m_referenceClock;

  WorldServerCollision m_collision{*this};

  HashMap<NetCompatibilityRules, HashMap<pair<EntityId, uint64_t>, pair<ByteArray, uint64_t>>> m_netStateCache;
  OrderedHashMap<ConnectionId, shared_ptr<ClientInfo>> m_clientInfo;

  GameTimer m_entityUpdateTimer;
  GameTimer m_tileEntityBreakCheckTimer;

  WorldServerLiquid m_liquid{*this};
  FallingBlocksAgentPtr m_fallingBlocksAgent;
  Spawner m_spawner;

  // Keep track of material spaces and roots registered by tile entities to
  // make sure we can cleanly remove them when they change or when the entity
  // is removed / uninitialized
  HashMap<EntityId, TileEntitySpaces> m_tileEntitySpaces;

  struct WorldTimer {
    float remainingTime;
    WorldAction action;
  };
  List<WorldTimer> m_timers;

  bool m_needsGlobalBreakCheck;

  WorldServerDungeonProtection m_dungeonProtection{*this};

  HashMap<Uuid, pair<ConnectionId, MVariant<ConnectionId, RpcPromiseKeeper<Json>>>> m_entityMessageResponses;

  List<PhysicsForceRegion> m_forceRegions;

  String m_worldId;

  GameTimer m_expiryTimer;
};

}// namespace Star
