module;
import std;
import star.json;
import star.net_compatibility;
import star.byte_array;
import star.id_map;
import star.data_stream;
import star.bi_map;
import star.casting;
import star.variant;
import star.rpc_promise;
import star.strong_typedef;
import star.game_types;
import star.maybe;
import star.vector;
import star.rect;
import star.a_star;
import star.ordered_set;
import star.poly;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.color;
import star.string;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.either;
import star.random;
import star.periodic_function;
import star.ordered_map;

import star.uuid;
import star.item_descriptor;
import star.drawable;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.lua_root;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;

import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_containers;
import star.net_element_dynamic_group;
import star.net_element_system;

namespace Star {

class ArmorItem;
class ArmorWearer;
class ToolUser;
class EffectEmitter;

struct PlayerConfig;
class Songbook;
class WireConnector;
class PlayerInventory;
class PlayerBlueprints;
class PlayerTech;
class PlayerCompanions;
class PlayerDeployment;
class PlayerLog;
class TechController;
class ClientContext;
class Statistics;
class StatusController;
class PlayerCodexes;
class QuestManager;
class InteractiveEntity;
class PlayerUniverseMap;
class UniverseClient;

class Player :
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
  enum class State {
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
  static EnumMap<State> const StateNames;

  Player(std::shared_ptr<PlayerConfig> config, Uuid uuid = Uuid());
  Player(std::shared_ptr<PlayerConfig> config, ByteArray const& netStore, NetCompatibilityRules rules = {});
  Player(std::shared_ptr<PlayerConfig> config, Json const& diskStore);

  void diskLoad(Json const& diskStore);

  std::shared_ptr<ClientContext> clientContext() const;
  void setClientContext(std::shared_ptr<ClientContext> clientContext);

  std::shared_ptr<Statistics> statistics() const;
  void setStatistics(std::shared_ptr<Statistics> statistics);

  void setUniverseClient(UniverseClient* universeClient);
  UniverseClient* universeClient() const;

  std::shared_ptr<QuestManager> questManager() const;

  Json diskStore();
  ByteArray netStore(NetCompatibilityRules rules = {});

  EntityType entityType() const override;
  ClientEntityMode clientEntityMode() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
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

  // relative to current position
  RectF metaBoundBox() const override;

  // relative to current position
  RectF collisionArea() const override;

  std::pair<ByteArray, std::uint64_t> writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationStep = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  virtual Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;

  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  List<DamageNotification> selfDamageNotifications() override;

  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;

  List<DamageSource> damageSources() const override;

  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  Maybe<EntityAnchorState> loungingIn() const override;
  bool lounge(EntityId loungeableEntityId, std::size_t anchorIndex);
  void stopLounging();

  void revive(Vec2F const& footPosition);

  List<Drawable> portrait(PortraitMode mode) const override;
  bool underwater() const;

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

  std::uint64_t currency(String const& currencyType) const;

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

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) override;

  void update(float dt, std::uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;
  
  void setRenderLayer(Maybe<EntityRenderLayer> layer);

  Json getGenericProperty(String const& name, Json const& defaultValue = Json()) const;
  void setGenericProperty(String const& name, Json const& value);

  std::shared_ptr<PlayerInventory> inventory() const;
  // Returns the number of items from this stack that could be
  // picked up from the world, using inventory tab filtering
  std::uint64_t itemsCanHold(std::shared_ptr<Item> const& items) const;
  // Adds items to the inventory, returning the overflow.
  // The items parameter is invalid after use.
  std::shared_ptr<Item> pickupItems(std::shared_ptr<Item> const& items, bool silent = false);
  // Pick up all of the given items as possible, dropping the overflow.
  // The item parameter is invalid after use.
  void giveItem(std::shared_ptr<Item> const& item);

  void triggerPickupEvents(std::shared_ptr<Item> const& item);

  std::shared_ptr<Item> essentialItem(EssentialItem essentialItem) const;
  bool hasItem(ItemDescriptor const& descriptor, bool exactMatch = false) const;
  std::uint64_t hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch = false) const;
  // altough multiple entries may match, they might have different
  // serializations
  ItemDescriptor takeItem(ItemDescriptor const& descriptor, bool consumePartial = false, bool exactMatch = false);
  void giveItem(ItemDescriptor const& descriptor);

