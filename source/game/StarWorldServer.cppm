module;
import star.json;
import star.xx_hash;
import star.id_map;
import star.list;
import star.bi_map;
import star.variant;
#include "StarCasting.hpp"
import star.either;
import star.rect;
import star.image;
import star.color;
import star.vector;
import star.thread;
import star.game_types;
import star.material_types;
import star.io_device;
import star.lua;
import star.format;
import star.version;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.maybe;
import star.random;
import star.weighted_pool;
#include "StarStrongTypedef.hpp"
import star.rpc_promise;
import star.set;
import star.worker_pool;

import std;
import star.sector_array_2d;
import star.ordered_map;
import star.net_compatibility;
import star.perlin;
#include "StarBTree.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
import star.ordered_set;
import star.map;

#include "StarLuaRoot.hpp"
import star.tile_sector_array;
import star.collision_block;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.liquid_types;
import star.tile_damage;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
#include "StarLuaComponents.hpp"
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;

import star.byte_array;
namespace Star {

class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<const Player>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<const Player>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<const Player>;
class WorldTemplate;
using WorldTemplatePtr = std::shared_ptr<WorldTemplate>;
using WorldTemplateConstPtr = std::shared_ptr<const WorldTemplate>;
using WorldTemplateWeakPtr = std::weak_ptr<WorldTemplate>;
using WorldTemplateConstWeakPtr = std::weak_ptr<const WorldTemplate>;
using WorldTemplateUPtr = std::unique_ptr<WorldTemplate>;
using WorldTemplateConstUPtr = std::unique_ptr<const WorldTemplate>;
class Sky;
using SkyPtr = std::shared_ptr<Sky>;
using SkyConstPtr = std::shared_ptr<const Sky>;
using SkyWeakPtr = std::weak_ptr<Sky>;
using SkyConstWeakPtr = std::weak_ptr<const Sky>;
using SkyUPtr = std::unique_ptr<Sky>;
using SkyConstUPtr = std::unique_ptr<const Sky>;
class ServerWeather;
using ServerWeatherPtr = std::shared_ptr<ServerWeather>;
using ServerWeatherConstPtr = std::shared_ptr<const ServerWeather>;
using ServerWeatherWeakPtr = std::weak_ptr<ServerWeather>;
using ServerWeatherConstWeakPtr = std::weak_ptr<const ServerWeather>;
using ServerWeatherUPtr = std::unique_ptr<ServerWeather>;
using ServerWeatherConstUPtr = std::unique_ptr<const ServerWeather>;
struct SkyParameters;
using SkyParametersPtr = std::shared_ptr<SkyParameters>;
using SkyParametersConstPtr = std::shared_ptr<const SkyParameters>;
using SkyParametersWeakPtr = std::weak_ptr<SkyParameters>;
using SkyParametersConstWeakPtr = std::weak_ptr<const SkyParameters>;
using SkyParametersUPtr = std::unique_ptr<SkyParameters>;
using SkyParametersConstUPtr = std::unique_ptr<const SkyParameters>;
class DamageManager;
using DamageManagerPtr = std::shared_ptr<DamageManager>;
using DamageManagerConstPtr = std::shared_ptr<const DamageManager>;
using DamageManagerWeakPtr = std::weak_ptr<DamageManager>;
using DamageManagerConstWeakPtr = std::weak_ptr<const DamageManager>;
using DamageManagerUPtr = std::unique_ptr<DamageManager>;
using DamageManagerConstUPtr = std::unique_ptr<const DamageManager>;
class WireProcessor;
using WireProcessorPtr = std::shared_ptr<WireProcessor>;
using WireProcessorConstPtr = std::shared_ptr<const WireProcessor>;
using WireProcessorWeakPtr = std::weak_ptr<WireProcessor>;
using WireProcessorConstWeakPtr = std::weak_ptr<const WireProcessor>;
using WireProcessorUPtr = std::unique_ptr<WireProcessor>;
using WireProcessorConstUPtr = std::unique_ptr<const WireProcessor>;
class EntityMap;
using EntityMapPtr = std::shared_ptr<EntityMap>;
using EntityMapConstPtr = std::shared_ptr<const EntityMap>;
using EntityMapWeakPtr = std::weak_ptr<EntityMap>;
using EntityMapConstWeakPtr = std::weak_ptr<const EntityMap>;
using EntityMapUPtr = std::unique_ptr<EntityMap>;
using EntityMapConstUPtr = std::unique_ptr<const EntityMap>;
class WorldStorage;
using WorldStoragePtr = std::shared_ptr<WorldStorage>;
using WorldStorageConstPtr = std::shared_ptr<const WorldStorage>;
using WorldStorageWeakPtr = std::weak_ptr<WorldStorage>;
using WorldStorageConstWeakPtr = std::weak_ptr<const WorldStorage>;
using WorldStorageUPtr = std::unique_ptr<WorldStorage>;
using WorldStorageConstUPtr = std::unique_ptr<const WorldStorage>;
class FallingBlocksAgent;
using FallingBlocksAgentPtr = std::shared_ptr<FallingBlocksAgent>;
using FallingBlocksAgentConstPtr = std::shared_ptr<const FallingBlocksAgent>;
using FallingBlocksAgentWeakPtr = std::weak_ptr<FallingBlocksAgent>;
using FallingBlocksAgentConstWeakPtr = std::weak_ptr<const FallingBlocksAgent>;
using FallingBlocksAgentUPtr = std::unique_ptr<FallingBlocksAgent>;
using FallingBlocksAgentConstUPtr = std::unique_ptr<const FallingBlocksAgent>;
class DungeonDefinition;
using DungeonDefinitionPtr = std::shared_ptr<DungeonDefinition>;
using DungeonDefinitionConstPtr = std::shared_ptr<const DungeonDefinition>;
using DungeonDefinitionWeakPtr = std::weak_ptr<DungeonDefinition>;
using DungeonDefinitionConstWeakPtr = std::weak_ptr<const DungeonDefinition>;
using DungeonDefinitionUPtr = std::unique_ptr<DungeonDefinition>;
using DungeonDefinitionConstUPtr = std::unique_ptr<const DungeonDefinition>;
class WorldServer;
using WorldServerPtr = std::shared_ptr<WorldServer>;
using WorldServerConstPtr = std::shared_ptr<const WorldServer>;
using WorldServerWeakPtr = std::weak_ptr<WorldServer>;
using WorldServerConstWeakPtr = std::weak_ptr<const WorldServer>;
using WorldServerUPtr = std::unique_ptr<WorldServer>;
using WorldServerConstUPtr = std::unique_ptr<const WorldServer>;
class TileEntity;
using TileEntityPtr = std::shared_ptr<TileEntity>;
using TileEntityConstPtr = std::shared_ptr<const TileEntity>;
using TileEntityWeakPtr = std::weak_ptr<TileEntity>;
using TileEntityConstWeakPtr = std::weak_ptr<const TileEntity>;
using TileEntityUPtr = std::unique_ptr<TileEntity>;
using TileEntityConstUPtr = std::unique_ptr<const TileEntity>;
class UniverseSettings;
using UniverseSettingsPtr = std::shared_ptr<UniverseSettings>;
using UniverseSettingsConstPtr = std::shared_ptr<const UniverseSettings>;
using UniverseSettingsWeakPtr = std::weak_ptr<UniverseSettings>;
using UniverseSettingsConstWeakPtr = std::weak_ptr<const UniverseSettings>;
using UniverseSettingsUPtr = std::unique_ptr<UniverseSettings>;
using UniverseSettingsConstUPtr = std::unique_ptr<const UniverseSettings>;
class UniverseServer;
using UniverseServerPtr = std::shared_ptr<UniverseServer>;
using UniverseServerConstPtr = std::shared_ptr<const UniverseServer>;
using UniverseServerWeakPtr = std::weak_ptr<UniverseServer>;
using UniverseServerConstWeakPtr = std::weak_ptr<const UniverseServer>;
using UniverseServerUPtr = std::unique_ptr<UniverseServer>;
using UniverseServerConstUPtr = std::unique_ptr<const UniverseServer>;

struct WorldServerExceptionTag {
  static constexpr char const* name() { return "WorldServerException"; }
};
using WorldServerException = StarError<WorldServerExceptionTag, StarException>;

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
  typedef LuaMessageHandlingComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> ScriptComponent;
  typedef std::shared_ptr<ScriptComponent> ScriptComponentPtr;
  typedef std::function<void(Json const&)> WorldPropertyListener;

