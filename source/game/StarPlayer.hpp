#pragma once

#include "StarActorMovementController.hpp"
#include "StarAiTypes.hpp"
#include "StarArmorWearer.hpp"
#include "StarAssets.hpp"
#include "StarChattyEntity.hpp"
#include "StarConfiguration.hpp"
#include "StarDamageBarEntity.hpp"
#include "StarEmoteEntity.hpp"
#include "StarEntityFactory.hpp"
#include "StarEntityRendering.hpp"
#include "StarHumanoid.hpp"
#include "StarInspectableEntity.hpp"
#include "StarInventoryTypes.hpp"
#include "StarItemBag.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLoungingEntities.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarNametagEntity.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarPlayerAppearance.hpp"
#include "StarPlayerTypes.hpp"
#include "StarPortraitEntity.hpp"
#include "StarRadioMessageDatabase.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarToolUser.hpp"
#include "StarToolUserEntity.hpp"
#include "StarUuid.hpp"

namespace Star {

struct PlayerConfig;
using PlayerConfigPtr = SharedPtr<PlayerConfig>;
class Songbook;
using SongbookPtr = SharedPtr<Songbook>;
class WireConnector;
class PlayerInventory;
using PlayerInventoryPtr = SharedPtr<PlayerInventory>;
class PlayerBlueprints;
using PlayerBlueprintsPtr = SharedPtr<PlayerBlueprints>;
class PlayerTech;
using PlayerTechPtr = SharedPtr<PlayerTech>;
class TechDatabase;
using TechDatabaseConstPtr = SharedPtr<TechDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class QuestTemplateDatabase;
using QuestTemplateDatabaseConstPtr = SharedPtr<QuestTemplateDatabase const>;
class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;
class CodexDatabase;
using CodexDatabaseConstPtr = SharedPtr<CodexDatabase const>;
class DanceDatabase;
using DanceDatabaseConstPtr = SharedPtr<DanceDatabase const>;
class EmoteProcessor;
using EmoteProcessorConstPtr = SharedPtr<EmoteProcessor const>;
class AiDatabase;
using AiDatabaseConstPtr = SharedPtr<AiDatabase const>;
class CollectionDatabase;
using CollectionDatabaseConstPtr = SharedPtr<CollectionDatabase const>;
class PlayerCompanions;
using PlayerCompanionsPtr = SharedPtr<PlayerCompanions>;
class PlayerDeployment;
using PlayerDeploymentPtr = SharedPtr<PlayerDeployment>;
class PlayerLog;
using PlayerLogPtr = SharedPtr<PlayerLog>;
class TechController;
using TechControllerPtr = SharedPtr<TechController>;
class ClientContext;
using ClientContextPtr = SharedPtr<ClientContext>;
class Statistics;
using StatisticsPtr = SharedPtr<Statistics>;
class StatusController;
using StatusControllerPtr = SharedPtr<StatusController>;
class PlayerCodexes;
using PlayerCodexesPtr = SharedPtr<PlayerCodexes>;
class QuestManager;
using QuestManagerPtr = SharedPtr<QuestManager>;
class InteractiveEntity;
using InteractiveEntityPtr = SharedPtr<InteractiveEntity>;
class PlayerUniverseMap;
using PlayerUniverseMapPtr = SharedPtr<PlayerUniverseMap>;
class UniverseClient;
class PlayerNarrativeQueue;
using PlayerNarrativeQueuePtr = SharedPtr<PlayerNarrativeQueue>;
class PlayerChatAndEmotes;
using PlayerChatAndEmotesPtr = SharedPtr<PlayerChatAndEmotes>;
class PlayerDamagePipeline;
using PlayerDamagePipelinePtr = SharedPtr<PlayerDamagePipeline>;
class PlayerTeleporter;
using PlayerTeleporterPtr = SharedPtr<PlayerTeleporter>;

class Player;
using PlayerPtr = SharedPtr<Player>;

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

