#pragma once

#include "StarAmbient.hpp"
#include "StarAssets.hpp"
#include "StarBiomeDatabase.hpp"
#include "StarCellularLighting.hpp"
#include "StarChatAction.hpp"
#include "StarConfiguration.hpp"
#include "StarDamageDatabase.hpp"
#include "StarDungeonGenerator.hpp"
#include "StarEffectSourceDatabase.hpp"
#include "StarEntityFactory.hpp"
#include "StarEntityRendering.hpp"
#include "StarGameTimers.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarInterpolationTracker.hpp"
#include "StarItemDatabase.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLuaRoot.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarNetPackets.hpp"
#include "StarParticleDatabase.hpp"
#include "StarPlantDatabase.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarStatusEffectDatabase.hpp"
#include "StarStoredFunctions.hpp"
#include "StarTechDatabase.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarTickRateMonitor.hpp"
#include "StarTreasure.hpp"
#include "StarWeather.hpp"
#include "StarWiring.hpp"
#include "StarWorld.hpp"
#include "StarWorldClientAudio.hpp"
#include "StarWorldClientDamageFX.hpp"
#include "StarWorldClientLighting.hpp"
#include "StarWorldClientState.hpp"
#include "StarWorldClientTilePrediction.hpp"
#include "StarWorldRenderData.hpp"
#include "StarWorldStructure.hpp"

namespace Star {

struct Biome;
using BiomeConstPtr = SharedPtr<Biome const>;
class WorldTemplate;
using WorldTemplatePtr = SharedPtr<WorldTemplate>;
using WorldTemplateConstPtr = SharedPtr<WorldTemplate const>;
class Sky;
using SkyPtr = SharedPtr<Sky>;
using SkyConstPtr = SharedPtr<Sky const>;
class Parallax;
using ParallaxPtr = SharedPtr<Parallax>;
class LuaRoot;
using LuaRootPtr = SharedPtr<LuaRoot>;
class DamageManager;
using DamageManagerPtr = SharedPtr<DamageManager>;
class EntityMap;
using EntityMapPtr = SharedPtr<EntityMap>;
class ParticleManager;
using ParticleManagerPtr = SharedPtr<ParticleManager>;
class WorldClient;
using WorldClientPtr = SharedPtr<WorldClient>;
class Player;
using PlayerPtr = SharedPtr<Player>;
class Item;
using ItemPtr = SharedPtr<Item>;
class CelestialLog;
class ClientContext;
class PlayerStorage;
struct OverheadBar;

struct WorldClientExceptionTag {
  static constexpr char const* typeName = "WorldClientException";
};
using WorldClientException = TypedException<StarException, WorldClientExceptionTag>;

class WorldClient : public World {
public:
  WorldClient(PlayerPtr mainPlayer,
              LuaRootPtr luaRoot,
              AssetsConstPtr assets,
              ConfigurationPtr configuration,
              MaterialDatabaseConstPtr materialDatabase,
              ItemDatabaseConstPtr itemDatabase,
              ObjectDatabaseConstPtr objectDatabase,
              SpeciesDatabaseConstPtr speciesDatabase,
              EntityFactoryConstPtr entityFactory,
              LiquidsDatabaseConstPtr liquidsDatabase,
              TerrainDatabaseConstPtr terrainDatabase,
              BiomeDatabaseConstPtr biomeDatabase,
              FunctionDatabaseConstPtr functionDatabase,
              BehaviorDatabaseConstPtr behaviorDatabase,
              ParticleDatabaseConstPtr particleDatabase,
              ProjectileDatabaseConstPtr projectileDatabase,
              DamageDatabaseConstPtr damageDatabase,
              EffectSourceDatabaseConstPtr effectSourceDatabase,
              TechDatabaseConstPtr techDatabase,
              StatusEffectDatabaseConstPtr statusEffectDatabase,
              PlantDatabaseConstPtr plantDatabase,
              TreasureDatabaseConstPtr treasureDatabase,
              ImageMetadataDatabaseConstPtr imageMetadataDatabase,
              DungeonDefinitionsConstPtr dungeonDefinitions);
  ~WorldClient();