  // Create a new world with the given template, writing new storage file.
  WorldServer(WorldTemplatePtr const& worldTemplate, IODevicePtr storage);
  // Synonym for WorldServer(make_shared<WorldTemplate>(size), storage);
  WorldServer(Vec2U const& size, IODevicePtr storage);
  // Load an existing world from the given storage files
  WorldServer(IODevicePtr const& storage);
  // Load an existing world from the given in-memory chunks
  WorldServer(WorldChunks const& chunks);
  // Load an existing world from an in-memory representation
  ~WorldServer();

  void setWorldId(String worldId);
  String const& worldId() const;

  void setUniverseSettings(UniverseSettingsPtr universeSettings);
  UniverseSettingsPtr universeSettings() const;

  void setPause(bool pause);
  void setReferenceClock(ClockPtr clock);

  void initLua(UniverseServer* universe);

  // Give this world a central structure.  If there is a previous central
  // structure it is removed first.  Returns the structure with transformed
  // coordinates.
  WorldStructure setCentralStructure(WorldStructure centralStructure);
  WorldStructure const& centralStructure() const;
  // If there is an active central structure, it is removed and all unmodified
  // objects and blocks associated with the structure are removed.
  void removeCentralStructure();

  void setPlayerStart(Vec2F const& startPosition, bool respawnInWorld = false);

