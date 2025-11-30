#pragma once

#include "StarEntityAdapter.hpp"
#include "StarNpcDatabase.hpp"
#include "StarActorMovementController.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarStatusController.hpp"
#include "StarEffectEmitter.hpp"
#include "StarBehaviorState.hpp"
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
#include "StarScriptedEntity.hpp"
#include "StarChattyEntity.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarEmoteEntity.hpp"
#include "StarInteractiveEntity.hpp"
#include "StarLoungingEntities.hpp"
#include "StarToolUserEntity.hpp"

namespace Star {
namespace ECS {

// Tag component for NPC entities
struct NpcTag {};

// NPC-specific data component
struct NpcDataComponent {
  NpcVariant npcVariant;
  
  // Humanoid appearance
  HumanoidPtr humanoid;
  bool identityUpdated = false;
  Maybe<String> deathParticleBurst;
  
  // Movement and physics
  ActorMovementControllerPtr movementController;
  
  // Status
  StatusControllerPtr statusController;
  
  // Effects
  EffectEmitterPtr effectEmitter;
  
  // Equipment
  ArmorWearerPtr armor;
  ToolUserPtr tools;
  SongbookPtr songbook;
  
  // Scripting
  mutable LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> scriptComponent;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> scriptedAnimator;
  
  // Behaviors
  List<BehaviorStatePtr> behaviors;
  
  // Combat state
  bool aggressive = false;
  bool damageOnTouch = false;
  
  // Emote state
  HumanoidEmote emoteState = HumanoidEmote::Idle;
  GameTimer emoteCooldownTimer;
  Maybe<String> dance;
  GameTimer danceCooldownTimer;
  GameTimer blinkCooldownTimer;
  Vec2F blinkInterval;
  
  // Display and interaction
  bool isInteractive = false;
  Maybe<String> statusText;
  bool displayNametag = false;
  bool disableWornArmor = false;
  
  // Quests
  List<QuestArcDescriptor> offeredQuests;
  StringSet turnInQuests;
  Vec2F questIndicatorOffset;
  
  // Chat
  List<ChatAction> pendingChatActions;
  String chatMessage;
  String chatPortrait;
  Json chatConfig;
  bool chatMessageUpdated = false;
  
  // Aim position
  float xAimPosition = 0.0f;
  float yAimPosition = 0.0f;
  
  // Drop pools
  StringList dropPools;
  
  // Shifting state (crouching)
  bool shifting = false;
  
  // Damage notification limiter
  int hitDamageNotificationLimiter = 0;
  int hitDamageNotificationLimit = 10;
  
  // Scripted animation parameters
  StringMap<Json> scriptedAnimationParameters;
  
  // Client entity mode
  ClientEntityMode clientEntityMode = ClientEntityMode::ClientSlaveOnly;
};

class NpcAdapter 
  : public EntityAdapter,
    public virtual DamageBarEntity,
    public virtual PortraitEntity,
    public virtual NametagEntity,
    public virtual ScriptedEntity,
    public virtual ChattyEntity,
    public virtual InteractiveEntity,
    public virtual LoungingEntity,
    public virtual ToolUserEntity,
    public virtual PhysicsEntity,
    public virtual EmoteEntity {
public:
  // Factory method to create from variant
  static shared_ptr<NpcAdapter> create(World& ecsWorld, NpcVariant const& variant);
  
  // Factory method to create from disk store
  static shared_ptr<NpcAdapter> createFromDiskStore(World& ecsWorld, Json const& diskStore);
  
  // Factory method to create from network
  static shared_ptr<NpcAdapter> createFromNetStore(World& ecsWorld, ByteArray const& netStore, NetCompatibilityRules rules = {});

  NpcAdapter(World& ecsWorld, Entity entity);
  virtual ~NpcAdapter() = default;

  // Disk and network serialization
  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {});

  // Entity interface
  EntityType entityType() const override;
  ClientEntityMode clientEntityMode() const override;

