#pragma once

#include "StarActorEntity.hpp"
#include "StarActorMovementController.hpp"
#include "StarAggressiveEntity.hpp"
#include "StarBehaviorState.hpp"
#include "StarChattyEntity.hpp"
#include "StarConfig.hpp"
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
#include "StarScriptedEntity.hpp"

import std;

namespace Star {

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

  Monster(MonsterVariant const& variant, std::optional<float> level = {});
  Monster(Json const& diskStore);

  auto diskStore() const -> Json;
  auto netStore(NetCompatibilityRules rules = {}) -> ByteArray;

  auto entityType() const -> EntityType override;
  auto clientEntityMode() const -> ClientEntityMode override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto position() const -> Vec2F override;
  auto metaBoundBox() const -> RectF override;

  auto velocity() const -> Vec2F;

  auto mouthOffset() const -> Vec2F;
  auto feetOffset() const -> Vec2F;

  auto collisionArea() const -> RectF override;

  auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  auto name() const -> String override;
  auto description() const -> String override;

  auto lightSources() const -> List<LightSource> override;

  auto queryHit(DamageSource const& source) const -> std::optional<HitType> override;
  auto hitPoly() const -> std::optional<PolyF> override;

  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;

  auto applyDamage(DamageRequest const& damage) -> List<DamageNotification> override;
  auto selfDamageNotifications() -> List<DamageNotification> override;

  auto damageSources() const -> List<DamageSource> override;

  auto shouldDie() -> bool;
  void knockout();

  auto shouldDestroy() const -> bool override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, std::uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  void setPosition(Vec2F const& pos);

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json> override;

  auto maxHealth() const -> float override;
  auto health() const -> float override;
  auto damageBar() const -> DamageBarType override;

  auto monsterLevel() const -> float;
  auto activeSkillInfo() const -> SkillInfo;

  auto portrait(PortraitMode mode) const -> List<Drawable> override;
  auto typeName() const -> String;
  auto monsterVariant() const -> MonsterVariant;

  auto statusText() const -> std::optional<String> override;
  auto displayNametag() const -> bool override;
  auto nametagColor() const -> Vec3B override;
  auto nametagOrigin() const -> Vec2F override;
  auto nametag() const -> String override;

  auto aggressive() const -> bool override;

  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> override;
  auto evalScript(String const& code) -> std::optional<LuaValue> override;

  auto mouthPosition() const -> Vec2F override;
  auto mouthPosition(bool ignoreAdjustments) const -> Vec2F override;
  auto pullPendingChatActions() -> List<ChatAction> override;

  auto forceRegions() const -> List<PhysicsForceRegion> override;

  auto interact(InteractRequest const& request) -> InteractAction override;
  auto isInteractive() const -> bool override;

  auto questIndicatorPosition() const -> Vec2F override;

  auto movementController() -> ActorMovementController* override;
  auto statusController() -> StatusController* override;

  using Entity::setKeepAlive;
  using Entity::setUniqueId;

private:
  auto getAbsolutePosition(Vec2F relativePosition) const -> Vec2F;

  void updateStatus(float dt);
  auto makeMonsterCallbacks() -> LuaCallbacks;

  void addChatMessage(String const& message, String const& portrait = "");

  void setupNetStates();
  void getNetStates(bool initial);
  void setNetStates();

  NetElementTopGroup m_netGroup;

  NetElementData<std::optional<String>> m_uniqueIdNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  MonsterVariant m_monsterVariant;
  std::optional<float> m_monsterLevel;

  NetworkedAnimator m_networkedAnimator;
  NetworkedAnimator::DynamicTarget m_networkedAnimatorDynamicTarget;

  Ptr<ActorMovementController> m_movementController;
  Ptr<StatusController> m_statusController;

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

  List<Ptr<BehaviorState>> m_behaviors;
  mutable LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_scriptComponent;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;

  NetElementData<List<PhysicsForceRegion>> m_physicsForces;

  NetElementData<std::optional<float>> m_monsterLevelNetState;
  NetElementBool m_damageOnTouchNetState;
  NetElementData<StringSet> m_animationDamageParts;
  NetElementData<List<DamageSource>> m_damageSources;
  NetElementData<Json> m_dropPoolNetState;
  NetElementBool m_aggressiveNetState;
  NetElementBool m_knockedOutNetState;
  NetElementString m_deathParticleBurstNetState;
  NetElementString m_deathSoundNetState;
  NetElementString m_activeSkillNameNetState;
  NetElementData<std::optional<String>> m_name;
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