  // Clear the item swap slot.
  void clearSwap();

  void refreshItems();
  void refreshArmor();
  void refreshHumanoid() const;
  // Refresh worn equipment from the inventory
  void refreshEquipment();

  std::shared_ptr<PlayerBlueprints> blueprints() const;
  bool addBlueprint(ItemDescriptor const& descriptor, bool showFailure = false);
  bool blueprintKnown(ItemDescriptor const& descriptor) const;

  bool addCollectable(String const& collectionName, String const& collectableName);

  std::shared_ptr<PlayerUniverseMap> universeMap() const;

  std::shared_ptr<PlayerCodexes> codexes() const;

  std::shared_ptr<PlayerTech> techs() const;
  void overrideTech(Maybe<StringList> const& techModules);
  bool techOverridden() const;

  std::shared_ptr<PlayerCompanions> companions() const;

  std::shared_ptr<PlayerLog> log() const;

  std::shared_ptr<InteractiveEntity> bestInteractionEntity(bool includeNearby);
  void interactWithEntity(std::shared_ptr<InteractiveEntity> entity);

  // Aim this player's target at the given world position.
  void aim(Vec2F const& position);
  Vec2F aimPosition() const override;

  Vec2F armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const override;
  Vec2F handOffset(ToolHand hand, Direction facingDirection) const override;

  Vec2F handPosition(ToolHand hand, Vec2F const& handOffset = {}) const override;
  std::shared_ptr<Item> handItem(ToolHand hand) const override;

  Vec2F armAdjustment() const override;

