#pragma once

#include "StarAmbient.hpp"
#include "StarCellularLighting.hpp"
#include "StarChatAction.hpp"
#include "StarConfig.hpp"
#include "StarEntityRendering.hpp"
#include "StarException.hpp"
#include "StarGameTimers.hpp"
#include "StarInterpolationTracker.hpp"
#include "StarLuaRoot.hpp"
#include "StarNetPackets.hpp"
#include "StarWeather.hpp"
#include "StarWiring.hpp"
#include "StarWorld.hpp"
#include "StarWorldClientState.hpp"
#include "StarWorldRenderData.hpp"
#include "StarWorldStructure.hpp"

import std;

namespace Star {

class Player;
class WorldTemplate;
class Sky;
class ParticleManager;

using WorldClientException = ExceptionDerived<"WorldClientException">;

class WorldClient : public World {
public:
  WorldClient(Ptr<Player> mainPlayer, Ptr<LuaRoot> luaRoot);
  ~WorldClient() override;

  auto connection() const -> ConnectionId override;
  auto geometry() const -> WorldGeometry override;
  auto currentStep() const -> std::uint64_t override;
  auto material(Vec2I const& position, TileLayer layer) const -> MaterialId override;
  auto materialHueShift(Vec2I const& position, TileLayer layer) const -> MaterialHue override;
  auto mod(Vec2I const& position, TileLayer layer) const -> ModId override;
  auto modHueShift(Vec2I const& position, TileLayer layer) const -> MaterialHue override;
  auto colorVariant(Vec2I const& position, TileLayer layer) const -> MaterialColorVariant override;
  auto liquidLevel(Vec2I const& pos) const -> LiquidLevel override;
  auto liquidLevel(RectF const& region) const -> LiquidLevel override;
  auto validTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) const -> TileModificationList override;
  auto applyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) -> TileModificationList override;
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
  auto damageTiles(List<Vec2I> const& pos, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, std::optional<EntityId> sourceEntity = {}) -> TileDamageResult override;
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
  auto day() const -> std::uint32_t override;
  auto dayLength() const -> float override;
  auto timeOfDay() const -> float override;
  auto luaRoot() -> Ptr<LuaRoot> override;
  auto findUniqueEntity(String const& uniqueId) -> RpcPromise<Vec2F> override;
  auto sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) -> RpcPromise<Json> override;
  auto isTileProtected(Vec2I const& pos) const -> bool override;

  // Is this WorldClient properly initialized in a world
  auto inWorld() const -> bool;

  auto inSpace() const -> bool;
  auto flying() const -> bool;

  auto mainPlayerDead() const -> bool;
  void reviveMainPlayer();
  auto respawnInWorld() const -> bool;
  void setRespawnInWorld(bool respawnInWorld);

  auto latency() const -> std::int64_t;

  void resendEntity(EntityId entityId);
  void removeEntity(EntityId entityId, bool andDie);

  auto currentTemplate() const -> ConstPtr<WorldTemplate>;
  void setTemplate(Json newTemplate);
  auto currentSky() const -> ConstPtr<Sky>;

  void dimWorld();
  auto interactiveHighlightMode() const -> bool;
  void setInteractiveHighlightMode(bool enabled);
  void setParallax(Ptr<Parallax> newParallax);
  void overrideGravity(float gravity);
  void resetGravity();

  // Disable normal client-side lighting algorithm, everything full brightness.
  auto fullBright() const -> bool;
  void setFullBright(bool fullBright);
  // Disable asynchronous client-side lighting algorithm, run on main thread.
  auto asyncLighting() const -> bool;
  void setAsyncLighting(bool asyncLighting);
  // Spatial log generated collision geometry.
  auto collisionDebug() const -> bool;
  void setCollisionDebug(bool collisionDebug);

  void handleIncomingPackets(List<Ptr<Packet>> const& packets);
  auto getOutgoingPackets() -> List<Ptr<Packet>>;

  // Set the rendering window for this client.
  void setClientWindow(RectI window);
  // Sets the client window around the position of the main player.
  void centerClientWindowOnPlayer(Vec2U const& windowSize);
  void centerClientWindowOnPlayer();
  auto clientWindow() const -> RectI;
  auto clientState() -> WorldClientState&;

  void update(float dt);
  // borderTiles here should extend the client window for border tile
  // calculations.  It is not necessary on the light array.
  void render(WorldRenderData& renderData, unsigned borderTiles);
  auto pullPendingAudio() -> List<Ptr<AudioInstance>>;
  auto pullPendingMusic() -> List<Ptr<AudioInstance>>;

  auto playerCanReachEntity(EntityId entityId, bool preferInteractive = true) const -> bool;

  void disconnectAllWires(Vec2I wireEntityPosition, WireNode const& node);
  void wire(Vec2I const& outputPosition, size_t outputIndex, Vec2I const& inputPosition, size_t inputIndex);
  void connectWire(WireConnection const& output, WireConnection const& input);

  // Functions for sending broadcast messages to other players that can receive them,
  // on completely vanilla servers by smuggling it through a DamageNotification.
  // It's cursed as fuck, but it works.
  auto sendSecretBroadcast(StringView broadcast, bool raw = false, bool compress = true) -> bool;
  auto handleSecretBroadcast(Ptr<Player> player, StringView broadcast) -> bool;

  auto pullPendingChatActions() -> List<ChatAction>;

  auto centralStructure() const -> WorldStructure const&;

  auto dungeonId(Vec2I const& pos) const -> DungeonId;

  void collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId);

  auto waitForLighting(WorldRenderData* renderData = nullptr) -> bool;

  using BroadcastCallback = std::function<bool(Ptr<Player>, StringView)>;
  auto broadcastCallback() -> BroadcastCallback&;

