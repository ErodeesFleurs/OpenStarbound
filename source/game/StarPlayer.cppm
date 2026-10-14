module;
import std;
import star.json;
import star.net_compatibility;
import star.byte_array;
import star.id_map;
import star.data_stream;
import star.bi_map;
#include "StarCasting.hpp"
import star.variant;
import star.rpc_promise;
#include "StarStrongTypedef.hpp"
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
#include "StarLuaRoot.hpp"
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
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;

import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_containers;
import star.net_element_dynamic_group;
import star.net_element_system;

namespace Star {

class ArmorItem;
using ArmorItemPtr = std::shared_ptr<ArmorItem>;
using ArmorItemConstPtr = std::shared_ptr<const ArmorItem>;
using ArmorItemWeakPtr = std::weak_ptr<ArmorItem>;
using ArmorItemConstWeakPtr = std::weak_ptr<const ArmorItem>;
using ArmorItemUPtr = std::unique_ptr<ArmorItem>;
using ArmorItemConstUPtr = std::unique_ptr<const ArmorItem>;
class ArmorWearer;
using ArmorWearerPtr = std::shared_ptr<ArmorWearer>;
using ArmorWearerConstPtr = std::shared_ptr<const ArmorWearer>;
using ArmorWearerWeakPtr = std::weak_ptr<ArmorWearer>;
using ArmorWearerConstWeakPtr = std::weak_ptr<const ArmorWearer>;
using ArmorWearerUPtr = std::unique_ptr<ArmorWearer>;
using ArmorWearerConstUPtr = std::unique_ptr<const ArmorWearer>;
class ToolUser;
using ToolUserPtr = std::shared_ptr<ToolUser>;
using ToolUserConstPtr = std::shared_ptr<const ToolUser>;
using ToolUserWeakPtr = std::weak_ptr<ToolUser>;
using ToolUserConstWeakPtr = std::weak_ptr<const ToolUser>;
using ToolUserUPtr = std::unique_ptr<ToolUser>;
using ToolUserConstUPtr = std::unique_ptr<const ToolUser>;
class EffectEmitter;
using EffectEmitterPtr = std::shared_ptr<EffectEmitter>;
using EffectEmitterConstPtr = std::shared_ptr<const EffectEmitter>;
using EffectEmitterWeakPtr = std::weak_ptr<EffectEmitter>;
using EffectEmitterConstWeakPtr = std::weak_ptr<const EffectEmitter>;
using EffectEmitterUPtr = std::unique_ptr<EffectEmitter>;
using EffectEmitterConstUPtr = std::unique_ptr<const EffectEmitter>;

struct PlayerConfig;
using PlayerConfigPtr = std::shared_ptr<PlayerConfig>;
using PlayerConfigConstPtr = std::shared_ptr<const PlayerConfig>;
using PlayerConfigWeakPtr = std::weak_ptr<PlayerConfig>;
using PlayerConfigConstWeakPtr = std::weak_ptr<const PlayerConfig>;
using PlayerConfigUPtr = std::unique_ptr<PlayerConfig>;
using PlayerConfigConstUPtr = std::unique_ptr<const PlayerConfig>;
class Songbook;
using SongbookPtr = std::shared_ptr<Songbook>;
using SongbookConstPtr = std::shared_ptr<const Songbook>;
using SongbookWeakPtr = std::weak_ptr<Songbook>;
using SongbookConstWeakPtr = std::weak_ptr<const Songbook>;
using SongbookUPtr = std::unique_ptr<Songbook>;
using SongbookConstUPtr = std::unique_ptr<const Songbook>;
class WireConnector;
using WireConnectorPtr = std::shared_ptr<WireConnector>;
using WireConnectorConstPtr = std::shared_ptr<const WireConnector>;
using WireConnectorWeakPtr = std::weak_ptr<WireConnector>;
using WireConnectorConstWeakPtr = std::weak_ptr<const WireConnector>;
using WireConnectorUPtr = std::unique_ptr<WireConnector>;
using WireConnectorConstUPtr = std::unique_ptr<const WireConnector>;
class PlayerInventory;
using PlayerInventoryPtr = std::shared_ptr<PlayerInventory>;
using PlayerInventoryConstPtr = std::shared_ptr<const PlayerInventory>;
using PlayerInventoryWeakPtr = std::weak_ptr<PlayerInventory>;
using PlayerInventoryConstWeakPtr = std::weak_ptr<const PlayerInventory>;
using PlayerInventoryUPtr = std::unique_ptr<PlayerInventory>;
using PlayerInventoryConstUPtr = std::unique_ptr<const PlayerInventory>;
class PlayerBlueprints;
using PlayerBlueprintsPtr = std::shared_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstPtr = std::shared_ptr<const PlayerBlueprints>;
using PlayerBlueprintsWeakPtr = std::weak_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstWeakPtr = std::weak_ptr<const PlayerBlueprints>;
using PlayerBlueprintsUPtr = std::unique_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstUPtr = std::unique_ptr<const PlayerBlueprints>;
class PlayerTech;
using PlayerTechPtr = std::shared_ptr<PlayerTech>;
using PlayerTechConstPtr = std::shared_ptr<const PlayerTech>;
using PlayerTechWeakPtr = std::weak_ptr<PlayerTech>;
using PlayerTechConstWeakPtr = std::weak_ptr<const PlayerTech>;
using PlayerTechUPtr = std::unique_ptr<PlayerTech>;
using PlayerTechConstUPtr = std::unique_ptr<const PlayerTech>;
class PlayerCompanions;
using PlayerCompanionsPtr = std::shared_ptr<PlayerCompanions>;
using PlayerCompanionsConstPtr = std::shared_ptr<const PlayerCompanions>;
using PlayerCompanionsWeakPtr = std::weak_ptr<PlayerCompanions>;
using PlayerCompanionsConstWeakPtr = std::weak_ptr<const PlayerCompanions>;
using PlayerCompanionsUPtr = std::unique_ptr<PlayerCompanions>;
using PlayerCompanionsConstUPtr = std::unique_ptr<const PlayerCompanions>;
class PlayerDeployment;
using PlayerDeploymentPtr = std::shared_ptr<PlayerDeployment>;
using PlayerDeploymentConstPtr = std::shared_ptr<const PlayerDeployment>;
using PlayerDeploymentWeakPtr = std::weak_ptr<PlayerDeployment>;
using PlayerDeploymentConstWeakPtr = std::weak_ptr<const PlayerDeployment>;
using PlayerDeploymentUPtr = std::unique_ptr<PlayerDeployment>;
using PlayerDeploymentConstUPtr = std::unique_ptr<const PlayerDeployment>;
class PlayerLog;
using PlayerLogPtr = std::shared_ptr<PlayerLog>;
using PlayerLogConstPtr = std::shared_ptr<const PlayerLog>;
using PlayerLogWeakPtr = std::weak_ptr<PlayerLog>;
using PlayerLogConstWeakPtr = std::weak_ptr<const PlayerLog>;
using PlayerLogUPtr = std::unique_ptr<PlayerLog>;
using PlayerLogConstUPtr = std::unique_ptr<const PlayerLog>;
class TechController;
using TechControllerPtr = std::shared_ptr<TechController>;
using TechControllerConstPtr = std::shared_ptr<const TechController>;
using TechControllerWeakPtr = std::weak_ptr<TechController>;
using TechControllerConstWeakPtr = std::weak_ptr<const TechController>;
using TechControllerUPtr = std::unique_ptr<TechController>;
using TechControllerConstUPtr = std::unique_ptr<const TechController>;
class ClientContext;
using ClientContextPtr = std::shared_ptr<ClientContext>;
using ClientContextConstPtr = std::shared_ptr<const ClientContext>;
using ClientContextWeakPtr = std::weak_ptr<ClientContext>;
using ClientContextConstWeakPtr = std::weak_ptr<const ClientContext>;
using ClientContextUPtr = std::unique_ptr<ClientContext>;
using ClientContextConstUPtr = std::unique_ptr<const ClientContext>;
class Statistics;
using StatisticsPtr = std::shared_ptr<Statistics>;
using StatisticsConstPtr = std::shared_ptr<const Statistics>;
using StatisticsWeakPtr = std::weak_ptr<Statistics>;
using StatisticsConstWeakPtr = std::weak_ptr<const Statistics>;
using StatisticsUPtr = std::unique_ptr<Statistics>;
using StatisticsConstUPtr = std::unique_ptr<const Statistics>;
class StatusController;
using StatusControllerPtr = std::shared_ptr<StatusController>;
using StatusControllerConstPtr = std::shared_ptr<const StatusController>;
using StatusControllerWeakPtr = std::weak_ptr<StatusController>;
using StatusControllerConstWeakPtr = std::weak_ptr<const StatusController>;
using StatusControllerUPtr = std::unique_ptr<StatusController>;
using StatusControllerConstUPtr = std::unique_ptr<const StatusController>;
class PlayerCodexes;
using PlayerCodexesPtr = std::shared_ptr<PlayerCodexes>;
using PlayerCodexesConstPtr = std::shared_ptr<const PlayerCodexes>;
using PlayerCodexesWeakPtr = std::weak_ptr<PlayerCodexes>;
using PlayerCodexesConstWeakPtr = std::weak_ptr<const PlayerCodexes>;
using PlayerCodexesUPtr = std::unique_ptr<PlayerCodexes>;
using PlayerCodexesConstUPtr = std::unique_ptr<const PlayerCodexes>;
class QuestManager;
using QuestManagerPtr = std::shared_ptr<QuestManager>;
using QuestManagerConstPtr = std::shared_ptr<const QuestManager>;
using QuestManagerWeakPtr = std::weak_ptr<QuestManager>;
using QuestManagerConstWeakPtr = std::weak_ptr<const QuestManager>;
using QuestManagerUPtr = std::unique_ptr<QuestManager>;
using QuestManagerConstUPtr = std::unique_ptr<const QuestManager>;
class InteractiveEntity;
using InteractiveEntityPtr = std::shared_ptr<InteractiveEntity>;
using InteractiveEntityConstPtr = std::shared_ptr<const InteractiveEntity>;
using InteractiveEntityWeakPtr = std::weak_ptr<InteractiveEntity>;
using InteractiveEntityConstWeakPtr = std::weak_ptr<const InteractiveEntity>;
using InteractiveEntityUPtr = std::unique_ptr<InteractiveEntity>;
using InteractiveEntityConstUPtr = std::unique_ptr<const InteractiveEntity>;
class PlayerUniverseMap;
using PlayerUniverseMapPtr = std::shared_ptr<PlayerUniverseMap>;
using PlayerUniverseMapConstPtr = std::shared_ptr<const PlayerUniverseMap>;
using PlayerUniverseMapWeakPtr = std::weak_ptr<PlayerUniverseMap>;
using PlayerUniverseMapConstWeakPtr = std::weak_ptr<const PlayerUniverseMap>;
using PlayerUniverseMapUPtr = std::unique_ptr<PlayerUniverseMap>;
using PlayerUniverseMapConstUPtr = std::unique_ptr<const PlayerUniverseMap>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<const UniverseClient>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<const UniverseClient>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<const UniverseClient>;

class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<const Player>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<const Player>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<const Player>;

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

