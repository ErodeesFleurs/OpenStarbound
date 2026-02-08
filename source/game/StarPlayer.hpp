#pragma once

#include "StarActorMovementController.hpp"
#include "StarAiTypes.hpp"
#include "StarArmorWearer.hpp"
#include "StarChattyEntity.hpp"
#include "StarConfig.hpp"
#include "StarDamageBarEntity.hpp"
#include "StarEmoteEntity.hpp"
#include "StarEntityRendering.hpp"
#include "StarHumanoid.hpp"
#include "StarInspectableEntity.hpp"
#include "StarInventoryTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLoungingEntities.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNametagEntity.hpp"
#include "StarNetElementDynamicGroup.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarPlayerTypes.hpp"
#include "StarPortraitEntity.hpp"
#include "StarRadioMessageDatabase.hpp"
#include "StarToolUser.hpp"
#include "StarToolUserEntity.hpp"
#include "StarUuid.hpp"

import std;

namespace Star {

class ClientContext;
class Statistics;
class UniverseClient;
class QuestManager;
class PlayerInventory;
class PlayerBlueprints;
class PlayerUniverseMap;
class PlayerCodexes;
class PlayerTech;
class PlayerCompanions;
class PlayerLog;
class InteractiveEntity;
class WireConnector;
class Songbook;
class PlayerDeployment;
class TechController;

struct PlayerConfig;

class Player : public virtual ToolUserEntity,
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

  Player(Ptr<PlayerConfig> config, Uuid uuid = Uuid());
  Player(Ptr<PlayerConfig> config, ByteArray const& netStore, NetCompatibilityRules rules = {});
  Player(Ptr<PlayerConfig> config, Json const& diskStore);

  void diskLoad(Json const& diskStore);

  auto clientContext() const -> Ptr<ClientContext>;
  void setClientContext(Ptr<ClientContext> clientContext);

  auto statistics() const -> Ptr<Statistics>;
  void setStatistics(Ptr<Statistics> statistics);

  void setUniverseClient(UniverseClient* universeClient);
  auto universeClient() const -> UniverseClient*;

  auto questManager() const -> Ptr<QuestManager>;

  auto diskStore() -> Json;
  auto netStore(NetCompatibilityRules rules = {}) -> ByteArray;

  auto entityType() const -> EntityType override;
  auto clientEntityMode() const -> ClientEntityMode override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto position() const -> Vec2F override;
  auto velocity() const -> Vec2F override;

  auto mouthPosition() const -> Vec2F override;
  auto mouthPosition(bool ignoreAdjustments) const -> Vec2F override;
  auto mouthOffset(bool ignoreAdjustments = true) const -> Vec2F;
  auto feetOffset() const -> Vec2F;
  auto headArmorOffset() const -> Vec2F;
  auto chestArmorOffset() const -> Vec2F;
  auto legsArmorOffset() const -> Vec2F;
  auto backArmorOffset() const -> Vec2F;

  // relative to current position
  auto metaBoundBox() const -> RectF override;

  // relative to current position
  auto collisionArea() const -> RectF override;

