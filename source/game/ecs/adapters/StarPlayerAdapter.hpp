#pragma once

#include "StarEntityAdapter.hpp"
#include "StarPlayerTypes.hpp"
#include "StarActorMovementController.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarStatusController.hpp"
#include "StarEffectEmitter.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarHumanoid.hpp"
#include "StarArmorWearer.hpp"
#include "StarToolUser.hpp"
#include "StarSongbook.hpp"
#include "StarDamageBarEntity.hpp"
#include "StarNametagEntity.hpp"
#include "StarPortraitEntity.hpp"
#include "StarChattyEntity.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarEmoteEntity.hpp"
#include "StarInspectableEntity.hpp"
#include "StarLoungingEntities.hpp"
#include "StarToolUserEntity.hpp"
#include "StarAiTypes.hpp"
#include "StarUuid.hpp"

namespace Star {

// Forward declarations
STAR_STRUCT(PlayerConfig);
STAR_CLASS(PlayerInventory);
STAR_CLASS(PlayerBlueprints);
STAR_CLASS(PlayerTech);
STAR_CLASS(PlayerCompanions);
STAR_CLASS(PlayerDeployment);
STAR_CLASS(PlayerLog);
STAR_CLASS(TechController);
STAR_CLASS(ClientContext);
STAR_CLASS(Statistics);
STAR_CLASS(PlayerCodexes);
STAR_CLASS(QuestManager);
STAR_CLASS(PlayerUniverseMap);
STAR_CLASS(UniverseClient);
STAR_CLASS(WireConnector);

namespace ECS {

// Tag component for player entities
struct PlayerTag {};

// Player state enumeration
enum class PlayerState {
  Idle,
  Walk,
  Run,
  Jump,
  Fall,
  Swim,
  SwimIdle,
  TeleportIn,
  TeleportOut,
  Crouch,
  Lounge
};

// Player-specific data component (comprehensive)
struct PlayerDataComponent {
  PlayerConfigPtr config;
  Uuid uuid;
  
  // Humanoid appearance
  HumanoidPtr humanoid;
  HumanoidIdentity identity;
  bool identityUpdated = false;
  Maybe<String> deathParticleBurst;
  
  // Movement and physics
  ActorMovementControllerPtr movementController;
  TechControllerPtr techController;
  
  // Status
  StatusControllerPtr statusController;
  
  // Effects
  EffectEmitterPtr effectEmitter;
  NetworkedAnimatorPtr effectsAnimator;
  
  // Equipment
  ArmorWearerPtr armor;
  ToolUserPtr tools;
  SongbookPtr songbook;
  
  // Inventory and progression
  PlayerInventoryPtr inventory;
  PlayerBlueprintsPtr blueprints;
  PlayerUniverseMapPtr universeMap;
  PlayerCodexesPtr codexes;
  PlayerTechPtr techs;
  PlayerCompanionsPtr companions;
  PlayerDeploymentPtr deployment;
  PlayerLogPtr log;
  
  // Quest system
  QuestManagerPtr questManager;
  
  // Client context
  ClientContextPtr clientContext;
  StatisticsPtr statistics;
  UniverseClient* client = nullptr;
  
  // State
  PlayerState state = PlayerState::Idle;
  HumanoidEmote emoteState = HumanoidEmote::Idle;
  Maybe<String> dance;
  
  // Timers
  GameTimer emoteCooldownTimer;
  GameTimer danceCooldownTimer;
  GameTimer blinkCooldownTimer;
  GameTimer ageItemsTimer;
  Vec2F blinkInterval;
  float footstepTimer = 0.0f;
  float teleportTimer = 0.0f;
  float lastDamagedOtherTimer = 0.0f;
  EntityId lastDamagedTarget = NullEntityId;
  
  // Audio state
  float footstepVolumeVariance = 0.0f;
  float landingVolume = 0.0f;
  bool landingNoisePending = false;
  bool footstepPending = false;
  String teleportAnimationType;
  
