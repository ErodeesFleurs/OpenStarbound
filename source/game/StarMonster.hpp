#pragma once

#include "StarActorEntity.hpp"
#include "StarActorMovementController.hpp"
#include "StarAggressiveEntity.hpp"
#include "StarAssets.hpp"
#include "StarBehaviorState.hpp"
#include "StarChattyEntity.hpp"
#include "StarDamageBarEntity.hpp"
#include "StarEffectEmitter.hpp"
#include "StarEntity.hpp"
#include "StarEntityRendering.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarMonsterDatabase.hpp"
#include "StarNametagEntity.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarPortraitEntity.hpp"
#include "StarScriptedEntity.hpp"

namespace Star {

class Monster;
using MonsterPtr = SharedPtr<Monster>;
class LiquidsDatabase;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;
class StatusController;
using StatusControllerPtr = SharedPtr<StatusController>;

class Monster
    : public virtual DamageBarEntity,
      public virtual AggressiveEntity,
      public virtual ScriptedEntity,
      public virtual PhysicsEntity,
      public virtual NametagEntity,
      public virtual ChattyEntity,
      public virtual InteractiveEntity,
      public virtual ActorEntity {
public:
  struct SkillInfo {
    String label;
    String image;
  };

  Monster(AssetsConstPtr assets, MonsterDatabaseConstPtr monsterDatabase, MonsterVariant const& variant, LiquidsDatabaseConstPtr liquidsDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Maybe<float> level = {});
  Monster(AssetsConstPtr assets, MonsterDatabaseConstPtr monsterDatabase, Json const& diskStore, LiquidsDatabaseConstPtr liquidsDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] Json diskStore() const;
  [[nodiscard]] ByteArray netStore(NetCompatibilityRules rules = {});

  [[nodiscard]] EntityType entityType() const override;
  [[nodiscard]] ClientEntityMode clientEntityMode() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  [[nodiscard]] Vec2F position() const override;
  [[nodiscard]] RectF metaBoundBox() const override;

  [[nodiscard]] Vec2F velocity() const;

  [[nodiscard]] Vec2F mouthOffset() const;
  [[nodiscard]] Vec2F feetOffset() const;

  [[nodiscard]] RectF collisionArea() const override;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  [[nodiscard]] String name() const override;
  [[nodiscard]] String description() const override;

  [[nodiscard]] List<LightSource> lightSources() const override;

  [[nodiscard]] Maybe<HitType> queryHit(DamageSource const& source) const override;
  [[nodiscard]] Maybe<PolyF> hitPoly() const override;

  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;

  [[nodiscard]] List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  [[nodiscard]] List<DamageNotification> selfDamageNotifications() override;

  [[nodiscard]] List<DamageSource> damageSources() const override;

  [[nodiscard]] bool shouldDie();
  void knockout();

  [[nodiscard]] bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  void setPosition(Vec2F const& pos);

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  [[nodiscard]] float maxHealth() const override;
  [[nodiscard]] float health() const override;
  [[nodiscard]] DamageBarType damageBar() const override;

  [[nodiscard]] float monsterLevel() const;
  [[nodiscard]] SkillInfo activeSkillInfo() const;

  [[nodiscard]] List<Drawable> portrait(PortraitMode mode) const override;
  [[nodiscard]] String typeName() const;
  [[nodiscard]] MonsterVariant monsterVariant() const;

  [[nodiscard]] Maybe<String> statusText() const override;
  [[nodiscard]] bool displayNametag() const override;
  [[nodiscard]] Vec3B nametagColor() const override;
  [[nodiscard]] Vec2F nametagOrigin() const override;
  [[nodiscard]] String nametag() const override;

  [[nodiscard]] bool aggressive() const override;

  [[nodiscard]] Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  [[nodiscard]] Maybe<LuaValue> evalScript(String const& code) override;

  [[nodiscard]] Vec2F mouthPosition() const override;
  [[nodiscard]] Vec2F mouthPosition(bool ignoreAdjustments) const override;
  [[nodiscard]] List<ChatAction> pullPendingChatActions() override;

  [[nodiscard]] List<PhysicsForceRegion> forceRegions() const override;

  [[nodiscard]] InteractAction interact(InteractRequest const& request) override;
  [[nodiscard]] bool isInteractive() const override;

  [[nodiscard]] Vec2F questIndicatorPosition() const override;

  [[nodiscard]] ActorMovementController* movementController() override;
  [[nodiscard]] StatusController* statusController() override;

  using Entity::setKeepAlive;
  using Entity::setUniqueId;

private:
  [[nodiscard]] Vec2F getAbsolutePosition(Vec2F relativePosition) const;

  void updateStatus(float dt);
  [[nodiscard]] LuaCallbacks makeMonsterCallbacks();

  void addChatMessage(String const& message, String const& portrait = "");

  void setupNetStates();
  void getNetStates(bool initial);
  void setNetStates();

  NetElementTopGroup m_netGroup;

  NetElementData<Maybe<String>> m_uniqueIdNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  MonsterVariant m_monsterVariant;
  MonsterDatabaseConstPtr m_monsterDatabase;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  Maybe<float> m_monsterLevel;

  NetworkedAnimator m_networkedAnimator;
  NetworkedAnimator::DynamicTarget m_networkedAnimatorDynamicTarget;

  ActorMovementControllerPtr m_movementController;
  StatusControllerPtr m_statusController;

  EffectEmitter m_effectEmitter;

  // The set of damage source kinds that were used to kill this entity.
  StringSet m_deathDamageSourceKinds;

  bool m_damageOnTouch;
  bool m_aggressive;

  bool m_knockedOut;
  double m_knockoutTimer;
  String m_deathParticleBurst;
  String m_deathSound;

  String m_activeSkillName;
  Json m_dropPool;

  Vec2F m_questIndicatorOffset;

  List<BehaviorStatePtr> m_behaviors;
  mutable LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_scriptComponent;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;

  NetElementData<List<PhysicsForceRegion>> m_physicsForces;

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
  NetElementData<Maybe<String>> m_name;
  NetElementBool m_displayNametag;
  NetElementBool m_interactive;

  List<ChatAction> m_pendingChatActions;
  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;

  NetElementData<DamageBarType> m_damageBar;

  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
};

}// namespace Star