  Player(PlayerConfigPtr config, Uuid uuid = Uuid());
  Player(PlayerConfigPtr config, ByteArray const& netStore, NetCompatibilityRules rules = {});
  Player(PlayerConfigPtr config, Json const& diskStore);

  void diskLoad(Json const& diskStore);

  ClientContextPtr clientContext() const;
  void setClientContext(ClientContextPtr clientContext);

  StatisticsPtr statistics() const;
  void setStatistics(StatisticsPtr statistics);

  void setUniverseClient(UniverseClient* universeClient);
  UniverseClient* universeClient() const;

  QuestManagerPtr questManager() const;

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

  PlayerInventoryPtr inventory() const;
  // Returns the number of items from this stack that could be
  // picked up from the world, using inventory tab filtering
  std::uint64_t itemsCanHold(ItemPtr const& items) const;
  // Adds items to the inventory, returning the overflow.
  // The items parameter is invalid after use.
  ItemPtr pickupItems(ItemPtr const& items, bool silent = false);
  // Pick up all of the given items as possible, dropping the overflow.
  // The item parameter is invalid after use.
  void giveItem(ItemPtr const& item);

  void triggerPickupEvents(ItemPtr const& item);

  ItemPtr essentialItem(EssentialItem essentialItem) const;
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

  InteractiveEntityPtr bestInteractionEntity(bool includeNearby);
  void interactWithEntity(InteractiveEntityPtr entity);