  auto writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, uint64_t> override;
  void readNetState(ByteArray data, float interpolationStep = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  auto queryHit(DamageSource const& source) const -> std::optional<HitType> override;
  auto hitPoly() const -> std::optional<PolyF> override;

  auto applyDamage(DamageRequest const& damage) -> List<DamageNotification> override;
  auto selfDamageNotifications() -> List<DamageNotification> override;

  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;

  auto damageSources() const -> List<DamageSource> override;

  auto shouldDestroy() const -> bool override;
  void destroy(RenderCallback* renderCallback) override;

  auto loungingIn() const -> std::optional<EntityAnchorState> override;
  auto lounge(EntityId loungeableEntityId, size_t anchorIndex) -> bool;
  void stopLounging();

  void revive(Vec2F const& footPosition);

  auto portrait(PortraitMode mode) const -> List<Drawable> override;
  auto underwater() const -> bool;

  auto shifting() const -> bool;
  void setShifting(bool shifting);
  void special(int specialKey);

  void setMoveVector(Vec2F const& vec);
  void moveLeft();
  void moveRight();
  void moveUp();
  void moveDown();
  void jump();

  void dropItem();

  auto toolRadius() const -> float;
  auto interactRadius() const -> float override;
  void setInteractRadius(float interactRadius);
  auto pullInteractActions() -> List<InteractAction>;

  auto currency(String const& currencyType) const -> uint64_t;

  auto health() const -> float override;
  auto maxHealth() const -> float override;
  auto damageBar() const -> DamageBarType override;
  auto healthPercentage() const -> float;

  auto energy() const -> float override;
  auto maxEnergy() const -> float;
  auto energyPercentage() const -> float;

  auto energyRegenBlockPercent() const -> float;

  auto energyLocked() const -> bool override;
  auto fullEnergy() const -> bool override;
  auto consumeEnergy(float energy) -> bool override;

  auto foodPercentage() const -> float;

  auto breath() const -> float;
  auto maxBreath() const -> float;

  auto protection() const -> float;

  auto forceNude() const -> bool;

  auto description() const -> String override;
  void setDescription(String const& description);

  auto lightSources() const -> List<LightSource> override;

  auto walkingDirection() const -> Direction override;
  auto facingDirection() const -> Direction override;

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) -> std::optional<Json> override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  auto getGenericProperty(String const& name, Json const& defaultValue = Json()) const -> Json;
  void setGenericProperty(String const& name, Json const& value);

  auto inventory() const -> Ptr<PlayerInventory>;
  // Returns the number of items from this stack that could be
  // picked up from the world, using inventory tab filtering
  auto itemsCanHold(Ptr<Item> const& items) const -> uint64_t;
  // Adds items to the inventory, returning the overflow.
  // The items parameter is invalid after use.
  auto pickupItems(Ptr<Item> const& items, bool silent = false) -> Ptr<Item>;
  // Pick up all of the given items as possible, dropping the overflow.
  // The item parameter is invalid after use.
  void giveItem(Ptr<Item> const& item);

  void triggerPickupEvents(Ptr<Item> const& item);