  // Emote cooldown
  float emoteCooldown = 0.0f;
  
  // Movement input
  HashSet<MoveControlType> pendingMoves;
  Vec2F moveVector;
  bool shifting = false;
  ActorMovementParameters zeroGMovementParameters;
  
  // Combat
  List<DamageSource> damageSources;
  
  // Description
  String description;
  
  // Mode
  PlayerMode modeType = PlayerMode::Casual;
  PlayerModeConfig modeConfig;
  ShipUpgrades shipUpgrades;
  
  // Tool state
  bool useDown = false;
  bool edgeTriggeredUse = false;
  
  // Aim
  Vec2F aimPosition;
  Maybe<EntityId> cameraFocusEntity;
  
  // Food system
  float foodLowThreshold = 0.0f;
  List<PersistentStatusEffect> foodLowStatusEffects;
  List<PersistentStatusEffect> foodEmptyStatusEffects;
  List<PersistentStatusEffect> inCinematicStatusEffects;
  
  // Admin mode
  bool isAdmin = false;
  float interactRadius = 8.0f;
  Vec2F walkIntoInteractBias;
  
  // Pending actions
  List<RpcPromise<InteractAction>> pendingInteractActions;
  List<Particle> callbackParticles;
  List<tuple<String, float, float>> callbackSounds;
  List<String> queuedMessages;
  List<ItemPtr> queuedItemPickups;
  List<ChatAction> pendingChatActions;
  
  // Radio messages
  StringSet missionRadioMessages;
  bool interruptRadioMessage = false;
  List<pair<GameTimer, RadioMessage>> delayedRadioMessages;
  Deque<RadioMessage> pendingRadioMessages;
  Maybe<Json> pendingCinematic;
  Maybe<pair<Maybe<pair<StringList, int>>, float>> pendingAltMusic;
  Maybe<PlayerWarpRequest> pendingWarp;
  Deque<pair<Json, RpcPromiseKeeper<Json>>> pendingConfirmations;
  
  // AI state
  AiState aiState;
  
  // Chat
  String chatMessage;
  bool chatMessageChanged = false;
  bool chatMessageUpdated = false;
  
  // Nametag override
  Maybe<String> nametagOverride;
  
  // Damage notification
  int hitDamageNotificationLimiter = 0;
  int hitDamageNotificationLimit = 10;
  
  // Interesting objects for scanning
  StringSet interestingObjects;
  
  // Generic script contexts
  StringMap<shared_ptr<void>> genericScriptContexts;
  JsonObject genericProperties;
  