  // Aim this player's target at the given world position.
  void aim(Vec2F const& position);
  Vec2F aimPosition() const override;

  Vec2F armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const override;
  Vec2F handOffset(ToolHand hand, Direction facingDirection) const override;

  Vec2F handPosition(ToolHand hand, Vec2F const& handOffset = {}) const override;
  ItemPtr handItem(ToolHand hand) const override;

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

  ItemPtr primaryHandItem() const;
  ItemPtr altHandItem() const;

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

  HumanoidPtr humanoid();
  HumanoidPtr humanoid() const;
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
  List<ItemPtr> pullQueuedItemDrops();

  void queueUIMessage(String const& message) override;
  void queueItemPickupMessage(ItemPtr const& item);

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

  StatusControllerPtr statusControllerPtr();
  ActorMovementControllerPtr movementControllerPtr();

  PlayerConfigPtr config();

  SongbookPtr songbook() const;

  void finalizeCreation();

  float timeSinceLastGaveDamage() const;
  EntityId lastDamagedTarget() const;

  bool invisible() const;

  void animatePortrait(float dt);

  bool isOutside();

  void dropSelectedItems(std::function<bool(ItemPtr)> filter);
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
  EntityHighlightEffect inspectionHighlight(InspectableEntityPtr const& inspectableEntity) const;

