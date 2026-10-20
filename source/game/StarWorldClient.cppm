module;
import star.type_identity;
import star.xx_hash;
import star.json;
import star.id_map;
import star.game_types;
import star.material_types;
import star.image;
import star.list;
import star.bi_map;
import star.vector;
import star.lua;
import star.format;
import star.version;
import star.color;
import star.string;
import star.data_stream;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.either;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.casting;
import star.strong_typedef;
import star.audio;
import star.map;
import star.set;
import star.variant;
import star.rpc_promise;
import star.ordered_map;
import star.net_compatibility;
import std;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.data_stream_devices;
import star.btree_database;
import star.ordered_set;

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.warping;
import star.world_geometry;
import star.wiring;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.lua_root;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.mixer;
import star.entity_rendering;
import star.world;
import star.lua_components;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;

namespace Star {

class Root;

struct Biome;
class WorldTemplate;
class TileDrawer;
class Sky;
class Parallax;
class LuaRoot;
class DamageManager;
class EntityMap;
class ParticleManager;
class UniverseClient;
class Player;
class Item;
class CelestialLog;
class ClientContext;
class PlayerStorage;
struct OverheadBar;

struct WorldClientExceptionTag {
  static constexpr char const* name() { return "WorldClientException"; }
};
using WorldClientException = StarError<WorldClientExceptionTag, StarException>;

class WorldClient : public World {
public:
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<WorldClient, World>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<WorldClient>(); }

  typedef LuaMessageHandlingComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> ScriptComponent;
  