  void setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) override;

  void playEmote(HumanoidEmote emote) override;

  bool canUseTool() const;

  // "Fires" whatever is in the primary (left) item slot, or the primary fire
  // of the 2H item, at whatever the current aim position is.  Will auto-repeat
  // depending on the item auto repeat setting.
  void beginPrimaryFire();
  // "Fires" whatever is in the alternate (right) item slot, or the alt fire of
  // the 2H item, at whatever the current aim position is.  Will auto-repeat
  // depending on the item auto repeat setting.
  void beginAltFire();

  void endPrimaryFire();
  void endAltFire();

  // Triggered whenever the use key is pressed
  void beginTrigger();
  void endTrigger();

  std::shared_ptr<Item> primaryHandItem() const;
  std::shared_ptr<Item> altHandItem() const;

  Uuid uuid() const;

  PlayerMode modeType() const;
  void setModeType(PlayerMode mode);
  PlayerModeConfig modeConfig() const;

  ShipUpgrades shipUpgrades();
  void setShipUpgrades(ShipUpgrades shipUpgrades);
  void applyShipUpgrades(Json const& upgrades);
  void setShipSpecies(String species);
  String shipSpecies() const;

  String name() const override;
  void setName(String const& name);

  Maybe<String> statusText() const override;
  bool displayNametag() const override;
  Vec3B nametagColor() const override;
  Vec2F nametagOrigin() const override;
  String nametag() const override;
  void setNametag(Maybe<String> nametag);

  void updateIdentity();

  void setHumanoidParameter(String key, Maybe<Json> value);
  Maybe<Json> getHumanoidParameter(String key);
  void setHumanoidParameters(JsonObject parameters);
  JsonObject getHumanoidParameters();
  void refreshHumanoidParameters();

  void setBodyDirectives(String const& directives);
  void setEmoteDirectives(String const& directives);

  void setHairGroup(String const& group);
  void setHairType(String const& type);
  void setHairDirectives(String const& directives);

  void setFacialHairGroup(String const& group);
  void setFacialHairType(String const& type);
  void setFacialHairDirectives(String const& directives);

  void setFacialMaskGroup(String const& group);
  void setFacialMaskType(String const& type);
  void setFacialMaskDirectives(String const& directives);

  void setHair      (String const& group, String const& type, String const& directives);
  void setFacialHair(String const& group, String const& type, String const& directives);
  void setFacialMask(String const& group, String const& type, String const& directives);

  String species() const override;
  void setSpecies(String const& species);
  Gender gender() const;
  void setGender(Gender const& gender);
  void setPersonality(Personality const& personality);
  void setImagePath(Maybe<String> const& imagePath);

  std::shared_ptr<Humanoid> humanoid();
  std::shared_ptr<Humanoid> humanoid() const;
  HumanoidIdentity const& identity() const;

  void setIdentity(HumanoidIdentity identity);

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

  // Starts the teleport animation sequence, locking player movement and
  // preventing some update code
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

  // A hard move to a specified location
  void moveTo(Vec2F const& footPosition);

  List<String> pullQueuedMessages();
  List<std::shared_ptr<Item>> pullQueuedItemDrops();

  void queueUIMessage(String const& message) override;
  void queueItemPickupMessage(std::shared_ptr<Item> const& item);

  void addChatMessage(String const& message, Json const& config = {});
  void addEmote(HumanoidEmote const& emote, Maybe<float> emoteCooldown = {});
  void setDance(Maybe<String> const & danceName);
  std::pair<HumanoidEmote, float> currentEmote() const;

  State currentState() const;

  List<ChatAction> pullPendingChatActions() override;

  Maybe<String> inspectionLogName() const override;
  Maybe<String> inspectionDescription(String const& species) const override;

  float beamGunRadius() const override;

  bool instrumentPlaying() override;
  void instrumentEquipped(String const& instrumentKind) override;
  void interact(InteractAction const& action) override;
  void addEffectEmitters(StringSet const& emitters) override;
  void requestEmote(String const& emote) override;

  ActorMovementController* movementController() override;
  StatusController* statusController() override;

  List<PhysicsForceRegion> forceRegions() const override;

  std::shared_ptr<StatusController> statusControllerPtr();
  std::shared_ptr<ActorMovementController> movementControllerPtr();

  std::shared_ptr<PlayerConfig> config();

  std::shared_ptr<Songbook> songbook() const;

  void finalizeCreation();

  float timeSinceLastGaveDamage() const;
  EntityId lastDamagedTarget() const;

  bool invisible() const;

  void animatePortrait(float dt);

  bool isOutside();

  void dropSelectedItems(std::function<bool(std::shared_ptr<Item>)> filter);
  void dropEverything();

  bool isPermaDead() const;

  bool interruptRadioMessage();
  Maybe<RadioMessage> pullPendingRadioMessage();
  void queueRadioMessage(Json const& messageConfig, float delay = 0);
  void queueRadioMessage(RadioMessage message);

  // If a cinematic should play, returns it and clears it.  May stop cinematics
  // by returning a null Json.
  Maybe<Json> pullPendingCinematic();
  void setPendingCinematic(Json const& cinematic, bool unique = false);

  void setInCinematic(bool inCinematic);

  Maybe<std::pair<Maybe<std::pair<StringList, int>>, float>> pullPendingAltMusic();

  Maybe<PlayerWarpRequest> pullPendingWarp();
  void setPendingWarp(String const& action, Maybe<String> const& animation = {}, bool deploy = false);

  Maybe<std::pair<Json, RpcPromiseKeeper<Json>>> pullPendingConfirmation();
  void queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise);

  AiState const& aiState() const;
  AiState& aiState();

  // In inspection mode, scannable, scanned, and interesting objects will be
  // rendered with special highlighting.
  bool inspecting() const;

  // Will return the highlight effect to give an inspectable entity when inspecting
  EntityHighlightEffect inspectionHighlight(std::shared_ptr<InspectableEntity> const& inspectableEntity) const;

  Vec2F cameraPosition();

  using Entity::setTeam;

  std::shared_ptr<NetworkedAnimator> effectsAnimator();

  // We need to store ephemeral/large/always-changing networked properties that other clients can read. Candidates:
  // genericProperties:
  //   Non-starter, is not networked.
  // statusProperties:
  //   Nope! Changes to the status properties aren't networked efficiently - one change resends the whole map.
  //   We can't fix that because it would break compatibility with vanilla servers.
  // effectsAnimator's globalTags:
  //   Cursed, but viable.
  //   Efficient networking due to using a NetElementMapWrapper.
  //   Unfortunately values are Strings, so to work with Json we need to serialize/deserialize. Whatever.
  //   Additionally, this is compatible with vanilla networking.
  // I call this a 'secret property'.

  // If the secret property exists as a serialized Json string, returns a view to it without deserializing.
  Maybe<StringView> getSecretPropertyView(String const& name) const;
  String const* getSecretPropertyPtr(String const& name) const;
  // Gets a secret Json property. It will be de-serialized.
  Json getSecretProperty(String const& name, Json defaultValue = Json()) const;
  // Sets a secret Json property. It will be serialized.
  void setSecretProperty(String const& name, Json const& value);

  void setAnimationParameter(String name, Json value);