  Vec2F cameraPosition();

  using Entity::setTeam;

  NetworkedAnimatorPtr effectsAnimator();

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
  typedef std::shared_ptr<GenericScriptComponent> GenericScriptComponentPtr;

  typedef std::function<void(ItemPtr)> ItemSetFunc;

  // handle input and other events (master only) that happen BEFORE movement/tech controller updates
  void processControls();

  // state changes and effect animations (master and slave) that happen AFTER movement/tech controller updates
  void processStateChanges(float dt);

  void getNetStates(bool initial);
  void setNetStates();
  void getNetArmorSecrets();
  void setNetArmorSecret(EquipmentSlot slot, ArmorItemPtr const& armor, bool visible = true);
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

  PlayerConfigPtr m_config;

  NetElementTopGroup m_netGroup;

  ClientContextPtr m_clientContext;
  StatisticsPtr m_statistics;
  QuestManagerPtr m_questManager;

  PlayerInventoryPtr m_inventory;
  PlayerBlueprintsPtr m_blueprints;
  PlayerUniverseMapPtr m_universeMap;
  PlayerCodexesPtr m_codexes;
  PlayerTechPtr m_techs;
  PlayerCompanionsPtr m_companions;
  PlayerDeploymentPtr m_deployment;
  PlayerLogPtr m_log;

  UniverseClient* m_client; // required for celestial callbacks in scripts
  StringMap<GenericScriptComponentPtr> m_genericScriptContexts;
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
  NetworkedAnimatorPtr m_effectsAnimator;
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

  ToolUserPtr m_tools;
  ArmorWearerPtr m_armor;
  HashMap<EquipmentSlot, std::uint64_t> m_armorSecretNetVersions;