  // Scripted animation
  StringMap<Json> scriptedAnimationParameters;
};

class PlayerAdapter 
  : public EntityAdapter,
    public virtual ToolUserEntity,
    public virtual LoungingEntity,
    public virtual ChattyEntity,
    public virtual InspectableEntity,
    public virtual DamageBarEntity,
    public virtual PortraitEntity,
    public virtual NametagEntity,
    public virtual PhysicsEntity,
    public virtual EmoteEntity {
public:
  // Factory method to create new player
  static shared_ptr<PlayerAdapter> create(World& ecsWorld, PlayerConfigPtr config, Uuid uuid = Uuid());
  
  // Factory method to create from disk store
  static shared_ptr<PlayerAdapter> createFromDiskStore(World& ecsWorld, PlayerConfigPtr config, Json const& diskStore);
  
  // Factory method to create from network
  static shared_ptr<PlayerAdapter> createFromNetStore(World& ecsWorld, PlayerConfigPtr config, ByteArray const& netStore, NetCompatibilityRules rules = {});

  PlayerAdapter(World& ecsWorld, Entity entity);
  virtual ~PlayerAdapter() = default;

  // Context management
  ClientContextPtr clientContext() const;
  void setClientContext(ClientContextPtr clientContext);
  
  StatisticsPtr statistics() const;
  void setStatistics(StatisticsPtr statistics);
  
  void setUniverseClient(UniverseClient* universeClient);
  UniverseClient* universeClient() const;
  
  QuestManagerPtr questManager() const;

  // Disk and network serialization
  Json diskStore();
  ByteArray netStore(NetCompatibilityRules rules = {});

  // Entity interface
  EntityType entityType() const override;
  ClientEntityMode clientEntityMode() const override;

  void init(::Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  Vec2F position() const override;
  Vec2F velocity() const override;

  Vec2F mouthPosition() const override;
  Vec2F mouthPosition(bool ignoreAdjustments) const override;
  Vec2F mouthOffset(bool ignoreAdjustments = true) const;
  Vec2F feetOffset() const;
  Vec2F headArmorOffset() const;
  Vec2F chestArmorOffset() const;
  Vec2F legsArmorOffset() const;
  Vec2F backArmorOffset() const;

  RectF metaBoundBox() const override;
  RectF collisionArea() const override;

  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  // Combat
  Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;

  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  List<DamageNotification> selfDamageNotifications() override;

  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;

  List<DamageSource> damageSources() const override;

  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  // Lounging
  Maybe<EntityAnchorState> loungingIn() const override;
  bool lounge(EntityId loungeableEntityId, size_t anchorIndex);
  void stopLounging();

  void revive(Vec2F const& footPosition);

  // Portrait and rendering
  List<Drawable> portrait(PortraitMode mode) const override;
  bool underwater() const;

  // Movement control
  bool shifting() const;
  void setShifting(bool shifting);
  void special(int specialKey);

  void setMoveVector(Vec2F const& vec);
  void moveLeft();
  void moveRight();
  void moveUp();
  void moveDown();
  void jump();

  void dropItem();

  float toolRadius() const;
  float interactRadius() const override;
  void setInteractRadius(float interactRadius);
  List<InteractAction> pullInteractActions();

  uint64_t currency(String const& currencyType) const;

  // DamageBarEntity interface
  float health() const override;
  float maxHealth() const override;
  DamageBarType damageBar() const override;
  float healthPercentage() const;

  float energy() const override;
  float maxEnergy() const;
  float energyPercentage() const;
  float energyRegenBlockPercent() const;

  bool energyLocked() const override;
  bool fullEnergy() const override;
  bool consumeEnergy(float energy) override;

  float foodPercentage() const;

  float breath() const;
  float maxBreath() const;

  float protection() const;

  bool forceNude() const;

  String description() const override;
  void setDescription(String const& description);

  List<LightSource> lightSources() const override;

  Direction walkingDirection() const override;
  Direction facingDirection() const override;

  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;

  // Generic properties
  Json getGenericProperty(String const& name, Json const& defaultValue = Json()) const;
  void setGenericProperty(String const& name, Json const& value);

  // Inventory
  PlayerInventoryPtr inventory() const;
  uint64_t itemsCanHold(ItemPtr const& items) const;
  ItemPtr pickupItems(ItemPtr const& items, bool silent = false);
  void giveItem(ItemPtr const& item);
  void triggerPickupEvents(ItemPtr const& item);

  ItemPtr essentialItem(EssentialItem essentialItem) const;
  bool hasItem(ItemDescriptor const& descriptor, bool exactMatch = false) const;
  uint64_t hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch = false) const;
  ItemDescriptor takeItem(ItemDescriptor const& descriptor, bool consumePartial = false, bool exactMatch = false);
  void giveItem(ItemDescriptor const& descriptor);

  void clearSwap();

  void refreshItems();
  void refreshArmor();
  void refreshHumanoid() const;
  void refreshEquipment();

  // Blueprints
  PlayerBlueprintsPtr blueprints() const;
  bool addBlueprint(ItemDescriptor const& descriptor, bool showFailure = false);
  bool blueprintKnown(ItemDescriptor const& descriptor) const;

  bool addCollectable(String const& collectionName, String const& collectableName);

  PlayerUniverseMapPtr universeMap() const;

  PlayerCodexesPtr codexes() const;

  PlayerTechPtr techs() const;
  void overrideTech(Maybe<StringList> const& techModules);
  bool techOverridden() const;

  PlayerCompanionsPtr companions() const;

  PlayerLogPtr log() const;

  // Interaction
  InteractiveEntityPtr bestInteractionEntity(bool includeNearby);
  void interactWithEntity(InteractiveEntityPtr entity);

  // Aiming
  void aim(Vec2F const& position);
  Vec2F aimPosition() const override;

  // ToolUserEntity interface
  Vec2F armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const override;
  Vec2F handOffset(ToolHand hand, Direction facingDirection) const override;
  Vec2F handPosition(ToolHand hand, Vec2F const& handOffset = {}) const override;
  ItemPtr handItem(ToolHand hand) const override;
  Vec2F armAdjustment() const override;

  void setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) override;