  bool spawnTargetValid(SpawnTarget const& spawnTarget) const;

  // Returns false if the client id already exists, or the spawn target is
  // invalid.
  bool addClient(ConnectionId clientId, SpawnTarget const& spawnTarget, bool isLocal, bool isAdmin = false, NetCompatibilityRules netRules = {}, ClientSubWorldId subWorldId = MainClientWorldId);

  // Removes client, sends the WorldStopPacket, and returns any pending packets
  // for that client
  List<PacketPtr> removeClient(ConnectionId clientId);

  List<ConnectionId> clientIds() const;
  bool hasClient(ConnectionId clientId) const;
  RectF clientWindow(ConnectionId clientId) const;
  // May return null if a Player is not available or if the client id is not
  // valid.
  PlayerPtr clientPlayer(ConnectionId clientId) const;
  
  ClientSubWorldId clientSubWorld(ConnectionId clientId) const;

  List<EntityId> players() const;

  void handleIncomingPackets(ConnectionId clientId, List<PacketPtr> const& packets);
  List<PacketPtr> getOutgoingPackets(ConnectionId clientId);
  bool sendPacket(ConnectionId clientId, PacketPtr const& packet);

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId fromConnection, String const& message, JsonArray const& args);

  void startFlyingSky(bool enterHyperspace, bool startInWarp, Json settings = {});
  void stopFlyingSkyAt(SkyParameters const& destination);
  void setOrbitalSky(SkyParameters const& destination);

  // Defaults to Medium
  WorldServerFidelity fidelity() const;
  void setFidelity(WorldServerFidelity fidelity);

  bool shouldExpire();
  void setExpiryTime(float expiryTime);
  float expiryTime();

  void update(float dt);

  ConnectionId connection() const override;
  WorldGeometry geometry() const override;
  std::uint64_t currentStep() const override;
  MaterialId material(Vec2I const& position, TileLayer layer) const override;
  MaterialHue materialHueShift(Vec2I const& position, TileLayer layer) const override;
  ModId mod(Vec2I const& position, TileLayer layer) const override;
  MaterialHue modHueShift(Vec2I const& position, TileLayer layer) const override;
  MaterialColorVariant colorVariant(Vec2I const& position, TileLayer layer) const override;
  LiquidLevel liquidLevel(Vec2I const& pos) const override;
  LiquidLevel liquidLevel(RectF const& region) const override;

  TileModificationList validTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) const override;
  TileModificationList applyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) override;
  bool replaceTile(Vec2I const& pos, TileModification const& modification, TileDamage const& tileDamage);
  TileModificationList replaceTiles(TileModificationList const& modificationList, TileDamage const& tileDamage, bool applyDamage = false) override;
  bool damageWouldDestroy(Vec2I const& pos, TileLayer layer, TileDamage const& tileDamage) const override;
  EntityPtr entity(EntityId entityId) const override;
  void addEntity(EntityPtr const& entity, EntityId entityId = NullEntityId) override;
  EntityPtr closestEntity(Vec2F const& center, float radius, EntityFilter selector = EntityFilter()) const override;
  void forAllEntities(EntityCallback entityCallback) const override;
  void forEachEntity(RectF const& boundBox, EntityCallback callback) const override;
  void forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback callback) const override;
  void forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> entityCallback) const override;
  EntityPtr findEntity(RectF const& boundBox, EntityFilter entityFilter) const override;
  EntityPtr findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter entityFilter) const override;
  EntityPtr findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> entityFilter) const override;
  bool tileIsOccupied(Vec2I const& pos, TileLayer layer, bool includeEphemeral = false, bool checkCollision = false) const override;
  CollisionKind tileCollisionKind(Vec2I const& pos) const override;
  void forEachCollisionBlock(RectI const& region, std::function<void(CollisionBlock const&)> const& iterator) const override;
  bool isTileConnectable(Vec2I const& pos, TileLayer layer, bool tilesOnly = false) const override;
  bool pointTileCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  bool lineTileCollision(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  Maybe<std::pair<Vec2F, Vec2I>> lineTileCollisionPoint(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  List<Vec2I> collidingTilesAlongLine(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet, int maxSize = -1, bool includeEdges = true) const override;
  bool rectTileCollision(RectI const& region, CollisionSet const& collisionSet = DefaultCollisionSet) const override;
  TileDamageResult damageTiles(List<Vec2I> const& pos, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, Maybe<EntityId> sourceEntity = {}) override;
  InteractiveEntityPtr getInteractiveInRange(Vec2F const& targetPosition, Vec2F const& sourcePosition, float maxRange) const override;
  bool canReachEntity(Vec2F const& position, float radius, EntityId targetEntity, bool preferInteractive = true) const override;
  RpcPromise<InteractAction> interact(InteractRequest const& request) override;
  float gravity(Vec2F const& pos) const override;
  float windLevel(Vec2F const& pos) const override;
  float lightLevel(Vec2F const& pos) const override;
  bool breathable(Vec2F const& pos) const override;
  float threatLevel() const override;
  StringList environmentStatusEffects(Vec2F const& pos) const override;
  StringList weatherStatusEffects(Vec2F const& pos) const override;
  bool exposedToWeather(Vec2F const& pos) const override;
  Maybe<String> activeWeather(Vec2F const& pos) const override;
  bool isUnderground(Vec2F const& pos) const override;
  bool disableDeathDrops() const override;
  List<PhysicsForceRegion> forceRegions() const override;
  Json getProperty(String const& propertyName, Json const& def = Json()) const override;
  void setProperty(String const& propertyName, Json const& property) override;
  void timer(float delay, WorldAction worldAction) override;
  double epochTime() const override;
  std::uint32_t day() const override;
  float dayLength() const override;
  float timeOfDay() const override;
  LuaRootPtr luaRoot() override;
  RpcPromise<Vec2F> findUniqueEntity(String const& uniqueId) override;
  RpcPromise<Json> sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) override;
  bool isTileProtected(Vec2I const& pos) const override;
  
  StringMap<LuaCallbacks> luaThreadCallbacks() const override;
  
  void wire(Vec2I const& outputPosition, std::size_t outputIndex, Vec2I const& inputPosition, std::size_t inputIndex);

  bool getTileProtection(DungeonId dungeonId) const;
  void setTileProtection(DungeonId dungeonId, bool isProtected);
  // sets a provided list of DungeonIds all at once and returns how many were changed
  std::size_t setTileProtection(List<DungeonId> const& dungeonIds, bool isProtected);
  // used to globally, temporarily disable protection for certain operations
  void setTileProtectionEnabled(bool enabled);

  void setDungeonGravity(DungeonId dungeonId, Maybe<float> gravity);
  void setDungeonBreathable(DungeonId dungeonId, Maybe<bool> breathable);

  void setDungeonId(RectI const& tileRegion, DungeonId dungeonId);

  // Signal a region to load / generate, returns true if it is now fully loaded
  // and generated
  bool signalRegion(RectI const& region);
  // Immediately generate a given region
  void generateRegion(RectI const& region);
  // Returns true if a region is fully active without signaling it.
  bool regionActive(RectI const& region);

  ScriptComponentPtr scriptContext(String const& contextName);

  List<EntityId> entityIds() const;

  // Queues a microdungeon for placement
  RpcPromise<Vec2I> enqueuePlacement(List<BiomeItemDistribution> distributions, Maybe<DungeonId> id);

  ServerTile const& getServerTile(Vec2I const& position, bool withSignal = false);
  // Gets mutable pointer to server tile and marks it as needing updates to all
  // clients.
  ServerTile* modifyServerTile(Vec2I const& position, bool withSignal = false);

  EntityId loadUniqueEntity(String const& uniqueId);

  WorldTemplatePtr worldTemplate() const;
  void setTemplate(WorldTemplatePtr newTemplate);
  SkyPtr sky() const;
  void modifyLiquid(Vec2I const& pos, LiquidId liquid, float quantity, bool additive = false);
  void setLiquid(Vec2I const& pos, LiquidId liquid, float level, float pressure);
  List<ItemDescriptor> destroyBlock(TileLayer layer, Vec2I const& pos, bool genItems, bool destroyModFirst, bool updateNeighbors = true);
  void removeEntity(EntityId entityId, bool andDie);

  void updateTileEntityTiles(TileEntityPtr const& object, bool removing = false, bool checkBreaks = true);

  bool isVisibleToPlayer(RectF const& region) const;
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
  WorldChunks readChunks();

  bool forceModifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap);
  TileModificationList forceApplyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap);

  DungeonId dungeonId(Vec2I const& pos) const;

  bool isPlayerModified(RectI const& region) const;

  ItemDescriptor collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId);

  bool placeDungeon(String const& dungeonName, Vec2I const& position, Maybe<DungeonId> dungeonId = {}, bool forcePlacement = true);

  void addBiomeRegion(Vec2I const& position, String const& biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  // queue generation of the sectors that will be needed to insert or
  // expand a biome region in order to spread processing over time
  bool pregenerateAddBiome(Vec2I const& position, int width);
  bool pregenerateExpandBiome(Vec2I const& position, int newWidth);

  // set the biome at the given position to be the environment biome for the layer
  void setLayerEnvironmentBiome(Vec2I const& position);

  // for terrestrial worlds only. updates the planet type in the celestial as well as local
  // world parameters along with the primary biome and the weather pool
  void setPlanetType(String const& planetType, String const& primaryBiomeName);

  // Force the current weather to a specific index from the world's weather pool
  void setWeatherIndex(std::size_t weatherIndex, bool force = false);
  // Force the current weather to a specific weather type by name
  void setWeather(String const& weatherName, bool force = false);
  void setWeather(Vec2F const& position, String const& weatherName, bool force = false);

  // Returns the list of weather names available in this world
  StringList weatherList() const;
  StringList weatherList(Vec2F const& position) const;

  // used to notify the universe server that the celestial planet type has changed
  Maybe<std::pair<String, String>> pullNewPlanetType();

private:
  struct ClientInfo {
    ClientInfo(ConnectionId clientId, InterpolationTracker const trackerInit);

    List<RectI> monitoringRegions(EntityMapPtr const& entityMap) const;

    bool needsDamageNotification(RemoteDamageNotification const& rdn) const;

    ConnectionId clientId;
    ClientSubWorldId subWorldId;
    std::uint64_t skyNetVersion;
    std::uint64_t weatherNetVersion;
    Maybe<String> weatherDomain;
    WorldClientState clientState;
    bool pendingForward;
    bool started;
    bool local;
    bool admin;

    List<PacketPtr> outgoingPackets;

    // All slave entities for which the player should be knowledgable about.
    HashMap<EntityId, std::uint64_t> clientSlavesNetVersion;

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

  typedef std::function<ServerTile const& (Vec2I)> ServerTileGetter;

  void init(bool firstTime);

  // Returns nothing if the processing defined by the given configuration entry
  // should not run this tick, if it should run this tick, returns the number
  // of ticks since the last run.
  Maybe<unsigned> shouldRunThisStep(String const& timingConfiguration);

  TileModificationList doApplyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap, bool ignoreTileProtection = false, bool updateNeighbors = true);

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

  Vec2F findPlayerStart(Maybe<Vec2F> firstTry = {});
  Vec2F findPlayerSpaceStart(float targetX);
  void readMetadata();
  void writeMetadata();
  float gravityFromTile(ServerTile const& tile) const;

  bool isFloatingDungeonWorld() const;

  void setupForceRegions();
  void setupWeatherDomains(bool preserveNonSurface = false);
  ServerWeatherPtr weatherForDomain(Maybe<String> const& domain) const;
  ServerWeatherPtr weatherAt(Vec2F const& position) const;
  ServerWeatherPtr surfaceWeather() const;

  Json m_serverConfig;

  WorldTemplatePtr m_worldTemplate;
  WorldStructure m_centralStructure;
  Vec2F m_playerStart;
  bool m_adjustPlayerStart = false;
  bool m_respawnInWorld = false;
  JsonObject m_worldProperties;
  StringMap<WorldPropertyListener> m_worldPropertyListeners;

  Maybe<std::pair<String, String>> m_newPlanetType;

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
  std::uint64_t m_currentStep;
  mutable CellularLightIntensityCalculator m_lightIntensityCalculator;
  SkyPtr m_sky;

  StringMap<ServerWeatherPtr> m_weatherDomains;
  ServerWeatherPtr m_emptyWeather;

  ClockPtr m_referenceClock;

  CollisionGenerator m_collisionGenerator;
  List<CollisionBlock> m_workingCollisionBlocks;

  HashMap<NetCompatibilityRules, HashMap<std::pair<EntityId, std::uint64_t>, std::pair<ByteArray, std::uint64_t>>> m_netStateCache;
  OrderedHashMap<ConnectionId, std::shared_ptr<ClientInfo>> m_clientInfo;

  GameTimer m_entityUpdateTimer;
  GameTimer m_tileEntityBreakCheckTimer;

  std::shared_ptr<LiquidCellEngine<LiquidId>> m_liquidEngine;
  FallingBlocksAgentPtr m_fallingBlocksAgent;
  Spawner m_spawner;

  // Keep track of material spaces and roots registered by tile entities to
  // make sure we can cleanly remove them when they change or when the entity
  // is removed / uninitialized
  HashMap<EntityId, TileEntitySpaces> m_tileEntitySpaces;

  List<std::pair<float, WorldAction>> m_timers;

  bool m_needsGlobalBreakCheck = false;

  bool m_generatingDungeon = false;
  HashMap<DungeonId, float> m_dungeonIdGravity;
  HashMap<DungeonId, bool> m_dungeonIdBreathable;
  StableHashSet<DungeonId> m_protectedDungeonIds;
  bool m_tileProtectionEnabled = true;

  HashMap<Uuid, std::pair<ConnectionId, MVariant<ConnectionId, RpcPromiseKeeper<Json>>>> m_entityMessageResponses;
  HashMap<Uuid, std::pair<ConnectionId, RpcPromise<Json>>> m_entityMessagePromises;

  List<PhysicsForceRegion> m_forceRegions;

  String m_worldId;

  GameTimer m_expiryTimer;
  
  StringMap<LuaCallbacks> m_luaThreadCallbacks;
};

}