private:
  typedef LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> GenericScriptComponent;

  typedef std::function<void(std::shared_ptr<Item>)> ItemSetFunc;

  // handle input and other events (master only) that happen BEFORE movement/tech controller updates
  void processControls();

  // state changes and effect animations (master and slave) that happen AFTER movement/tech controller updates
  void processStateChanges(float dt);

  void getNetStates(bool initial);
  void setNetStates();
  void getNetArmorSecrets();
  void setNetArmorSecret(EquipmentSlot slot, std::shared_ptr<ArmorItem> const& armor, bool visible = true);
  void setNetArmorSecrets(bool includeEmpty = false);

  List<Drawable> drawables() const;
  List<OverheadBar> bars() const;
  List<Particle> particles();
  String getFootstepSound(Vec2I const& sensor) const;

  void tickShared(float dt);

  HumanoidEmote detectEmotes(String const& chatter);

  NetElementDynamicGroup<NetHumanoid> m_netHumanoid;
  NetElementData<Maybe<String>> m_deathParticleBurst;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
  NetworkedAnimator::DynamicTarget m_humanoidDynamicTarget;

  std::shared_ptr<PlayerConfig> m_config;

  NetElementTopGroup m_netGroup;

  std::shared_ptr<ClientContext> m_clientContext;
  std::shared_ptr<Statistics> m_statistics;
  std::shared_ptr<QuestManager> m_questManager;

  std::shared_ptr<PlayerInventory> m_inventory;
  std::shared_ptr<PlayerBlueprints> m_blueprints;
  std::shared_ptr<PlayerUniverseMap> m_universeMap;
  std::shared_ptr<PlayerCodexes> m_codexes;
  std::shared_ptr<PlayerTech> m_techs;
  std::shared_ptr<PlayerCompanions> m_companions;
  std::shared_ptr<PlayerDeployment> m_deployment;
  std::shared_ptr<PlayerLog> m_log;

  UniverseClient* m_client; // required for celestial callbacks in scripts
  StringMap<std::shared_ptr<Player::GenericScriptComponent>> m_genericScriptContexts;
  JsonObject m_genericProperties;

  State m_state;
  HumanoidEmote m_emoteState;

  Maybe<String> m_dance;
  GameTimer m_danceCooldownTimer;

  float m_footstepTimer;
  float m_teleportTimer;
  GameTimer m_emoteCooldownTimer;
  GameTimer m_blinkCooldownTimer;
  float m_lastDamagedOtherTimer;
  EntityId m_lastDamagedTarget;
  GameTimer m_ageItemsTimer;

  float m_footstepVolumeVariance;
  float m_landingVolume;
  bool m_landingNoisePending;
  bool m_footstepPending;

  String m_teleportAnimationType;
  std::shared_ptr<NetworkedAnimator> m_effectsAnimator;
  NetworkedAnimator::DynamicTarget m_effectsAnimatorDynamicTarget;

  float m_emoteCooldown;
  Vec2F m_blinkInterval;

  HashSet<MoveControlType> m_pendingMoves;
  Vec2F m_moveVector;
  bool m_shifting;
  ActorMovementParameters m_zeroGMovementParameters;

  List<DamageSource> m_damageSources;

  String m_description;

  PlayerMode m_modeType;
  PlayerModeConfig m_modeConfig;
  ShipUpgrades m_shipUpgrades;
  String m_shipSpecies;

  std::shared_ptr<ToolUser> m_tools;
  std::shared_ptr<ArmorWearer> m_armor;
  HashMap<EquipmentSlot, std::uint64_t> m_armorSecretNetVersions;

  bool m_useDown;
  bool m_edgeTriggeredUse;

  Vec2F m_aimPosition;

  Maybe<EntityId> m_cameraFocusEntity;

  std::shared_ptr<ActorMovementController> m_movementController;
  std::shared_ptr<TechController> m_techController;
  std::shared_ptr<StatusController> m_statusController;

  float m_foodLowThreshold;
  List<PersistentStatusEffect> m_foodLowStatusEffects;
  List<PersistentStatusEffect> m_foodEmptyStatusEffects;

  List<PersistentStatusEffect> m_inCinematicStatusEffects;

  HumanoidIdentity m_identity;
  bool m_identityUpdated;

  bool m_isAdmin;
  float m_interactRadius; // hand interact radius
  Vec2F m_walkIntoInteractBias; // offset on position to find an interactable
  // when not pointing at
  // an interactable with the mouse

  List<RpcPromise<InteractAction>> m_pendingInteractActions;

  List<Particle> m_callbackParticles;
  List<std::tuple<String, float, float>> m_callbackSounds;

  List<String> m_queuedMessages;
  List<std::shared_ptr<Item>> m_queuedItemPickups;

  List<ChatAction> m_pendingChatActions;

  StringSet m_missionRadioMessages;
  bool m_interruptRadioMessage;
  List<std::pair<GameTimer, RadioMessage>> m_delayedRadioMessages;
  Deque<RadioMessage> m_pendingRadioMessages;
  Maybe<Json> m_pendingCinematic;
  Maybe<std::pair<Maybe<std::pair<StringList, int>>, float>> m_pendingAltMusic;
  Maybe<PlayerWarpRequest> m_pendingWarp;
  Deque<std::pair<Json, RpcPromiseKeeper<Json>>> m_pendingConfirmations;

  AiState m_aiState;

  String m_chatMessage;
  bool m_chatMessageChanged;
  bool m_chatMessageUpdated;

  std::shared_ptr<EffectEmitter> m_effectEmitter;

  std::shared_ptr<Songbook> m_songbook;

  int m_hitDamageNotificationLimiter;
  int m_hitDamageNotificationLimit;

  StringSet m_interestingObjects;

  NetElementUInt m_stateNetState;
  NetElementBool m_shiftingNetState;
  NetElementFloat m_xAimPositionNetState;
  NetElementFloat m_yAimPositionNetState;
  NetElementData<HumanoidIdentity> m_identityNetState;
  NetElementEvent m_refreshedHumanoidParameters;
  JsonObject m_humanoidParameters = JsonObject();
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementEvent m_landedNetState;
  NetElementString m_chatMessageNetState;
  NetElementEvent m_newChatMessageNetState;
  NetElementString m_emoteNetState;
  NetElementData<Maybe<String>> m_humanoidDanceNetState;
};

}

export module star.player;

export namespace Star {
  using ::Star::ArmorItem;
  using ::Star::ArmorWearer;
  using ::Star::ToolUser;
  using ::Star::EffectEmitter;
  using ::Star::PlayerConfig;
  using ::Star::Songbook;
  using ::Star::WireConnector;
  using ::Star::PlayerInventory;
  using ::Star::PlayerBlueprints;
  using ::Star::PlayerTech;
  using ::Star::PlayerCompanions;
  using ::Star::PlayerDeployment;
  using ::Star::PlayerLog;
  using ::Star::TechController;
  using ::Star::ClientContext;
  using ::Star::Statistics;
  using ::Star::StatusController;
  using ::Star::PlayerCodexes;
  using ::Star::QuestManager;
  using ::Star::InteractiveEntity;
  using ::Star::PlayerUniverseMap;
  using ::Star::UniverseClient;
  using ::Star::Player;
}