  // EmoteEntity interface
  void playEmote(HumanoidEmote emote) override;

  bool canUseTool() const;

  // Tool firing
  void beginPrimaryFire();
  void beginAltFire();
  void endPrimaryFire();
  void endAltFire();

  void beginTrigger();
  void endTrigger();

  ItemPtr primaryHandItem() const;
  ItemPtr altHandItem() const;

  // Player identity
  Uuid uuid() const;

  PlayerMode modeType() const;
  void setModeType(PlayerMode mode);
  PlayerModeConfig modeConfig() const;

  ShipUpgrades shipUpgrades();
  void setShipUpgrades(ShipUpgrades shipUpgrades);
  void applyShipUpgrades(Json const& upgrades);

  // PortraitEntity/NametagEntity interface
  String name() const override;
  void setName(String const& name);

  Maybe<String> statusText() const override;
  bool displayNametag() const override;
  Vec3B nametagColor() const override;
  Vec2F nametagOrigin() const override;
  String nametag() const override;
  void setNametag(Maybe<String> nametag);

  // Identity management
  void updateIdentity();
  HumanoidPtr humanoid();
  HumanoidPtr humanoid() const;
  HumanoidIdentity const& identity() const;
  void setIdentity(HumanoidIdentity identity);

  // Admin mode
  void setAdmin(bool isAdmin);
  bool isAdmin() const override;

  bool inToolRange() const override;
  bool inToolRange(Vec2F const& aimPos) const override;
  bool inInteractionRange() const;
  bool inInteractionRange(Vec2F aimPos) const;

  void addParticles(List<Particle> const& particles) override;
  void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) override;

  bool wireToolInUse() const;
  void setWireConnector(WireConnector* wireConnector) const;