  Player(PlayerConfigPtr config, Uuid uuid, AssetsConstPtr assets, ConfigurationPtr configuration, MaterialDatabaseConstPtr materialDatabase, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase, CodexDatabaseConstPtr codexDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, RadioMessageDatabaseConstPtr radioMessageDatabase, AiDatabaseConstPtr aiDatabase, CollectionDatabaseConstPtr collectionDatabase, SpeciesDatabaseConstPtr speciesDatabase, EntityFactoryConstPtr entityFactory, LiquidsDatabaseConstPtr liquidsDatabase, TechDatabaseConstPtr techDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);
  Player(PlayerConfigPtr config, ByteArray const& netStore, NetCompatibilityRules rules, AssetsConstPtr assets, ConfigurationPtr configuration, MaterialDatabaseConstPtr materialDatabase, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase, CodexDatabaseConstPtr codexDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, RadioMessageDatabaseConstPtr radioMessageDatabase, AiDatabaseConstPtr aiDatabase, CollectionDatabaseConstPtr collectionDatabase, SpeciesDatabaseConstPtr speciesDatabase, EntityFactoryConstPtr entityFactory, LiquidsDatabaseConstPtr liquidsDatabase, TechDatabaseConstPtr techDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);
  Player(PlayerConfigPtr config, Json const& diskStore, AssetsConstPtr assets, ConfigurationPtr configuration, MaterialDatabaseConstPtr materialDatabase, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase, CodexDatabaseConstPtr codexDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, RadioMessageDatabaseConstPtr radioMessageDatabase, AiDatabaseConstPtr aiDatabase, CollectionDatabaseConstPtr collectionDatabase, SpeciesDatabaseConstPtr speciesDatabase, EntityFactoryConstPtr entityFactory, LiquidsDatabaseConstPtr liquidsDatabase, TechDatabaseConstPtr techDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  void diskLoad(Json const& diskStore);

  [[nodiscard]] ClientContextPtr clientContext() const;
  void setClientContext(ClientContextPtr clientContext);

  [[nodiscard]] StatisticsPtr statistics() const;
  void setStatistics(StatisticsPtr statistics);

  void setUniverseClient(UniverseClient* universeClient);
  [[nodiscard]] UniverseClient* universeClient() const;

  [[nodiscard]] QuestManagerPtr questManager() const;
  [[nodiscard]] ItemDatabaseConstPtr itemDatabase() const;

  [[nodiscard]] Json diskStore();
  [[nodiscard]] ByteArray netStore(NetCompatibilityRules rules = {});

  [[nodiscard]] EntityType entityType() const override;
  [[nodiscard]] ClientEntityMode clientEntityMode() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  [[nodiscard]] Vec2F position() const override;
  [[nodiscard]] Vec2F velocity() const override;

  [[nodiscard]] Vec2F mouthPosition() const override;
  [[nodiscard]] Vec2F mouthPosition(bool ignoreAdjustments) const override;
  [[nodiscard]] Vec2F mouthOffset(bool ignoreAdjustments = true) const;
  [[nodiscard]] Vec2F feetOffset() const;
  [[nodiscard]] Vec2F headArmorOffset() const;
  [[nodiscard]] Vec2F chestArmorOffset() const;
  [[nodiscard]] Vec2F legsArmorOffset() const;
  [[nodiscard]] Vec2F backArmorOffset() const;

  // relative to current position
  [[nodiscard]] RectF metaBoundBox() const override;

  // relative to current position
  [[nodiscard]] RectF collisionArea() const override;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationStep = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  [[nodiscard]] Maybe<HitType> queryHit(DamageSource const& source) const override;
  [[nodiscard]] Maybe<PolyF> hitPoly() const override;

  [[nodiscard]] List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  [[nodiscard]] List<DamageNotification> selfDamageNotifications() override;

  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;

  [[nodiscard]] List<DamageSource> damageSources() const override;

  [[nodiscard]] bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  [[nodiscard]] Maybe<EntityAnchorState> loungingIn() const override;
  [[nodiscard]] bool lounge(EntityId loungeableEntityId, size_t anchorIndex);
  void stopLounging();

  void revive(Vec2F const& footPosition);

  [[nodiscard]] List<Drawable> portrait(PortraitMode mode) const override;
  [[nodiscard]] bool underwater() const;

  [[nodiscard]] bool shifting() const;
  void setShifting(bool shifting);
  void special(int specialKey);
  void setBuildToolControlPresses(String const& bindId, Maybe<unsigned> presses);
  [[nodiscard]] Maybe<unsigned> buildToolControlPresses(String const& bindId) const;

  void setMoveVector(Vec2F const& vec);
  void moveLeft();
  void moveRight();
  void moveUp();
  void moveDown();
  void jump();

  void dropItem();

  [[nodiscard]] float toolRadius() const;
  [[nodiscard]] float interactRadius() const override;
  void setInteractRadius(float interactRadius);
  [[nodiscard]] List<InteractAction> pullInteractActions();

  [[nodiscard]] uint64_t currency(String const& currencyType) const;

  [[nodiscard]] float health() const override;
  [[nodiscard]] float maxHealth() const override;
  [[nodiscard]] DamageBarType damageBar() const override;
  [[nodiscard]] float healthPercentage() const;

  [[nodiscard]] float energy() const override;
  [[nodiscard]] float maxEnergy() const;
  [[nodiscard]] float energyPercentage() const;

  [[nodiscard]] float energyRegenBlockPercent() const;

  [[nodiscard]] bool energyLocked() const override;
  [[nodiscard]] bool fullEnergy() const override;
  [[nodiscard]] bool consumeEnergy(float energy) override;

  [[nodiscard]] float foodPercentage() const;

  [[nodiscard]] float breath() const;
  [[nodiscard]] float maxBreath() const;

  [[nodiscard]] float protection() const;

  [[nodiscard]] bool forceNude() const;

  [[nodiscard]] String description() const override;
  void setDescription(String const& description);

  [[nodiscard]] List<LightSource> lightSources() const override;

  [[nodiscard]] Direction walkingDirection() const override;
  [[nodiscard]] Direction facingDirection() const override;

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  [[nodiscard]] Json getGenericProperty(String const& name, Json const& defaultValue = Json()) const;
  void setGenericProperty(String const& name, Json const& value);

  [[nodiscard]] PlayerInventoryPtr inventory() const;
  // Returns the number of items from this stack that could be
  // picked up from the world, using inventory tab filtering
  [[nodiscard]] uint64_t itemsCanHold(ItemPtr const& items) const;
  // Adds items to the inventory, returning the overflow.
  // The items parameter is invalid after use.
  [[nodiscard]] ItemPtr pickupItems(ItemPtr const& items, bool silent = false);
  // Pick up all of the given items as possible, dropping the overflow.
  // The item parameter is invalid after use.
  void giveItem(ItemPtr const& item);

  void triggerPickupEvents(ItemPtr const& item);

  [[nodiscard]] ItemPtr essentialItem(EssentialItem essentialItem) const;
  [[nodiscard]] bool hasItem(ItemDescriptor const& descriptor, bool exactMatch = false) const;
  [[nodiscard]] uint64_t hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch = false) const;
  // altough multiple entries may match, they might have different
  // serializations
  [[nodiscard]] ItemDescriptor takeItem(ItemDescriptor const& descriptor, bool consumePartial = false, bool exactMatch = false);
  void giveItem(ItemDescriptor const& descriptor);