  WorldClient(Root& root, std::shared_ptr<Player> mainPlayer, std::shared_ptr<LuaRoot> luaRoot, UniverseClient* universe);
  WorldClient(Root& root, ClientSubWorldId subWorldId, UniverseClient* universe);
  ~WorldClient();

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
  TileModificationList replaceTiles(TileModificationList const& modificationList, TileDamage const& tileDamage, bool applyDamage = false) override;
  bool damageWouldDestroy(Vec2I const& pos, TileLayer layer, TileDamage const& tileDamage) const override;
  std::shared_ptr<Entity> entity(EntityId entityId) const override;
  void addEntity(std::shared_ptr<Entity> const& entity, EntityId entityId = NullEntityId) override;
  std::shared_ptr<Entity> closestEntity(Vec2F const& center, float radius, EntityFilter selector = EntityFilter()) const override;
  void forAllEntities(EntityCallback entityCallback) const override;
  void forEachEntity(RectF const& boundBox, EntityCallback callback) const override;
  void forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback callback) const override;
  void forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> entityCallback) const override;
  std::shared_ptr<Entity> findEntity(RectF const& boundBox, EntityFilter entityFilter) const override;
  std::shared_ptr<Entity> findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter entityFilter) const override;
  std::shared_ptr<Entity> findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> entityFilter) const override;
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
  std::shared_ptr<InteractiveEntity> getInteractiveInRange(Vec2F const& targetPosition, Vec2F const& sourcePosition, float maxRange) const override;
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
  std::shared_ptr<LuaRoot> luaRoot() override;
  RpcPromise<Vec2F> findUniqueEntity(String const& uniqueId) override;
  RpcPromise<Json> sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) override;
  bool isTileProtected(Vec2I const& pos) const override;
  StringMap<LuaCallbacks> luaThreadCallbacks() const override;

  // Is this WorldClient properly initialized in a world
  bool inWorld() const;
  
  bool isHeadless() const;
  ClientSubWorldId subWorldId() const;

  bool inSpace() const;
  bool flying() const;

  bool mainPlayerDead() const;
  void reviveMainPlayer();
  bool respawnInWorld() const;
  void setRespawnInWorld(bool respawnInWorld);

  std::int64_t latency() const;

  void resendEntity(EntityId entityId);
  void removeEntity(EntityId entityId, bool andDie);

  std::shared_ptr<WorldTemplate const> currentTemplate() const;
  void setTemplate(Json newTemplate);
  std::shared_ptr<Sky const> currentSky() const;

  void dimWorld();
  bool interactiveHighlightMode() const;
  void setInteractiveHighlightMode(bool enabled);
  void setParallax(std::shared_ptr<Parallax> newParallax);
  void overrideGravity(float gravity);
  void resetGravity();

  List<EntityId> entityIds() const;

  // Disable normal client-side lighting algorithm, everything full brightness.
  bool fullBright() const;
  void setFullBright(bool fullBright);
  // Disable asynchronous client-side lighting algorithm, run on main thread.
  bool asyncLighting() const;
  void setAsyncLighting(bool asyncLighting);
  // Spatial log generated collision geometry.
  bool collisionDebug() const;
  void setCollisionDebug(bool collisionDebug);

  void handleIncomingPackets(List<std::shared_ptr<Packet>> const& packets);
  List<std::shared_ptr<Packet>> getOutgoingPackets();

  // Set the rendering window for this client.
  void setClientWindow(RectI window);
  // Sets the client window around the position of the main player.
  void centerClientWindowOnPlayer(Vec2U const& windowSize);
  void centerClientWindowOnPlayer();
  RectI clientWindow() const;
  WorldClientState& clientState();

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId fromConnection, String const& message, JsonArray const& args);

  std::shared_ptr<WorldClient::ScriptComponent> scriptContext(String const& contextName);

  void update(float dt);
  // borderTiles here should extend the client window for border tile
  // calculations.  It is not necessary on the light array.
  void render(WorldRenderData& renderData, unsigned borderTiles, TileDrawer* tileDrawer);
  List<std::shared_ptr<AudioInstance>> pullPendingAudio();
  List<std::shared_ptr<AudioInstance>> pullPendingMusic();

  bool playerCanReachEntity(EntityId entityId, bool preferInteractive = true) const;

  void disconnectAllWires(Vec2I wireEntityPosition, WireNode const& node);
  void wire(Vec2I const& outputPosition, std::size_t outputIndex, Vec2I const& inputPosition, std::size_t inputIndex);
  void connectWire(WireConnection const& output, WireConnection const& input);

  // Functions for sending broadcast messages to other players that can receive them,
  // on completely vanilla servers by smuggling it through a DamageNotification.
  // It's cursed as fuck, but it works.
  bool sendSecretBroadcast(StringView broadcast, bool raw = false, bool compress = true);
  bool handleSecretBroadcast(std::shared_ptr<Player> player, StringView broadcast);

  List<ChatAction> pullPendingChatActions();

  WorldStructure const& centralStructure() const;

  DungeonId dungeonId(Vec2I const& pos) const;

  void collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId);

  bool waitForLighting(WorldRenderData* renderData = nullptr);

  typedef std::function<bool(std::shared_ptr<Player>, StringView)> BroadcastCallback;
  BroadcastCallback& broadcastCallback();
  
  bool shouldExpire();
  
  bool pullRequestedDestroy();
  void requestDestroy();