  void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) override;
  ActiveUniqueStatusEffectSummary activeUniqueStatusEffectSummary() const override;

  float powerMultiplier() const override;

  bool isDead() const;
  void kill();

  void setFavoriteColor(Color color);
  Color favoriteColor() const override;

  // Teleportation
  void teleportOut(String const& animationType = "default", bool deploy = false);
  void teleportIn();
  void teleportAbort();

  bool isTeleporting() const;
  bool isTeleportingOut() const;
  bool canDeploy();
  void deployAbort(String const& animationType = "default");
  bool isDeploying() const;
  bool isDeployed() const;

  void setBusyState(PlayerBusyState busyState);

  void moveTo(Vec2F const& footPosition);

  List<String> pullQueuedMessages();
  List<ItemPtr> pullQueuedItemDrops();

  void queueUIMessage(String const& message) override;
  void queueItemPickupMessage(ItemPtr const& item);

  void addChatMessage(String const& message, Json const& config = {});
  void addEmote(HumanoidEmote const& emote, Maybe<float> emoteCooldown = {});
  void setDance(Maybe<String> const& danceName);
  pair<HumanoidEmote, float> currentEmote() const;

  PlayerState currentState() const;

  // ChattyEntity interface
  List<ChatAction> pullPendingChatActions() override;

  // InspectableEntity interface
  Maybe<String> inspectionLogName() const override;
  Maybe<String> inspectionDescription(String const& species) const override;

  float beamGunRadius() const override;

  bool instrumentPlaying() override;
  void instrumentEquipped(String const& instrumentKind) override;
  void interact(InteractAction const& action) override;
  void addEffectEmitters(StringSet const& emitters) override;
  void requestEmote(String const& emote) override;

  // ActorEntity interface
  ActorMovementController* movementController() override;
  StatusController* statusController() override;

  // PhysicsEntity interface
  List<PhysicsForceRegion> forceRegions() const override;

  StatusControllerPtr statusControllerPtr();
  ActorMovementControllerPtr movementControllerPtr();

  PlayerConfigPtr playerConfig();

  SongbookPtr songbook() const;

  void finalizeCreation();

  float timeSinceLastGaveDamage() const;
  EntityId lastDamagedTarget() const;

  bool invisible() const;

  void animatePortrait(float dt);

  bool isOutside();

  void dropSelectedItems(function<bool(ItemPtr)> filter);
  void dropEverything();

  bool isPermaDead() const;

  bool interruptRadioMessage();
  Maybe<RadioMessage> pullPendingRadioMessage();
  void queueRadioMessage(Json const& messageConfig, float delay = 0);
  void queueRadioMessage(RadioMessage message);

  Maybe<Json> pullPendingCinematic();
  void setPendingCinematic(Json const& cinematic, bool unique = false);

  void setInCinematic(bool inCinematic);

  Maybe<pair<Maybe<pair<StringList, int>>, float>> pullPendingAltMusic();

  Maybe<PlayerWarpRequest> pullPendingWarp();
  void setPendingWarp(String const& action, Maybe<String> const& animation = {}, bool deploy = false);

  Maybe<pair<Json, RpcPromiseKeeper<Json>>> pullPendingConfirmation();
  void queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise);

  AiState const& aiState() const;
  AiState& aiState();

  bool inspecting() const;

  EntityHighlightEffect inspectionHighlight(InspectableEntityPtr const& inspectableEntity) const;

  Vec2F cameraPosition();

  using EntityAdapter::setTeam;

  NetworkedAnimatorPtr effectsAnimator();

  // Secret property API for network state
  Maybe<StringView> getSecretPropertyView(String const& name) const;
  String const* getSecretPropertyPtr(String const& name) const;
  Json getSecretProperty(String const& name, Json defaultValue = Json()) const;
  void setSecretProperty(String const& name, Json const& value);

  void setAnimationParameter(String name, Json value);

private:
  PlayerDataComponent* getData();
  PlayerDataComponent const* getData() const;

  void processControls();
  void processStateChanges(float dt);

  void getNetStates(bool initial);
  void setNetStates();

  List<Drawable> drawables() const;
  List<OverheadBar> bars() const;
  List<Particle> particles();
  String getFootstepSound(Vec2I const& sensor) const;

  void tickShared(float dt);

  HumanoidEmote detectEmotes(String const& chatter);

  // Network state elements
  NetElementTopGroup m_netGroup;
  NetElementUInt m_stateNetState;
  NetElementBool m_shiftingNetState;
  NetElementFloat m_xAimPositionNetState;
  NetElementFloat m_yAimPositionNetState;
  NetElementData<HumanoidIdentity> m_identityNetState;
  NetElementEvent m_refreshedHumanoidParameters;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementEvent m_landedNetState;
  NetElementString m_chatMessageNetState;
  NetElementEvent m_newChatMessageNetState;
  NetElementString m_emoteNetState;
  NetElementData<Maybe<String>> m_humanoidDanceNetState;
  NetElementData<Maybe<String>> m_deathParticleBurst;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
};

} // namespace ECS
} // namespace Star