  [[nodiscard]] ConnectionId connection() const override;
  [[nodiscard]] WorldGeometry geometry() const override;
  [[nodiscard]] uint64_t currentStep() const override;
  [[nodiscard]] AssetsConstPtr const& assets() const override;
  [[nodiscard]] ItemDatabaseConstPtr const& itemDatabase() const override;
  [[nodiscard]] ObjectDatabaseConstPtr const& objectDatabase() const override;
  [[nodiscard]] MaterialDatabaseConstPtr const& materialDatabase() const override;
  [[nodiscard]] LiquidsDatabaseConstPtr const& liquidsDatabase() const override;
  [[nodiscard]] ParticleDatabaseConstPtr const& particleDatabase() const override;
  [[nodiscard]] ProjectileDatabaseConstPtr const& projectileDatabase() const override;
  [[nodiscard]] EffectSourceDatabaseConstPtr const& effectSourceDatabase() const override;
  [[nodiscard]] TechDatabaseConstPtr const& techDatabase() const override;
  [[nodiscard]] StatusEffectDatabaseConstPtr const& statusEffectDatabase() const override;
  [[nodiscard]] PlantDatabaseConstPtr const& plantDatabase() const override;
  [[nodiscard]] TreasureDatabaseConstPtr const& treasureDatabase() const override;
  [[nodiscard]] ImageMetadataDatabaseConstPtr const& imageMetadataDatabase() const override;
  [[nodiscard]] FunctionDatabaseConstPtr const& functionDatabase() const override;
  [[nodiscard]] BehaviorDatabaseConstPtr const& behaviorDatabase() const override;
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

  // Is this WorldClient properly initialized in a world
  [[nodiscard]] bool inWorld() const;

  [[nodiscard]] bool inSpace() const;
  [[nodiscard]] bool flying() const;

  [[nodiscard]] bool mainPlayerDead() const;
  void reviveMainPlayer();
  [[nodiscard]] bool respawnInWorld() const;
  void setRespawnInWorld(bool respawnInWorld);

  [[nodiscard]] int64_t latency() const;

  void resendEntity(EntityId entityId);
  void removeEntity(EntityId entityId, bool andDie);

  [[nodiscard]] WorldTemplateConstPtr currentTemplate() const;
  void setTemplate(Json newTemplate);
  [[nodiscard]] SkyConstPtr currentSky() const;

  void dimWorld();
  [[nodiscard]] bool interactiveHighlightMode() const;
  void setInteractiveHighlightMode(bool enabled);
  void setParallax(ParallaxPtr newParallax);
  void overrideGravity(float gravity);
  void resetGravity();

  // Disable normal client-side lighting algorithm, everything full brightness.
  [[nodiscard]] bool fullBright() const;
  void setFullBright(bool fullBright);
  // Disable asynchronous client-side lighting algorithm, run on main thread.
  [[nodiscard]] bool asyncLighting() const;
  void setAsyncLighting(bool asyncLighting);
  // Spatial log generated collision geometry.
  [[nodiscard]] bool collisionDebug() const;
  void setCollisionDebug(bool collisionDebug);

  void handleIncomingPackets(List<PacketPtr> const& packets);
  [[nodiscard]] List<PacketPtr> getOutgoingPackets();

  // Set the rendering window for this client.
  void setClientWindow(RectI window);
  // Sets the client window around the position of the main player.
  void centerClientWindowOnPlayer(Vec2U const& windowSize);
  void centerClientWindowOnPlayer();
  [[nodiscard]] RectI clientWindow() const;
  [[nodiscard]] WorldClientState& clientState();