private:
  static const float DropDist;

  struct ClientRenderCallback : RenderCallback {
    explicit ClientRenderCallback(TileDrawer* drawer) : m_tileDrawer(drawer) {}
    TileDrawer* tileDrawer() const override { return m_tileDrawer; }
    TileDrawer* m_tileDrawer;
    void addDrawable(Drawable drawable, EntityRenderLayer renderLayer) override;
    void addLightSource(LightSource lightSource) override;
    void addParticle(Particle particle) override;
    void addAudio(std::shared_ptr<AudioInstance> audio) override;
    void addTilePreview(PreviewTile preview) override;
    void addOverheadBar(OverheadBar bar) override;

    Map<EntityRenderLayer, List<Drawable>> drawables;
    List<LightSource> lightSources;
    List<Particle> particles;
    List<std::shared_ptr<AudioInstance>> audios;
    List<PreviewTile> previewTiles;
    List<OverheadBar> overheadBars;
  };

  struct DamageNumber {
    float amount;
    Vec2F position;
    double timestamp;
  };

  struct DamageNumberKey {
    String damageNumberParticleKind;
    EntityId sourceEntityId;
    EntityId targetEntityId;

    bool operator<(DamageNumberKey const& other) const;
  };

  typedef std::function<ClientTile const& (Vec2I)> ClientTileGetter;

  void lightingTileGather();
  void lightingCalc();
  void lightingMain();

  void initWorld(WorldStartPacket const& packet);
  void clearWorld();
  void tryGiveMainPlayerItem(std::shared_ptr<Item> item, bool silent = false);

  void notifyEntityCreate(std::shared_ptr<Entity> const& entity);

  // Queues pending (step based) updates to server,
  void queueUpdatePackets(bool sendEntityUpdates);
  void handleDamageNotifications();

  void sparkDamagedBlocks();

  Vec2I environmentBiomeTrackPosition() const;
  Maybe<String> currentWeatherDomain() const;
  std::shared_ptr<AmbientNoisesDescription> currentAmbientNoises() const;
  std::shared_ptr<WeatherNoisesDescription> currentWeatherNoises() const;
  std::shared_ptr<AmbientNoisesDescription> currentMusicTrack() const;
  std::shared_ptr<AmbientNoisesDescription> currentAltMusicTrack() const;

  void playAltMusic(StringList const& newTracks, float fadeTime, int loops = -1);
  void stopAltMusic(float fadeTime);

  std::shared_ptr<Biome const> mainEnvironmentBiome() const;

  // Populates foregroundTransparent / backgroundTransparent flag on ClientTile
  // based on transparency rules.
  bool readNetTile(Vec2I const& pos, NetTile const& netTile, bool updateCollision = true);
  void dirtyCollision(RectI const& region);
  void freshenCollision(RectI const& region);
  void renderCollisionDebug();

  void informTilePrediction(Vec2I const& pos, TileModification const& modification);

  void setTileProtection(DungeonId dungeonId, bool isProtected);

  void setupForceRegions();

  Json m_clientConfig;
  std::shared_ptr<WorldTemplate> m_worldTemplate;
  WorldStructure m_centralStructure;
  Vec2F m_playerStart;
  bool m_respawnInWorld = false;
  JsonObject m_worldProperties;

  std::shared_ptr<EntityMap> m_entityMap;
  std::shared_ptr<ClientTileSectorArray> m_tileArray;
  ClientTileGetter m_tileGetterFunction;
  std::shared_ptr<DamageManager> m_damageManager;
  std::shared_ptr<LuaRoot> m_luaRoot;

  WorldGeometry m_geometry;
  std::uint64_t m_currentStep;
  double m_currentTime;
  bool m_fullBright = false;
  bool m_asyncLighting = false;
  CellularLightingCalculator m_lightingCalculator;
  mutable CellularLightIntensityCalculator m_lightIntensityCalculator;
  ThreadFunction<void> m_lightingThread;
  
  Mutex m_lightingMutex;
  ConditionVariable m_lightingCond;
  std::atomic<bool> m_stopLightingThread;

  Mutex m_lightMapPrepMutex;
  Mutex m_lightMapMutex;

  Lightmap m_pendingLightMap;
  Lightmap m_lightMap;
  List<LightSource> m_pendingLights;
  List<std::pair<Vec2F, Vec3F>> m_pendingParticleLights;
  RectI m_pendingLightRange;
  std::atomic<bool> m_pendingLightReady;
  Vec2I m_lightMinPosition;
  List<PreviewTile> m_previewTiles;

  std::shared_ptr<Sky> m_sky;

  CollisionGenerator m_collisionGenerator;

  WorldClientState m_clientState;
  Maybe<ConnectionId> m_clientId;

  std::shared_ptr<Player> m_mainPlayer;

  bool m_collisionDebug = false;

  // Client side entity updates are not done until m_inWorld is true, which is
  // set to true after we have entered a world *and* the first batch of updates
  // are received.
  bool m_inWorld = false;
  
  bool m_headless = false;
  
  ClientSubWorldId m_subWorldId;
  bool m_requestedDestroy = false;

  GameTimer m_worldDimTimer;
  float m_worldDimLevel;
  Vec3B m_worldDimColor;

  bool m_interactiveHighlightMode = false;

  GameTimer m_parallaxFadeTimer;
  std::shared_ptr<Parallax> m_currentParallax;
  std::shared_ptr<Parallax> m_nextParallax;
  Maybe<String> m_weatherParallaxAsset;
  std::shared_ptr<Biome const> m_weatherParallaxBiome;
  std::shared_ptr<Parallax> m_weatherParallax;
  Maybe<String> m_weatherDomain;
  Maybe<double> m_lastParallaxWindEpoch;
  double m_parallaxWindDirectionTime = 0.0;
  double m_parallaxWindMagnitudeTime = 0.0;
  double m_parallaxSignedWindTime = 0.0;

  Maybe<float> m_overrideGravity;

  ClientWeather m_weather;
  std::shared_ptr<ParticleManager> m_particles;

  List<std::shared_ptr<AudioInstance>> m_samples;
  List<std::shared_ptr<AudioInstance>> m_music;

  HashMap<EntityId, std::uint64_t> m_masterEntitiesNetVersion;

  InterpolationTracker m_interpolationTracker;
  GameTimer m_entityUpdateTimer;

  List<std::shared_ptr<Packet>> m_outgoingPackets;
  Maybe<std::int64_t> m_pingTime;
  std::int64_t m_latency;

  Set<EntityId> m_requestedDrops;

  Particle m_blockDamageParticle;
  Particle m_blockDamageParticleVariance;
  float m_blockDamageParticleProbability;

  Particle m_blockDingParticle;
  Particle m_blockDingParticleVariance;
  float m_blockDingParticleProbability;

  HashSet<Vec2I> m_damagedBlocks;

  AmbientManager m_ambientSounds;
  AmbientManager m_musicTrack;
  AmbientManager m_altMusicTrack;

  List<std::pair<float, WorldAction>> m_timers;

  Map<DamageNumberKey, DamageNumber> m_damageNumbers;
  float m_damageNotificationBatchDuration;

  std::shared_ptr<AudioInstance> m_spaceSound;
  String m_activeSpaceSound;

  std::shared_ptr<AmbientNoisesDescription> m_altMusicTrackDescription;
  bool m_altMusicActive = false;

  int m_modifiedTilePredictionTimeout;
  HashMap<Vec2I, PredictedTile> m_predictedTiles;
  HashSet<EntityId> m_startupHiddenEntities;

  HashMap<DungeonId, float> m_dungeonIdGravity;
  HashMap<DungeonId, bool> m_dungeonIdBreathable;
  StableHashSet<DungeonId> m_protectedDungeonIds;

  HashMap<String, List<RpcPromiseKeeper<Vec2F>>> m_findUniqueEntityResponses;
  HashMap<Uuid, RpcPromiseKeeper<Json>> m_entityMessageResponses;
  HashMap<Uuid, RpcPromise<Json>> m_entityMessagePromises;
  HashMap<Uuid, RpcPromiseKeeper<InteractAction>> m_entityInteractionResponses;

  List<PhysicsForceRegion> m_forceRegions;

  BroadcastCallback m_broadcastCallback;

  // used to keep track of already-printed stack traces caused by remote entities, so they don't clog the log
  HashSet<std::uint64_t> m_entityExceptionsLogged;
  
  StringMap<std::shared_ptr<WorldClient::ScriptComponent>> m_scriptContexts;

  GameTimer m_expiryTimer;
  
  UniverseClient* m_universe;
  
  StringMap<LuaCallbacks> m_luaThreadCallbacks;
};

}

export module star.world_client;

export namespace Star {
  using ::Star::Biome;
  using ::Star::WorldTemplate;
  using ::Star::Sky;
  using ::Star::Parallax;
  using ::Star::LuaRoot;
  using ::Star::DamageManager;
  using ::Star::EntityMap;
  using ::Star::ParticleManager;
  using ::Star::WorldClient;
  using ::Star::UniverseClient;
  using ::Star::Player;
  using ::Star::Item;
  using ::Star::CelestialLog;
  using ::Star::ClientContext;
  using ::Star::PlayerStorage;
  using ::Star::OverheadBar;
  using ::Star::WorldClientExceptionTag;
  using ::Star::WorldClientException;
}