  // Clear the item swap slot.
  void clearSwap();

  void refreshItems();
  void refreshArmor();
  void refreshHumanoid() const;
  // Refresh worn equipment from the inventory
  void refreshEquipment();

  [[nodiscard]] PlayerBlueprintsPtr blueprints() const;
  [[nodiscard]] bool addBlueprint(ItemDescriptor const& descriptor, bool showFailure = false);
  [[nodiscard]] bool blueprintKnown(ItemDescriptor const& descriptor) const;

  [[nodiscard]] bool addCollectable(String const& collectionName, String const& collectableName);

  [[nodiscard]] PlayerUniverseMapPtr universeMap() const;

  [[nodiscard]] PlayerCodexesPtr codexes() const;

  [[nodiscard]] PlayerTechPtr techs() const;
  void overrideTech(Maybe<StringList> const& techModules);
  [[nodiscard]] bool techOverridden() const;

  [[nodiscard]] PlayerCompanionsPtr companions() const;

  [[nodiscard]] PlayerLogPtr log() const;

  [[nodiscard]] InteractiveEntityPtr bestInteractionEntity(bool includeNearby);
  void interactWithEntity(InteractiveEntityPtr entity);

  // Aim this player's target at the given world position.
  void aim(Vec2F const& position);
  [[nodiscard]] Vec2F aimPosition() const override;

  [[nodiscard]] Vec2F armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const override;
  [[nodiscard]] Vec2F handOffset(ToolHand hand, Direction facingDirection) const override;