  void update(float dt);
  // borderTiles here should extend the client window for border tile
  // calculations.  It is not necessary on the light array.
  void render(WorldRenderData& renderData, unsigned borderTiles);
  [[nodiscard]] List<AudioInstancePtr> pullPendingAudio();
  [[nodiscard]] List<AudioInstancePtr> pullPendingMusic();

  [[nodiscard]] bool playerCanReachEntity(EntityId entityId, bool preferInteractive = true) const;

  void disconnectAllWires(Vec2I wireEntityPosition, WireNode const& node);
  void wire(Vec2I const& outputPosition, size_t outputIndex, Vec2I const& inputPosition, size_t inputIndex);
  void connectWire(WireConnection const& output, WireConnection const& input);

  // Functions for sending broadcast messages to other players that can receive them,
  // on completely vanilla servers by smuggling it through a DamageNotification.
  // It's cursed as fuck, but it works.
  [[nodiscard]] bool sendSecretBroadcast(StringView broadcast, bool raw = false, bool compress = true);
  [[nodiscard]] bool handleSecretBroadcast(PlayerPtr player, StringView broadcast);

  [[nodiscard]] List<ChatAction> pullPendingChatActions();

  [[nodiscard]] WorldStructure const& centralStructure() const;

  [[nodiscard]] DungeonId dungeonId(Vec2I const& pos) const;

  void collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId);

  [[nodiscard]] bool waitForLighting(WorldRenderData* renderData = nullptr);

  using BroadcastCallback = std::function<bool(PlayerPtr, StringView)>;
  [[nodiscard]] BroadcastCallback& broadcastCallback();

private:
  static const float DropDist;

  struct ClientRenderCallback : RenderCallback {
    void addDrawable(Drawable drawable, EntityRenderLayer renderLayer) override;
    void addLightSource(LightSource lightSource) override;
    void addParticle(Particle particle) override;
    void addAudio(AudioInstancePtr audio) override;
    void addTilePreview(PreviewTile preview) override;
    void addOverheadBar(OverheadBar bar) override;

    Map<EntityRenderLayer, List<Drawable>> drawables;
    List<LightSource> lightSources;
    List<Particle> particles;
    List<AudioInstancePtr> audios;
    List<PreviewTile> previewTiles;
    List<OverheadBar> overheadBars;
  };

  using ClientTileGetter = function<ClientTile const&(Vec2I)>;

  void initWorld(WorldStartPacket const& packet);
  void clearWorld();
  void tryGiveMainPlayerItem(ItemPtr item, bool silent = false);

  void notifyEntityCreate(EntityPtr const& entity);

  // Queues pending (step based) updates to server,
  void queueUpdatePackets(bool sendEntityUpdates);

  [[nodiscard]] WeatherNoisesDescriptionPtr currentWeatherNoises() const;

  [[nodiscard]] BiomeConstPtr mainEnvironmentBiome() const;

  void dirtyCollision(RectI const& region);
  void freshenCollision(RectI const& region);
  void renderCollisionDebug();

  void setTileProtection(DungeonId dungeonId, bool isProtected);

  void setupForceRegions();

  Json m_clientConfig;

  friend class StarWorldClientLighting;
  friend class StarWorldClientAudio;
  friend class StarWorldClientDamageFX;
  friend class StarWorldClientTilePrediction;
  StarWorldClientLighting m_lighting{*this};
  StarWorldClientAudio m_audio{*this};
  StarWorldClientDamageFX m_damageFX{*this};
  StarWorldClientTilePrediction m_tilePrediction{*this};
  WorldTemplatePtr m_worldTemplate;
  WorldStructure m_centralStructure;
  Vec2F m_playerStart;
  bool m_respawnInWorld;
  JsonObject m_worldProperties;

  EntityMapPtr m_entityMap;
  ClientTileSectorArrayPtr m_tileArray;
  ClientTileGetter m_tileGetterFunction;
  DamageManagerPtr m_damageManager;
  LuaRootPtr m_luaRoot;

