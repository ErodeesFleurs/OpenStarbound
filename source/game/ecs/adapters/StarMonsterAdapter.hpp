#pragma once

#include "StarEntityAdapter.hpp"
#include "StarMonsterDatabase.hpp"
#include "StarActorMovementController.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarStatusController.hpp"
#include "StarEffectEmitter.hpp"
#include "StarBehaviorState.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarDamageBarEntity.hpp"
#include "StarNametagEntity.hpp"
#include "StarPortraitEntity.hpp"
#include "StarAggressiveEntity.hpp"
#include "StarScriptedEntity.hpp"
#include "StarChattyEntity.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarActorEntity.hpp"

namespace Star {
namespace ECS {

// Tag component for monster entities
struct MonsterTag {};

// Monster-specific data component
struct MonsterDataComponent {
  MonsterVariant variant;
  Maybe<float> level;
  
  // Movement and physics
  ActorMovementControllerPtr movementController;
  
  // Status
  StatusControllerPtr statusController;
  
  // Animation
  NetworkedAnimator networkedAnimator;
  NetworkedAnimator::DynamicTarget animatorDynamicTarget;
  
  // Effects
  EffectEmitter effectEmitter;
  
  // Scripting
  mutable LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> scriptComponent;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> scriptedAnimator;
  
  // Behaviors
  List<BehaviorStatePtr> behaviors;
  
  // Combat state
  bool damageOnTouch = false;
  bool aggressive = false;
  List<DamageSource> damageSources;
  StringSet animationDamageParts;
  StringSet deathDamageSourceKinds;
  
  // Death state
  bool knockedOut = false;
  double knockoutTimer = 0.0;
  String deathParticleBurst;
  String deathSound;
  
  // Drop state
  Json dropPool;
  
  // Active skill
  String activeSkillName;
  
  // Display
  Maybe<String> name;
  bool displayNametag = false;
  DamageBarType damageBar = DamageBarType::Default;
  bool interactive = false;
  
  // Physics forces
  List<PhysicsForceRegion> physicsForces;
  
  // Chat
  List<ChatAction> pendingChatActions;
  String chatMessage;
  String chatPortrait;
  
  // Quest
  Vec2F questIndicatorOffset;
  
  // Scripted animation parameters
  StringMap<Json> scriptedAnimationParameters;
};

// Skill info structure (same as Monster::SkillInfo)
struct MonsterSkillInfo {
  String label;
  String image;
};

class MonsterAdapter 
  : public EntityAdapter,
    public virtual DamageBarEntity,
    public virtual AggressiveEntity,
    public virtual ScriptedEntity,
    public virtual PhysicsEntity,
    public virtual NametagEntity,
    public virtual ChattyEntity,
    public virtual InteractiveEntity,
    public virtual ActorEntity {
public:
  // Factory method to create from variant
  static shared_ptr<MonsterAdapter> create(World& ecsWorld, MonsterVariant const& variant, Maybe<float> level = {});
  
  // Factory method to create from disk store
  static shared_ptr<MonsterAdapter> createFromDiskStore(World& ecsWorld, Json const& diskStore);

  MonsterAdapter(World& ecsWorld, Entity entity);
  virtual ~MonsterAdapter() = default;

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

  Vec2F velocity() const;

  Vec2F mouthOffset() const;
  Vec2F feetOffset() const;

  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  String name() const override;
  String description() const override;

  List<LightSource> lightSources() const override;

  // DamageBarEntity interface
  float maxHealth() const override;
  float health() const override;
  DamageBarType damageBar() const override;

  // Combat
  Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;

  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;

  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  List<DamageNotification> selfDamageNotifications() override;

  List<DamageSource> damageSources() const override;

  bool shouldDie();
  void knockout();

  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;

  void setPosition(Vec2F const& pos);

  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  // Monster-specific methods
  float monsterLevel() const;
  MonsterSkillInfo activeSkillInfo() const;
  String typeName() const;
  MonsterVariant monsterVariant() const;

  // PortraitEntity interface
  List<Drawable> portrait(PortraitMode mode) const override;

  // NametagEntity interface
  Maybe<String> statusText() const override;
  bool displayNametag() const override;
  Vec3B nametagColor() const override;
  Vec2F nametagOrigin() const override;
  String nametag() const override;

  // AggressiveEntity interface
  bool aggressive() const override;

  // ScriptedEntity interface
  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;

  // ChattyEntity interface
  virtual Vec2F mouthPosition() const override;
  virtual Vec2F mouthPosition(bool ignoreAdjustments) const override;
  virtual List<ChatAction> pullPendingChatActions() override;

  // PhysicsEntity interface
  List<PhysicsForceRegion> forceRegions() const override;

  // InteractiveEntity interface
  InteractAction interact(InteractRequest const& request) override;
  bool isInteractive() const override;

  Vec2F questIndicatorPosition() const override;

  // ActorEntity interface
  ActorMovementController* movementController() override;
  StatusController* statusController() override;

  using EntityAdapter::setKeepAlive;
  using EntityAdapter::setUniqueId;

private:
  MonsterDataComponent* getData();
  MonsterDataComponent const* getData() const;

  Vec2F getAbsolutePosition(Vec2F relativePosition) const;
  void updateStatus(float dt);
  LuaCallbacks makeMonsterCallbacks();
  void addChatMessage(String const& message, String const& portrait = "");

  void setupNetStates();
  void getNetStates(bool initial);
  void setNetStates();

  // Network state elements
  NetElementTopGroup m_netGroup;
  NetElementData<Maybe<String>> m_uniqueIdNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementData<Maybe<float>> m_monsterLevelNetState;
  NetElementBool m_damageOnTouchNetState;
  NetElementData<StringSet> m_animationDamageParts;
  NetElementData<List<DamageSource>> m_damageSources;
  NetElementData<Json> m_dropPoolNetState;
  NetElementBool m_aggressiveNetState;
  NetElementBool m_knockedOutNetState;
  NetElementString m_deathParticleBurstNetState;
  NetElementString m_deathSoundNetState;
  NetElementString m_activeSkillNameNetState;
  NetElementData<Maybe<String>> m_nameNetState;
  NetElementBool m_displayNametagNetState;
  NetElementBool m_interactiveNetState;
  NetElementData<List<PhysicsForceRegion>> m_physicsForces;
  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;
  NetElementData<DamageBarType> m_damageBarNetState;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
};

} // namespace ECS
} // namespace Star