  auto essentialItem(EssentialItem essentialItem) const -> Ptr<Item>;
  auto hasItem(ItemDescriptor const& descriptor, bool exactMatch = false) const -> bool;
  auto hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch = false) const -> uint64_t;
  // altough multiple entries may match, they might have different
  // serializations
  auto takeItem(ItemDescriptor const& descriptor, bool consumePartial = false, bool exactMatch = false) -> ItemDescriptor;
  void giveItem(ItemDescriptor const& descriptor);

  // Clear the item swap slot.
  void clearSwap();

  void refreshItems();
  void refreshArmor();
  void refreshHumanoid() const;
  // Refresh worn equipment from the inventory
  void refreshEquipment();

  auto blueprints() const -> Ptr<PlayerBlueprints>;
  auto addBlueprint(ItemDescriptor const& descriptor, bool showFailure = false) -> bool;
  auto blueprintKnown(ItemDescriptor const& descriptor) const -> bool;

  auto addCollectable(String const& collectionName, String const& collectableName) -> bool;

  auto universeMap() const -> Ptr<PlayerUniverseMap>;

  auto codexes() const -> Ptr<PlayerCodexes>;

  auto techs() const -> Ptr<PlayerTech>;
  void overrideTech(std::optional<StringList> const& techModules);
  auto techOverridden() const -> bool;

  auto companions() const -> Ptr<PlayerCompanions>;

  auto log() const -> Ptr<PlayerLog>;

  auto bestInteractionEntity(bool includeNearby) -> Ptr<InteractiveEntity>;
  void interactWithEntity(Ptr<InteractiveEntity> entity);

  // Aim this player's target at the given world position.
  void aim(Vec2F const& position);
  auto aimPosition() const -> Vec2F override;

  auto armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const -> Vec2F override;
  auto handOffset(ToolHand hand, Direction facingDirection) const -> Vec2F override;

  auto handPosition(ToolHand hand, Vec2F const& handOffset = {}) const -> Vec2F override;
  auto handItem(ToolHand hand) const -> Ptr<Item> override;

  auto armAdjustment() const -> Vec2F override;

  void setCameraFocusEntity(std::optional<EntityId> const& cameraFocusEntity) override;

  void playEmote(HumanoidEmote emote) override;

  auto canUseTool() const -> bool;

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

  auto primaryHandItem() const -> Ptr<Item>;
  auto altHandItem() const -> Ptr<Item>;

  auto uuid() const -> Uuid;

  auto modeType() const -> PlayerMode;
  void setModeType(PlayerMode mode);
  auto modeConfig() const -> PlayerModeConfig;

  auto shipUpgrades() -> ShipUpgrades;
  void setShipUpgrades(ShipUpgrades shipUpgrades);
  void applyShipUpgrades(Json const& upgrades);
  void setShipSpecies(String species);
  auto shipSpecies() const -> String;

  auto name() const -> String override;
  void setName(String const& name);

  auto statusText() const -> std::optional<String> override;
  auto displayNametag() const -> bool override;
  auto nametagColor() const -> Vec3B override;
  auto nametagOrigin() const -> Vec2F override;
  auto nametag() const -> String override;
  void setNametag(std::optional<String> nametag);

  void updateIdentity();

  void setHumanoidParameter(String key, std::optional<Json> value);
  auto getHumanoidParameter(String key) -> std::optional<Json>;
  void setHumanoidParameters(JsonObject parameters);
  auto getHumanoidParameters() -> JsonObject;
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

  void setHair(String const& group, String const& type, String const& directives);
  void setFacialHair(String const& group, String const& type, String const& directives);
  void setFacialMask(String const& group, String const& type, String const& directives);

  auto species() const -> String override;
  void setSpecies(String const& species);
  auto gender() const -> Gender;
  void setGender(Gender const& gender);
  void setPersonality(Personality const& personality);
  void setImagePath(std::optional<String> const& imagePath);

  auto humanoid() -> Ptr<Humanoid>;
  auto humanoid() const -> Ptr<Humanoid>;
  auto identity() const -> HumanoidIdentity const&;

  void setIdentity(HumanoidIdentity identity);

  void setAdmin(bool isAdmin);
  auto isAdmin() const -> bool override;

  auto inToolRange() const -> bool override;
  auto inToolRange(Vec2F const& aimPos) const -> bool override;
  auto inInteractionRange() const -> bool;
  auto inInteractionRange(Vec2F aimPos) const -> bool;

  void addParticles(List<Particle> const& particles) override;
  void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) override;

  auto wireToolInUse() const -> bool;
  void setWireConnector(WireConnector* wireConnector) const;

  void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) override;
  auto activeUniqueStatusEffectSummary() const -> ActiveUniqueStatusEffectSummary override;

  auto powerMultiplier() const -> float override;

  auto isDead() const -> bool;
  void kill();

  void setFavoriteColor(Color color);
  auto favoriteColor() const -> Color override;

  // Starts the teleport animation sequence, locking player movement and
  // preventing some update code
  void teleportOut(String const& animationType = "default", bool deploy = false);
  void teleportIn();
  void teleportAbort();

  auto isTeleporting() const -> bool;
  auto isTeleportingOut() const -> bool;
  auto canDeploy() -> bool;
  void deployAbort(String const& animationType = "default");
  auto isDeploying() const -> bool;
  auto isDeployed() const -> bool;

  void setBusyState(PlayerBusyState busyState);

  // A hard move to a specified location
  void moveTo(Vec2F const& footPosition);

  auto pullQueuedMessages() -> List<String>;
  auto pullQueuedItemDrops() -> List<Ptr<Item>>;

  void queueUIMessage(String const& message) override;
  void queueItemPickupMessage(Ptr<Item> const& item);

  void addChatMessage(String const& message, Json const& config = {});
  void addEmote(HumanoidEmote const& emote, std::optional<float> emoteCooldown = {});
  void setDance(std::optional<String> const& danceName);
  auto currentEmote() const -> std::pair<HumanoidEmote, float>;

  auto currentState() const -> State;

  auto pullPendingChatActions() -> List<ChatAction> override;

  auto inspectionLogName() const -> std::optional<String> override;
  auto inspectionDescription(String const& species) const -> std::optional<String> override;

  auto beamGunRadius() const -> float override;

  auto instrumentPlaying() -> bool override;
  void instrumentEquipped(String const& instrumentKind) override;
  void interact(InteractAction const& action) override;
  void addEffectEmitters(StringSet const& emitters) override;
  void requestEmote(String const& emote) override;

  auto movementController() -> ActorMovementController* override;
  auto statusController() -> StatusController* override;

  auto forceRegions() const -> List<PhysicsForceRegion> override;

  auto statusControllerPtr() -> Ptr<StatusController>;
  auto movementControllerPtr() -> Ptr<ActorMovementController>;

  auto config() -> Ptr<PlayerConfig>;

  auto songbook() const -> Ptr<Songbook>;

  void finalizeCreation();

  auto timeSinceLastGaveDamage() const -> float;
  auto lastDamagedTarget() const -> EntityId;

  auto invisible() const -> bool;

  void animatePortrait(float dt);

  auto isOutside() -> bool;

  void dropSelectedItems(std::function<bool(Ptr<Item>)> filter);
  void dropEverything();

  auto isPermaDead() const -> bool;

  auto interruptRadioMessage() -> bool;
  auto pullPendingRadioMessage() -> std::optional<RadioMessage>;
  void queueRadioMessage(Json const& messageConfig, float delay = 0);
  void queueRadioMessage(RadioMessage message);

  // If a cinematic should play, returns it and clears it.  May stop cinematics
  // by returning a null Json.
  auto pullPendingCinematic() -> std::optional<Json>;
  void setPendingCinematic(Json const& cinematic, bool unique = false);

  void setInCinematic(bool inCinematic);

  auto pullPendingAltMusic() -> std::optional<std::pair<std::optional<std::pair<StringList, int>>, float>>;

  auto pullPendingWarp() -> std::optional<PlayerWarpRequest>;
  void setPendingWarp(String const& action, std::optional<String> const& animation = {}, bool deploy = false);

  auto pullPendingConfirmation() -> std::optional<std::pair<Json, RpcPromiseKeeper<Json>>>;
  void queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise);

  auto aiState() const -> AiState const&;
  auto aiState() -> AiState&;

  // In inspection mode, scannable, scanned, and interesting objects will be
  // rendered with special highlighting.
  auto inspecting() const -> bool;

  // Will return the highlight effect to give an inspectable entity when inspecting
  auto inspectionHighlight(Ptr<InspectableEntity> const& inspectableEntity) const -> EntityHighlightEffect;

  auto cameraPosition() -> Vec2F;

  using Entity::setTeam;

  auto effectsAnimator() -> Ptr<NetworkedAnimator>;

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
  auto getSecretPropertyView(String const& name) const -> std::optional<StringView>;
  auto getSecretPropertyPtr(String const& name) const -> String const*;
  // Gets a secret Json property. It will be de-serialized.
  auto getSecretProperty(String const& name, Json defaultValue = Json()) const -> Json;
  // Sets a secret Json property. It will be serialized.
  void setSecretProperty(String const& name, Json const& value);

  void setAnimationParameter(String name, Json value);