  WorldGeometry m_geometry;
  uint64_t m_currentStep = 0;
  double m_currentTime = 0.0;

  List<PreviewTile> m_previewTiles;

  SkyPtr m_sky;

  AssetsConstPtr m_assets;

  CollisionGenerator m_collisionGenerator;
  HashMap<Vec2I, StaticList<CollisionBlock, CollisionGenerator::MaximumCollisionsPerSpace>> m_collisionCache;

  WorldClientState m_clientState;
  Maybe<ConnectionId> m_clientId;

  PlayerPtr m_mainPlayer;
  ConfigurationPtr m_configuration;
  MaterialDatabaseConstPtr m_materialDatabase;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  EntityFactoryConstPtr m_entityFactory;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  TerrainDatabaseConstPtr m_terrainDatabase;
  BiomeDatabaseConstPtr m_biomeDatabase;
  FunctionDatabaseConstPtr m_functionDatabase;
  BehaviorDatabaseConstPtr m_behaviorDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  ProjectileDatabaseConstPtr m_projectileDatabase;
  DamageDatabaseConstPtr m_damageDatabase;
  EffectSourceDatabaseConstPtr m_effectSourceDatabase;
  TechDatabaseConstPtr m_techDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  PlantDatabaseConstPtr m_plantDatabase;
  TreasureDatabaseConstPtr m_treasureDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  DungeonDefinitionsConstPtr m_dungeonDefinitions;

  bool m_collisionDebug = false;
  float m_interactivePulseAmount;
  float m_interactivePulseRate;
  float m_inspectionFlickerAmount;

  // Client side entity updates are not done until m_inWorld is true, which is
  // set to true after we have entered a world *and* the first batch of updates
  // are received.
  bool m_inWorld = false;

  GameTimer m_worldDimTimer;
  float m_worldDimLevel = 0.0f;
  Vec3B m_worldDimColor;

  GameTimer m_parallaxFadeTimer;
  ParallaxPtr m_currentParallax;
  ParallaxPtr m_nextParallax;

  Maybe<float> m_overrideGravity;

  ClientWeather m_weather;
  ParticleManagerPtr m_particles;

  List<AudioInstancePtr> m_samples;
  List<AudioInstancePtr> m_music;

  HashMap<EntityId, uint64_t> m_masterEntitiesNetVersion;

  InterpolationTracker m_interpolationTracker;
  GameTimer m_entityUpdateTimer;

  List<PacketPtr> m_outgoingPackets;
  Maybe<int64_t> m_pingTime;
  int64_t m_latency = 0;

  Set<EntityId> m_requestedDrops;

  HashSet<Vec2I> m_damagedBlocks;

  struct WorldTimer {
    float remainingTime;
    WorldAction action;
  };
  List<WorldTimer> m_timers;

  AudioInstancePtr m_spaceSound;
  String m_activeSpaceSound;

  AmbientNoisesDescriptionPtr m_altMusicTrackDescription;
  bool m_altMusicActive;

  HashSet<EntityId> m_startupHiddenEntities;

  HashMap<DungeonId, float> m_dungeonIdGravity;
  HashMap<DungeonId, bool> m_dungeonIdBreathable;
  StableHashSet<DungeonId> m_protectedDungeonIds;

  HashMap<String, List<RpcPromiseKeeper<Vec2F>>> m_findUniqueEntityResponses;
  HashMap<Uuid, RpcPromiseKeeper<Json>> m_entityMessageResponses;
  HashMap<Uuid, RpcPromiseKeeper<InteractAction>> m_entityInteractionResponses;

  List<PhysicsForceRegion> m_forceRegions;

  BroadcastCallback m_broadcastCallback;

  // used to keep track of already-printed stack traces caused by remote entities, so they don't clog the log
  HashSet<uint64_t> m_entityExceptionsLogged;
};

}// namespace Star