  [[nodiscard]] Vec2F handPosition(ToolHand hand, Vec2F const& handOffset = {}) const override;
  [[nodiscard]] ItemPtr handItem(ToolHand hand) const override;

  [[nodiscard]] Vec2F armAdjustment() const override;

  void setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) override;

  void playEmote(HumanoidEmote emote) override;

  [[nodiscard]] bool canUseTool() const;

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

  [[nodiscard]] ItemPtr primaryHandItem() const;
  [[nodiscard]] ItemPtr altHandItem() const;

  [[nodiscard]] Uuid uuid() const;

  [[nodiscard]] PlayerMode modeType() const;
  void setModeType(PlayerMode mode);
  [[nodiscard]] PlayerModeConfig modeConfig() const;

  [[nodiscard]] ShipUpgrades shipUpgrades();
  void setShipUpgrades(ShipUpgrades shipUpgrades);
  void applyShipUpgrades(Json const& upgrades);
  void setShipSpecies(String species);
  [[nodiscard]] String shipSpecies() const;

  [[nodiscard]] String name() const override;
  void setName(String const& name);

  [[nodiscard]] Maybe<String> statusText() const override;
  [[nodiscard]] bool displayNametag() const override;
  [[nodiscard]] Vec3B nametagColor() const override;
  [[nodiscard]] Vec2F nametagOrigin() const override;
  [[nodiscard]] String nametag() const override;
  void setNametag(Maybe<String> nametag);

  void updateIdentity();

  void setHumanoidParameter(String key, Maybe<Json> value);
  [[nodiscard]] Maybe<Json> getHumanoidParameter(String key);
  void setHumanoidParameters(JsonObject parameters);
  [[nodiscard]] JsonObject getHumanoidParameters();
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

  [[nodiscard]] String species() const override;
  void setSpecies(String const& species);
  [[nodiscard]] Gender gender() const;
  void setGender(Gender const& gender);
  void setPersonality(Personality const& personality);
  void setImagePath(Maybe<String> const& imagePath);

  [[nodiscard]] HumanoidPtr humanoid();
  [[nodiscard]] HumanoidPtr humanoid() const;
  [[nodiscard]] HumanoidIdentity const& identity() const;

  void setIdentity(HumanoidIdentity identity);

  void setAdmin(bool isAdmin);
  [[nodiscard]] bool isAdmin() const override;

  [[nodiscard]] bool inToolRange() const override;
  [[nodiscard]] bool inToolRange(Vec2F const& aimPos) const override;
  [[nodiscard]] bool inInteractionRange() const;
  [[nodiscard]] bool inInteractionRange(Vec2F aimPos) const;

  void addParticles(List<Particle> const& particles) override;
  void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) override;

  [[nodiscard]] bool wireToolInUse() const;
  void setWireConnector(WireConnector* wireConnector) const;

  void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) override;
  [[nodiscard]] ActiveUniqueStatusEffectSummary activeUniqueStatusEffectSummary() const override;

  [[nodiscard]] float powerMultiplier() const override;

  [[nodiscard]] bool isDead() const;
  void kill();

  void setFavoriteColor(Color color);
  [[nodiscard]] Color favoriteColor() const override;

  // Starts the teleport animation sequence, locking player movement and
  // preventing some update code
  void teleportOut(String const& animationType = "default", bool deploy = false);
  void teleportIn();
  void teleportAbort();

  [[nodiscard]] bool isTeleporting() const;
  [[nodiscard]] bool isTeleportingOut() const;
  [[nodiscard]] bool canDeploy();
  void deployAbort(String const& animationType = "default");
  [[nodiscard]] bool isDeploying() const;
  [[nodiscard]] bool isDeployed() const;

  void setBusyState(PlayerBusyState busyState);

  // A hard move to a specified location
  void moveTo(Vec2F const& footPosition);

  [[nodiscard]] List<String> pullQueuedMessages();
  [[nodiscard]] List<ItemPtr> pullQueuedItemDrops();

  void queueUIMessage(String const& message) override;
  void queueItemPickupMessage(ItemPtr const& item);

  void addChatMessage(String const& message, Json const& config = {});
  void addEmote(HumanoidEmote const& emote, Maybe<float> emoteCooldown = {});
  void setDance(Maybe<String> const& danceName);
  [[nodiscard]] pair<HumanoidEmote, float> currentEmote() const;

  [[nodiscard]] State currentState() const;

  [[nodiscard]] List<ChatAction> pullPendingChatActions() override;

  [[nodiscard]] Maybe<String> inspectionLogName() const override;
  [[nodiscard]] Maybe<String> inspectionDescription(String const& species) const override;

  [[nodiscard]] float beamGunRadius() const override;

  [[nodiscard]] bool instrumentPlaying() override;
  void instrumentEquipped(String const& instrumentKind) override;
  void interact(InteractAction const& action) override;
  void addEffectEmitters(StringSet const& emitters) override;
  void requestEmote(String const& emote) override;

  [[nodiscard]] ActorMovementController* movementController() override;
  [[nodiscard]] StatusController* statusController() override;

  [[nodiscard]] List<PhysicsForceRegion> forceRegions() const override;

  [[nodiscard]] StatusControllerPtr statusControllerPtr();
  [[nodiscard]] ActorMovementControllerPtr movementControllerPtr();

  [[nodiscard]] PlayerConfigPtr config();

  [[nodiscard]] SongbookPtr songbook() const;

  void finalizeCreation();

  [[nodiscard]] float timeSinceLastGaveDamage() const;
  [[nodiscard]] EntityId lastDamagedTarget() const;

  [[nodiscard]] bool invisible() const;

  void animatePortrait(float dt);

  [[nodiscard]] bool isOutside();

  void dropSelectedItems(function<bool(ItemPtr)> filter);
  void dropEverything();

  [[nodiscard]] bool isPermaDead() const;

  [[nodiscard]] bool interruptRadioMessage();
  [[nodiscard]] Maybe<RadioMessage> pullPendingRadioMessage();
  void queueRadioMessage(Json const& messageConfig, float delay = 0);
  void queueRadioMessage(RadioMessage message);

  // If a cinematic should play, returns it and clears it.  May stop cinematics
  // by returning a null Json.
  [[nodiscard]] Maybe<Json> pullPendingCinematic();
  void setPendingCinematic(Json const& cinematic, bool unique = false);

  void setInCinematic(bool inCinematic);

  [[nodiscard]] Maybe<pair<Maybe<pair<StringList, int>>, float>> pullPendingAltMusic();

  [[nodiscard]] Maybe<PlayerWarpRequest> pullPendingWarp();
  void setPendingWarp(String const& action, Maybe<String> const& animation = {}, bool deploy = false);

  [[nodiscard]] Maybe<pair<Json, RpcPromiseKeeper<Json>>> pullPendingConfirmation();
  void queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise);

  [[nodiscard]] AiState const& aiState() const;
  [[nodiscard]] AiState& aiState();

  // In inspection mode, scannable, scanned, and interesting objects will be
  // rendered with special highlighting.
  [[nodiscard]] bool inspecting() const;

  // Will return the highlight effect to give an inspectable entity when inspecting
  [[nodiscard]] EntityHighlightEffect inspectionHighlight(InspectableEntityPtr const& inspectableEntity) const;

  [[nodiscard]] Vec2F cameraPosition();

  using Entity::setTeam;

  [[nodiscard]] NetworkedAnimatorPtr effectsAnimator();

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
  [[nodiscard]] Maybe<StringView> getSecretPropertyView(String const& name) const;
  [[nodiscard]] String const* getSecretPropertyPtr(String const& name) const;
  // Gets a secret Json property. It will be de-serialized.
  [[nodiscard]] Json getSecretProperty(String const& name, Json defaultValue = Json()) const;
  // Sets a secret Json property. It will be serialized.
  void setSecretProperty(String const& name, Json const& value);

  void setAnimationParameter(String name, Json value);