  bool m_useDown;
  bool m_edgeTriggeredUse;

  Vec2F m_aimPosition;

  Maybe<EntityId> m_cameraFocusEntity;

  ActorMovementControllerPtr m_movementController;
  TechControllerPtr m_techController;
  StatusControllerPtr m_statusController;

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
  List<ItemPtr> m_queuedItemPickups;

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

  EffectEmitterPtr m_effectEmitter;

  SongbookPtr m_songbook;

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
  using ::Star::ArmorItemPtr;
  using ::Star::ArmorItemConstPtr;
  using ::Star::ArmorItemWeakPtr;
  using ::Star::ArmorItemConstWeakPtr;
  using ::Star::ArmorItemUPtr;
  using ::Star::ArmorItemConstUPtr;
  using ::Star::ArmorWearer;
  using ::Star::ArmorWearerPtr;
  using ::Star::ArmorWearerConstPtr;
  using ::Star::ArmorWearerWeakPtr;
  using ::Star::ArmorWearerConstWeakPtr;
  using ::Star::ArmorWearerUPtr;
  using ::Star::ArmorWearerConstUPtr;
  using ::Star::ToolUser;
  using ::Star::ToolUserPtr;
  using ::Star::ToolUserConstPtr;
  using ::Star::ToolUserWeakPtr;
  using ::Star::ToolUserConstWeakPtr;
  using ::Star::ToolUserUPtr;
  using ::Star::ToolUserConstUPtr;
  using ::Star::EffectEmitter;
  using ::Star::EffectEmitterPtr;
  using ::Star::EffectEmitterConstPtr;
  using ::Star::EffectEmitterWeakPtr;
  using ::Star::EffectEmitterConstWeakPtr;
  using ::Star::EffectEmitterUPtr;
  using ::Star::EffectEmitterConstUPtr;
  using ::Star::PlayerConfig;
  using ::Star::PlayerConfigPtr;
  using ::Star::PlayerConfigConstPtr;
  using ::Star::PlayerConfigWeakPtr;
  using ::Star::PlayerConfigConstWeakPtr;
  using ::Star::PlayerConfigUPtr;
  using ::Star::PlayerConfigConstUPtr;
  using ::Star::Songbook;
  using ::Star::SongbookPtr;
  using ::Star::SongbookConstPtr;
  using ::Star::SongbookWeakPtr;
  using ::Star::SongbookConstWeakPtr;
  using ::Star::SongbookUPtr;
  using ::Star::SongbookConstUPtr;
  using ::Star::WireConnector;
  using ::Star::WireConnectorPtr;
  using ::Star::WireConnectorConstPtr;
  using ::Star::WireConnectorWeakPtr;
  using ::Star::WireConnectorConstWeakPtr;
  using ::Star::WireConnectorUPtr;
  using ::Star::WireConnectorConstUPtr;
  using ::Star::PlayerInventory;
  using ::Star::PlayerInventoryPtr;
  using ::Star::PlayerInventoryConstPtr;
  using ::Star::PlayerInventoryWeakPtr;
  using ::Star::PlayerInventoryConstWeakPtr;
  using ::Star::PlayerInventoryUPtr;
  using ::Star::PlayerInventoryConstUPtr;
  using ::Star::PlayerBlueprints;
  using ::Star::PlayerBlueprintsPtr;
  using ::Star::PlayerBlueprintsConstPtr;
  using ::Star::PlayerBlueprintsWeakPtr;
  using ::Star::PlayerBlueprintsConstWeakPtr;
  using ::Star::PlayerBlueprintsUPtr;
  using ::Star::PlayerBlueprintsConstUPtr;
  using ::Star::PlayerTech;
  using ::Star::PlayerTechPtr;
  using ::Star::PlayerTechConstPtr;
  using ::Star::PlayerTechWeakPtr;
  using ::Star::PlayerTechConstWeakPtr;
  using ::Star::PlayerTechUPtr;
  using ::Star::PlayerTechConstUPtr;
  using ::Star::PlayerCompanions;
  using ::Star::PlayerCompanionsPtr;
  using ::Star::PlayerCompanionsConstPtr;
  using ::Star::PlayerCompanionsWeakPtr;
  using ::Star::PlayerCompanionsConstWeakPtr;
  using ::Star::PlayerCompanionsUPtr;
  using ::Star::PlayerCompanionsConstUPtr;
  using ::Star::PlayerDeployment;
  using ::Star::PlayerDeploymentPtr;
  using ::Star::PlayerDeploymentConstPtr;
  using ::Star::PlayerDeploymentWeakPtr;
  using ::Star::PlayerDeploymentConstWeakPtr;
  using ::Star::PlayerDeploymentUPtr;
  using ::Star::PlayerDeploymentConstUPtr;
  using ::Star::PlayerLog;
  using ::Star::PlayerLogPtr;
  using ::Star::PlayerLogConstPtr;
  using ::Star::PlayerLogWeakPtr;
  using ::Star::PlayerLogConstWeakPtr;
  using ::Star::PlayerLogUPtr;
  using ::Star::PlayerLogConstUPtr;
  using ::Star::TechController;
  using ::Star::TechControllerPtr;
  using ::Star::TechControllerConstPtr;
  using ::Star::TechControllerWeakPtr;
  using ::Star::TechControllerConstWeakPtr;
  using ::Star::TechControllerUPtr;
  using ::Star::TechControllerConstUPtr;
  using ::Star::ClientContext;
  using ::Star::ClientContextPtr;
  using ::Star::ClientContextConstPtr;
  using ::Star::ClientContextWeakPtr;
  using ::Star::ClientContextConstWeakPtr;
  using ::Star::ClientContextUPtr;
  using ::Star::ClientContextConstUPtr;
  using ::Star::Statistics;
  using ::Star::StatisticsPtr;
  using ::Star::StatisticsConstPtr;
  using ::Star::StatisticsWeakPtr;
  using ::Star::StatisticsConstWeakPtr;
  using ::Star::StatisticsUPtr;
  using ::Star::StatisticsConstUPtr;
  using ::Star::StatusController;
  using ::Star::StatusControllerPtr;
  using ::Star::StatusControllerConstPtr;
  using ::Star::StatusControllerWeakPtr;
  using ::Star::StatusControllerConstWeakPtr;
  using ::Star::StatusControllerUPtr;
  using ::Star::StatusControllerConstUPtr;
  using ::Star::PlayerCodexes;
  using ::Star::PlayerCodexesPtr;
  using ::Star::PlayerCodexesConstPtr;
  using ::Star::PlayerCodexesWeakPtr;
  using ::Star::PlayerCodexesConstWeakPtr;
  using ::Star::PlayerCodexesUPtr;
  using ::Star::PlayerCodexesConstUPtr;
  using ::Star::QuestManager;
  using ::Star::QuestManagerPtr;
  using ::Star::QuestManagerConstPtr;
  using ::Star::QuestManagerWeakPtr;
  using ::Star::QuestManagerConstWeakPtr;
  using ::Star::QuestManagerUPtr;
  using ::Star::QuestManagerConstUPtr;
  using ::Star::InteractiveEntity;
  using ::Star::InteractiveEntityPtr;
  using ::Star::InteractiveEntityConstPtr;
  using ::Star::InteractiveEntityWeakPtr;
  using ::Star::InteractiveEntityConstWeakPtr;
  using ::Star::InteractiveEntityUPtr;
  using ::Star::InteractiveEntityConstUPtr;
  using ::Star::PlayerUniverseMap;
  using ::Star::PlayerUniverseMapPtr;
  using ::Star::PlayerUniverseMapConstPtr;
  using ::Star::PlayerUniverseMapWeakPtr;
  using ::Star::PlayerUniverseMapConstWeakPtr;
  using ::Star::PlayerUniverseMapUPtr;
  using ::Star::PlayerUniverseMapConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
}