private:
  static const float DropDist;

  struct ClientRenderCallback : RenderCallback {
    void addDrawable(Drawable drawable, EntityRenderLayer renderLayer) override;
    void addLightSource(LightSource lightSource) override;
    void addParticle(Particle particle) override;
    void addAudio(Ptr<AudioInstance> audio) override;
    void addTilePreview(PreviewTile preview) override;
    void addOverheadBar(OverheadBar bar) override;

    Map<EntityRenderLayer, List<Drawable>> drawables;
    List<LightSource> lightSources;
    List<Particle> particles;
    List<Ptr<AudioInstance>> audios;
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

    auto operator<(DamageNumberKey const& other) const -> bool;
  };

  using ClientTileGetter = std::function<ClientTile const&(Vec2I)>;

  void lightingTileGather();
  void lightingCalc();
  void lightingMain();

  void initWorld(WorldStartPacket const& packet);
  void clearWorld();
  void tryGiveMainPlayerItem(Ptr<Item> item, bool silent = false);

  void notifyEntityCreate(Ptr<Entity> const& entity);

  // Queues pending (step based) updates to server,
  void queueUpdatePackets(bool sendEntityUpdates);
  void handleDamageNotifications();

  void sparkDamagedBlocks();

  auto environmentBiomeTrackPosition() const -> Vec2I;
  auto currentAmbientNoises() const -> Ptr<AmbientNoisesDescription>;
  auto currentWeatherNoises() const -> Ptr<WeatherNoisesDescription>;
  auto currentMusicTrack() const -> Ptr<AmbientNoisesDescription>;
  auto currentAltMusicTrack() const -> Ptr<AmbientNoisesDescription>;

  void playAltMusic(StringList const& newTracks, float fadeTime, int loops = -1);
  void stopAltMusic(float fadeTime);

  auto mainEnvironmentBiome() const -> ConstPtr<Biome>;

  // Populates foregroundTransparent / backgroundTransparent flag on ClientTile
  // based on transparency rules.
  auto readNetTile(Vec2I const& pos, NetTile const& netTile, bool updateCollision = true) -> bool;
  void dirtyCollision(RectI const& region);
  void freshenCollision(RectI const& region);
  void renderCollisionDebug();

  void informTilePrediction(Vec2I const& pos, TileModification const& modification);

  void setTileProtection(DungeonId dungeonId, bool isProtected);

  void setupForceRegions();

  Json m_clientConfig;
  Ptr<WorldTemplate> m_worldTemplate;
  WorldStructure m_centralStructure;
  Vec2F m_playerStart;
  bool m_respawnInWorld;
  JsonObject m_worldProperties;

  Ptr<EntityMap> m_entityMap;
  Ptr<ClientTileSectorArray> m_tileArray;
  ClientTileGetter m_tileGetterFunction;
  Ptr<DamageManager> m_damageManager;
  Ptr<LuaRoot> m_luaRoot;

  WorldGeometry m_geometry;
  std::uint64_t m_currentStep;
  double m_currentTime;
  bool m_fullBright;
  bool m_asyncLighting;
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

  Ptr<Sky> m_sky;

  CollisionGenerator m_collisionGenerator;

  WorldClientState m_clientState;
  std::optional<ConnectionId> m_clientId;

  Ptr<Player> m_mainPlayer;

  bool m_collisionDebug;

  // Client side entity updates are not done until m_inWorld is true, which is
  // set to true after we have entered a world *and* the first batch of updates
  // are received.
  bool m_inWorld;

  GameTimer m_worldDimTimer;
  float m_worldDimLevel;
  Vec3B m_worldDimColor;

  bool m_interactiveHighlightMode;

  GameTimer m_parallaxFadeTimer;
  Ptr<Parallax> m_currentParallax;
  Ptr<Parallax> m_nextParallax;

  std::optional<float> m_overrideGravity;

  ClientWeather m_weather;
  Ptr<ParticleManager> m_particles;

  List<Ptr<AudioInstance>> m_samples;
  List<Ptr<AudioInstance>> m_music;

  HashMap<EntityId, std::uint64_t> m_masterEntitiesNetVersion;

  InterpolationTracker m_interpolationTracker;
  GameTimer m_entityUpdateTimer;

  List<Ptr<Packet>> m_outgoingPackets;
  std::optional<std::int64_t> m_pingTime;
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

  Ptr<AudioInstance> m_spaceSound;
  String m_activeSpaceSound;

  Ptr<AmbientNoisesDescription> m_altMusicTrackDescription;
  bool m_altMusicActive;

  int m_modifiedTilePredictionTimeout;
  HashMap<Vec2I, PredictedTile> m_predictedTiles;
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
  HashSet<std::uint64_t> m_entityExceptionsLogged;
};

}// namespace Star