export module star.world_server;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::WorldTemplate;
  using ::Star::WorldTemplatePtr;
  using ::Star::WorldTemplateConstPtr;
  using ::Star::WorldTemplateWeakPtr;
  using ::Star::WorldTemplateConstWeakPtr;
  using ::Star::WorldTemplateUPtr;
  using ::Star::WorldTemplateConstUPtr;
  using ::Star::Sky;
  using ::Star::SkyPtr;
  using ::Star::SkyConstPtr;
  using ::Star::SkyWeakPtr;
  using ::Star::SkyConstWeakPtr;
  using ::Star::SkyUPtr;
  using ::Star::SkyConstUPtr;
  using ::Star::ServerWeather;
  using ::Star::ServerWeatherPtr;
  using ::Star::ServerWeatherConstPtr;
  using ::Star::ServerWeatherWeakPtr;
  using ::Star::ServerWeatherConstWeakPtr;
  using ::Star::ServerWeatherUPtr;
  using ::Star::ServerWeatherConstUPtr;
  using ::Star::SkyParameters;
  using ::Star::SkyParametersPtr;
  using ::Star::SkyParametersConstPtr;
  using ::Star::SkyParametersWeakPtr;
  using ::Star::SkyParametersConstWeakPtr;
  using ::Star::SkyParametersUPtr;
  using ::Star::SkyParametersConstUPtr;
  using ::Star::DamageManager;
  using ::Star::DamageManagerPtr;
  using ::Star::DamageManagerConstPtr;
  using ::Star::DamageManagerWeakPtr;
  using ::Star::DamageManagerConstWeakPtr;
  using ::Star::DamageManagerUPtr;
  using ::Star::DamageManagerConstUPtr;
  using ::Star::WireProcessor;
  using ::Star::WireProcessorPtr;
  using ::Star::WireProcessorConstPtr;
  using ::Star::WireProcessorWeakPtr;
  using ::Star::WireProcessorConstWeakPtr;
  using ::Star::WireProcessorUPtr;
  using ::Star::WireProcessorConstUPtr;
  using ::Star::EntityMap;
  using ::Star::EntityMapPtr;
  using ::Star::EntityMapConstPtr;
  using ::Star::EntityMapWeakPtr;
  using ::Star::EntityMapConstWeakPtr;
  using ::Star::EntityMapUPtr;
  using ::Star::EntityMapConstUPtr;
  using ::Star::WorldStorage;
  using ::Star::WorldStoragePtr;
  using ::Star::WorldStorageConstPtr;
  using ::Star::WorldStorageWeakPtr;
  using ::Star::WorldStorageConstWeakPtr;
  using ::Star::WorldStorageUPtr;
  using ::Star::WorldStorageConstUPtr;
  using ::Star::FallingBlocksAgent;
  using ::Star::FallingBlocksAgentPtr;
  using ::Star::FallingBlocksAgentConstPtr;
  using ::Star::FallingBlocksAgentWeakPtr;
  using ::Star::FallingBlocksAgentConstWeakPtr;
  using ::Star::FallingBlocksAgentUPtr;
  using ::Star::FallingBlocksAgentConstUPtr;
  using ::Star::DungeonDefinition;
  using ::Star::DungeonDefinitionPtr;
  using ::Star::DungeonDefinitionConstPtr;
  using ::Star::DungeonDefinitionWeakPtr;
  using ::Star::DungeonDefinitionConstWeakPtr;
  using ::Star::DungeonDefinitionUPtr;
  using ::Star::DungeonDefinitionConstUPtr;
  using ::Star::WorldServer;
  using ::Star::WorldServerPtr;
  using ::Star::WorldServerConstPtr;
  using ::Star::WorldServerWeakPtr;
  using ::Star::WorldServerConstWeakPtr;
  using ::Star::WorldServerUPtr;
  using ::Star::WorldServerConstUPtr;
  using ::Star::TileEntity;
  using ::Star::TileEntityPtr;
  using ::Star::TileEntityConstPtr;
  using ::Star::TileEntityWeakPtr;
  using ::Star::TileEntityConstWeakPtr;
  using ::Star::TileEntityUPtr;
  using ::Star::TileEntityConstUPtr;
  using ::Star::UniverseSettings;
  using ::Star::UniverseSettingsPtr;
  using ::Star::UniverseSettingsConstPtr;
  using ::Star::UniverseSettingsWeakPtr;
  using ::Star::UniverseSettingsConstWeakPtr;
  using ::Star::UniverseSettingsUPtr;
  using ::Star::UniverseSettingsConstUPtr;
  using ::Star::UniverseServer;
  using ::Star::UniverseServerPtr;
  using ::Star::UniverseServerConstPtr;
  using ::Star::UniverseServerWeakPtr;
  using ::Star::UniverseServerConstWeakPtr;
  using ::Star::UniverseServerUPtr;
  using ::Star::UniverseServerConstUPtr;
  using ::Star::WorldServerExceptionTag;
  using ::Star::WorldServerException;
  using ::Star::WorldServerFidelity;
  using ::Star::WorldServerFidelityNames;
}