private:
  using GenericScriptComponent = LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>>;
  using GenericScriptComponentPtr = shared_ptr<GenericScriptComponent>;

  using ItemSetFunc = std::function<void(ItemPtr)>;

  // handle input and other events (master only) that happen BEFORE movement/tech controller updates
  void processControls();

  // state changes and effect animations (master and slave) that happen AFTER movement/tech controller updates
  void processStateChanges(float dt);

  void getNetStates(bool initial);
  void setNetStates();
  void getNetArmorSecrets();
  void setNetArmorSecret(EquipmentSlot slot, ArmorItemPtr const& armor, bool visible = true);
  void setNetArmorSecrets(bool includeEmpty = false);

  [[nodiscard]] List<Drawable> drawables() const;
  [[nodiscard]] List<OverheadBar> bars() const;
  [[nodiscard]] List<Particle> particles();
  [[nodiscard]] String getFootstepSound(Vec2I const& sensor) const;

  void tickShared(float dt);

  [[nodiscard]] HumanoidEmote detectEmotes(String const& chatter);

  friend class PlayerChatAndEmotes;
  friend class PlayerDamagePipeline;
  friend class PlayerTeleporter;
  friend class PlayerAppearance;

  PlayerConfigPtr m_config;

  NetElementTopGroup m_netGroup;

  ClientContextPtr m_clientContext;
  StatisticsPtr m_statistics;
  QuestManagerPtr m_questManager;

  PlayerInventoryPtr m_inventory;
  PlayerBlueprintsPtr m_blueprints;
  PlayerUniverseMapPtr m_universeMap;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  MaterialDatabaseConstPtr m_materialDatabase;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  QuestTemplateDatabaseConstPtr m_questTemplateDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;
  CodexDatabaseConstPtr m_codexDatabase;
  DanceDatabaseConstPtr m_danceDatabase;
  EmoteProcessorConstPtr m_emoteProcessor;
  RadioMessageDatabaseConstPtr m_radioMessageDatabase;
  AiDatabaseConstPtr m_aiDatabase;
  CollectionDatabaseConstPtr m_collectionDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  EntityFactoryConstPtr m_entityFactory;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  TechDatabaseConstPtr m_techDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  PlayerCodexesPtr m_codexes;
  PlayerTechPtr m_techs;
  PlayerCompanionsPtr m_companions;
  PlayerDeploymentPtr m_deployment;
  PlayerLogPtr m_log;

  UniverseClient* m_client;// required for celestial callbacks in scripts
  StringMap<GenericScriptComponentPtr> m_genericScriptContexts;
  JsonObject m_genericProperties;

  PlayerChatAndEmotesPtr m_chatAndEmotes;

  State m_state;

  float m_footstepTimer;
  PlayerTeleporterPtr m_teleporter;
  PlayerAppearance m_appearance{*this};
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  GameTimer m_ageItemsTimer;

  float m_footstepVolumeVariance;
  float m_landingVolume;
  bool m_landingNoisePending;
  bool m_footstepPending;

  NetworkedAnimatorPtr m_effectsAnimator;
  NetworkedAnimator::DynamicTarget m_effectsAnimatorDynamicTarget;

  HashSet<MoveControlType> m_pendingMoves;
  StringMap<unsigned> m_buildToolControlPresses;
  Vec2F m_moveVector;
  bool m_shifting;
  ActorMovementParameters m_zeroGMovementParameters;

  PlayerDamagePipelinePtr m_damagePipeline;

  String m_description;

  PlayerMode m_modeType;
  PlayerModeConfig m_modeConfig;
  ShipUpgrades m_shipUpgrades;
  String m_shipSpecies;

  ToolUserPtr m_tools;
  ArmorWearerPtr m_armor;
  HashMap<EquipmentSlot, uint64_t> m_armorSecretNetVersions;

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

  bool m_isAdmin;
  float m_interactRadius;      // hand interact radius
  Vec2F m_walkIntoInteractBias;// offset on position to find an interactable
  // when not pointing at
  // an interactable with the mouse

  List<RpcPromise<InteractAction>> m_pendingInteractActions;

  List<Particle> m_callbackParticles;
  List<tuple<String, float, float>> m_callbackSounds;

  List<String> m_queuedMessages;
  List<ItemPtr> m_queuedItemPickups;

  PlayerNarrativeQueuePtr m_narrativeQueue;

  AiState m_aiState;

  EffectEmitterPtr m_effectEmitter;

  SongbookPtr m_songbook;

  StringSet m_interestingObjects;

  NetElementUInt m_stateNetState;
  NetElementBool m_shiftingNetState;
  NetElementFloat m_xAimPositionNetState;
  NetElementFloat m_yAimPositionNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementEvent m_landedNetState;
  NetElementString m_chatMessageNetState;
  NetElementEvent m_newChatMessageNetState;
  NetElementString m_emoteNetState;
};

}// namespace Star