private:
  using GenericScriptComponent = LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>>;

  using ItemSetFunc = std::function<void(Ptr<Item>)>;

  // handle input and other events (master only) that happen BEFORE movement/tech controller updates
  void processControls();

  // state changes and effect animations (master and slave) that happen AFTER movement/tech controller updates
  void processStateChanges(float dt);

  void getNetStates(bool initial);
  void setNetStates();
  void getNetArmorSecrets();
  void setNetArmorSecret(EquipmentSlot slot, Ptr<ArmorItem> const& armor, bool visible = true);
  void setNetArmorSecrets(bool includeEmpty = false);

  auto drawables() const -> List<Drawable>;
  auto bars() const -> List<OverheadBar>;
  auto particles() -> List<Particle>;
  auto getFootstepSound(Vec2I const& sensor) const -> String;

  void tickShared(float dt);

  auto detectEmotes(String const& chatter) -> HumanoidEmote;

  NetElementDynamicGroup<NetHumanoid> m_netHumanoid;
  NetElementData<std::optional<String>> m_deathParticleBurst;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
  NetworkedAnimator::DynamicTarget m_humanoidDynamicTarget;

  Ptr<PlayerConfig> m_config;

  NetElementTopGroup m_netGroup;

  Ptr<ClientContext> m_clientContext;
  Ptr<Statistics> m_statistics;
  Ptr<QuestManager> m_questManager;

  Ptr<PlayerInventory> m_inventory;
  Ptr<PlayerBlueprints> m_blueprints;
  Ptr<PlayerUniverseMap> m_universeMap;
  Ptr<PlayerCodexes> m_codexes;
  Ptr<PlayerTech> m_techs;
  Ptr<PlayerCompanions> m_companions;
  Ptr<PlayerDeployment> m_deployment;
  Ptr<PlayerLog> m_log;

  UniverseClient* m_client;// required for celestial callbacks in scripts
  StringMap<Ptr<GenericScriptComponent>> m_genericScriptContexts;
  JsonObject m_genericProperties;

  State m_state;
  HumanoidEmote m_emoteState;

  std::optional<String> m_dance;
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
  Ptr<NetworkedAnimator> m_effectsAnimator;
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

  Ptr<ToolUser> m_tools;
  Ptr<ArmorWearer> m_armor;
  HashMap<EquipmentSlot, uint64_t> m_armorSecretNetVersions;

  bool m_useDown;
  bool m_edgeTriggeredUse;

  Vec2F m_aimPosition;

  std::optional<EntityId> m_cameraFocusEntity;

  Ptr<ActorMovementController> m_movementController;
  Ptr<TechController> m_techController;
  Ptr<StatusController> m_statusController;

  float m_foodLowThreshold;
  List<PersistentStatusEffect> m_foodLowStatusEffects;
  List<PersistentStatusEffect> m_foodEmptyStatusEffects;

  List<PersistentStatusEffect> m_inCinematicStatusEffects;

  HumanoidIdentity m_identity;
  bool m_identityUpdated;

  bool m_isAdmin;
  float m_interactRadius;      // hand interact radius
  Vec2F m_walkIntoInteractBias;// offset on position to find an interactable
  // when not pointing at
  // an interactable with the mouse

  List<RpcPromise<InteractAction>> m_pendingInteractActions;

  List<Particle> m_callbackParticles;
  List<std::tuple<String, float, float>> m_callbackSounds;

  List<String> m_queuedMessages;
  List<Ptr<Item>> m_queuedItemPickups;

  List<ChatAction> m_pendingChatActions;

  StringSet m_missionRadioMessages;
  bool m_interruptRadioMessage;
  List<std::pair<GameTimer, RadioMessage>> m_delayedRadioMessages;
  Deque<RadioMessage> m_pendingRadioMessages;
  std::optional<Json> m_pendingCinematic;
  std::optional<std::pair<std::optional<std::pair<StringList, int>>, float>> m_pendingAltMusic;
  std::optional<PlayerWarpRequest> m_pendingWarp;
  Deque<std::pair<Json, RpcPromiseKeeper<Json>>> m_pendingConfirmations;

  AiState m_aiState;

  String m_chatMessage;
  bool m_chatMessageChanged;
  bool m_chatMessageUpdated;

  Ptr<EffectEmitter> m_effectEmitter;

  Ptr<Songbook> m_songbook;

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
  NetElementData<std::optional<String>> m_humanoidDanceNetState;
};

}// namespace Star