  void init(::Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  Vec2F position() const override;
  RectF metaBoundBox() const override;
  RectF collisionArea() const override;

  Vec2F velocity() const override;

  Vec2F mouthOffset(bool ignoreAdjustments = true) const;
  Vec2F feetOffset() const;
  Vec2F headArmorOffset() const;
  Vec2F chestArmorOffset() const;
  Vec2F legsArmorOffset() const;
  Vec2F backArmorOffset() const;

  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  String description() const override;
  String species() const override;
  Gender gender() const;
  String npcType() const;

  Json scriptConfigParameter(String const& parameterName, Json const& defaultValue = Json()) const;

  // DamageBarEntity interface
  float maxHealth() const override;
  float health() const override;
  DamageBarType damageBar() const override;

  // Combat
  Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;

  void damagedOther(DamageNotification const& damage) override;

  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  List<DamageNotification> selfDamageNotifications() override;

  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;

  void setPosition(Vec2F const& pos);

  // PortraitEntity interface
  List<Drawable> portrait(PortraitMode mode) const override;
  String name() const override;

  // NametagEntity interface
  Maybe<String> statusText() const override;
  bool displayNametag() const override;
  Vec3B nametagColor() const override;
  Vec2F nametagOrigin() const override;
  String nametag() const override;

  bool aggressive() const;

  // ScriptedEntity interface
  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;

  // ChattyEntity interface
  Vec2F mouthPosition() const override;
  Vec2F mouthPosition(bool ignoreAdjustments) const override;
  List<ChatAction> pullPendingChatActions() override;

  // InteractiveEntity interface
  bool isInteractive() const override;
  InteractAction interact(InteractRequest const& request) override;
  RectF interactiveBoundBox() const override;

  // LoungingEntity interface
  Maybe<EntityAnchorState> loungingIn() const override;

  List<QuestArcDescriptor> offeredQuests() const override;
  StringSet turnInQuests() const override;
  Vec2F questIndicatorPosition() const override;

  List<LightSource> lightSources() const override;

  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  // ToolUserEntity interface
  Vec2F armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const override;
  Vec2F handOffset(ToolHand hand, Direction facingDirection) const override;
  Vec2F handPosition(ToolHand hand, Vec2F const& handOffset = {}) const override;
  ItemPtr handItem(ToolHand hand) const override;
  Vec2F armAdjustment() const override;
  Vec2F aimPosition() const override;
  float interactRadius() const override;
  Direction facingDirection() const override;
  Direction walkingDirection() const override;
  bool isAdmin() const override;
  Color favoriteColor() const override;
  float beamGunRadius() const override;
  void addParticles(List<Particle> const& particles) override;
  void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) override;
  bool inToolRange() const override;
  bool inToolRange(Vec2F const& position) const override;
  void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) override;
  ActiveUniqueStatusEffectSummary activeUniqueStatusEffectSummary() const override;
  float powerMultiplier() const override;
  bool fullEnergy() const override;
  float energy() const override;
  bool energyLocked() const override;
  bool consumeEnergy(float energy) override;
  void queueUIMessage(String const& message) override;
  bool instrumentPlaying() override;
  void instrumentEquipped(String const& instrumentKind) override;
  void interact(InteractAction const& action) override;
  void addEffectEmitters(StringSet const& emitters) override;
  void requestEmote(String const& emote) override;
  ActorMovementController* movementController() override;
  StatusController* statusController() override;
  void setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) override;

  // EmoteEntity interface
  void playEmote(HumanoidEmote emote) override;

  // PhysicsEntity interface
  List<DamageSource> damageSources() const override;
  List<PhysicsForceRegion> forceRegions() const override;

  using EntityAdapter::setUniqueId;

  // Identity management
  HumanoidIdentity const& identity() const;
  void updateIdentity();
  void setIdentity(HumanoidIdentity identity);

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
  void setSpecies(String const& species);
  void setGender(Gender const& gender);
  void setPersonality(Personality const& personality);
  void setImagePath(Maybe<String> const& imagePath);
  void setFavoriteColor(Color color);
  void setName(String const& name);
  void setDescription(String const& description);

  HumanoidPtr humanoid();
  HumanoidPtr humanoid() const;

  bool forceNude() const;

  Songbook* songbook();

private:
  NpcDataComponent* getData();
  NpcDataComponent const* getData() const;

  Vec2F getAbsolutePosition(Vec2F relativePosition) const;
  void tickShared(float dt);
  LuaCallbacks makeNpcCallbacks();

  void setupNetStates();
  void getNetStates(bool initial);
  void setNetStates();

  void addChatMessage(String const& message, Json const& config, String const& portrait = "");
  void addEmote(HumanoidEmote const& emote);
  void setDance(Maybe<String> const& danceName);

  bool setItemSlot(String const& slot, ItemDescriptor itemDescriptor);
  bool canUseTool() const;
  void disableWornArmor(bool disable);

  // Network state elements
  NetElementTopGroup m_netGroup;
  NetElementFloat m_xAimPosition;
  NetElementFloat m_yAimPosition;

  NetElementData<Maybe<String>> m_uniqueIdNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementEnum<Humanoid::State> m_humanoidStateNetState;
  NetElementEnum<HumanoidEmote> m_humanoidEmoteStateNetState;
  NetElementData<Maybe<String>> m_humanoidDanceNetState;

  NetElementData<HumanoidIdentity> m_identityNetState;

  NetElementData<Maybe<String>> m_deathParticleBurst;

  NetElementBool m_aggressive;

  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;
  NetElementData<Json> m_chatConfig;

  NetElementData<Maybe<String>> m_statusText;
  NetElementBool m_displayNametag;

  NetElementBool m_isInteractive;

  NetElementData<List<QuestArcDescriptor>> m_offeredQuests;
  NetElementData<StringSet> m_turnInQuests;

  NetElementBool m_shifting;
  NetElementBool m_damageOnTouch;

  NetElementBool m_disableWornArmor;

  NetElementData<StringList> m_dropPools;

  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
};

} // namespace ECS
} // namespace Star
