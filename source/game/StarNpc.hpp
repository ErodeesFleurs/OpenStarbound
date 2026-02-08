#pragma once

#include "StarActorMovementController.hpp"
#include "StarArmorWearer.hpp"
#include "StarBehaviorState.hpp"
#include "StarChattyEntity.hpp"
#include "StarConfig.hpp"
#include "StarDamageBarEntity.hpp"
#include "StarEffectEmitter.hpp"
#include "StarEmoteEntity.hpp"
#include "StarEntity.hpp"
#include "StarEntitySplash.hpp"
#include "StarHumanoid.hpp"
#include "StarInteractiveEntity.hpp"
#include "StarLoungingEntities.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNametagEntity.hpp"
#include "StarNetElementDynamicGroup.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNpcDatabase.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarPortraitEntity.hpp"
#include "StarScriptedEntity.hpp"
#include "StarToolUser.hpp"
#include "StarToolUserEntity.hpp"

import std;

namespace Star {

class Songbook;
// STAR_CLASS(Songbook);
// STAR_CLASS(Item);
// STAR_CLASS(RenderCallback);
// STAR_CLASS(Npc);
// STAR_CLASS(StatusController);

class Npc
    : public virtual DamageBarEntity,
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
  Npc(ByteArray const& netStore, NetCompatibilityRules rules = {});
  Npc(NpcVariant const& npcVariant);
  Npc(NpcVariant const& npcVariant, Json const& initialState);

  auto diskStore() const -> Json;
  auto netStore(NetCompatibilityRules rules = {}) -> ByteArray;

  auto entityType() const -> EntityType override;
  auto clientEntityMode() const -> ClientEntityMode override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto position() const -> Vec2F override;
  auto metaBoundBox() const -> RectF override;

  auto mouthOffset(bool ignoreAdjustments = true) const -> Vec2F;
  auto feetOffset() const -> Vec2F;
  auto headArmorOffset() const -> Vec2F;
  auto chestArmorOffset() const -> Vec2F;
  auto legsArmorOffset() const -> Vec2F;
  auto backArmorOffset() const -> Vec2F;

  auto collisionArea() const -> RectF override;

  auto writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  auto description() const -> String override;
  auto species() const -> String override;
  auto gender() const -> Gender;
  auto npcType() const -> String;

  auto scriptConfigParameter(String const& parameterName, Json const& defaultValue = Json()) const -> Json;

  auto queryHit(DamageSource const& source) const -> std::optional<HitType> override;
  auto hitPoly() const -> std::optional<PolyF> override;

  void damagedOther(DamageNotification const& damage) override;

  auto applyDamage(DamageRequest const& damage) -> List<DamageNotification> override;
  auto selfDamageNotifications() -> List<DamageNotification> override;

  auto shouldDestroy() const -> bool override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, uint64_t currentVersion) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  void setPosition(Vec2F const& pos);

  auto maxHealth() const -> float override;
  auto health() const -> float override;
  auto damageBar() const -> DamageBarType override;

  auto portrait(PortraitMode mode) const -> List<Drawable> override;
  auto name() const -> String override;
  auto statusText() const -> std::optional<String> override;
  auto displayNametag() const -> bool override;
  auto nametagColor() const -> Vec3B override;
  auto nametagOrigin() const -> Vec2F override;
  auto nametag() const -> String override;

  auto aggressive() const -> bool;

  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> override;
  auto evalScript(String const& code) -> std::optional<LuaValue> override;

  auto mouthPosition() const -> Vec2F override;
  auto mouthPosition(bool ignoreAdjustments) const -> Vec2F override;
  auto pullPendingChatActions() -> List<ChatAction> override;

  auto isInteractive() const -> bool override;
  auto interact(InteractRequest const& request) -> InteractAction override;
  auto interactiveBoundBox() const -> RectF override;

  auto loungingIn() const -> std::optional<EntityAnchorState> override;

  auto offeredQuests() const -> List<QuestArcDescriptor> override;
  auto turnInQuests() const -> StringSet override;
  auto questIndicatorPosition() const -> Vec2F override;

  auto lightSources() const -> List<LightSource> override;

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json> override;

  auto armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const -> Vec2F override;
  auto handOffset(ToolHand hand, Direction facingDirection) const -> Vec2F override;
  auto handPosition(ToolHand hand, Vec2F const& handOffset = {}) const -> Vec2F override;
  auto handItem(ToolHand hand) const -> Ptr<Item> override;
  auto armAdjustment() const -> Vec2F override;
  auto velocity() const -> Vec2F override;
  auto aimPosition() const -> Vec2F override;
  auto interactRadius() const -> float override;
  auto facingDirection() const -> Direction override;
  auto walkingDirection() const -> Direction override;
  auto isAdmin() const -> bool override;
  auto favoriteColor() const -> Color override;
  auto beamGunRadius() const -> float override;
  void addParticles(List<Particle> const& particles) override;
  void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) override;
  auto inToolRange() const -> bool override;
  auto inToolRange(Vec2F const& position) const -> bool override;
  void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) override;
  auto activeUniqueStatusEffectSummary() const -> ActiveUniqueStatusEffectSummary override;
  auto powerMultiplier() const -> float override;
  auto fullEnergy() const -> bool override;
  auto energy() const -> float override;
  auto energyLocked() const -> bool override;
  auto consumeEnergy(float energy) -> bool override;
  void queueUIMessage(String const& message) override;
  auto instrumentPlaying() -> bool override;
  void instrumentEquipped(String const& instrumentKind) override;
  void interact(InteractAction const& action) override;
  void addEffectEmitters(StringSet const& emitters) override;
  void requestEmote(String const& emote) override;
  auto movementController() -> ActorMovementController* override;
  auto statusController() -> StatusController* override;
  auto songbook() -> Songbook*;
  void setCameraFocusEntity(std::optional<EntityId> const& cameraFocusEntity) override;

  void playEmote(HumanoidEmote emote) override;

  auto damageSources() const -> List<DamageSource> override;

  auto forceRegions() const -> List<PhysicsForceRegion> override;

  using Entity::setUniqueId;

  auto identity() const -> HumanoidIdentity const&;
  void updateIdentity();
  void setIdentity(HumanoidIdentity identity);

  void setHumanoidParameter(String key, std::optional<Json> value);
  auto getHumanoidParameter(String key) -> std::optional<Json>;
  void setHumanoidParameters(JsonObject parameters);
  auto getHumanoidParameters() -> JsonObject;

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
  void setImagePath(std::optional<String> const& imagePath);

  void setFavoriteColor(Color color);
  void setName(String const& name);
  void setDescription(String const& description);

  auto humanoid() -> Ptr<Humanoid>;
  auto humanoid() const -> Ptr<Humanoid>;

  auto forceNude() const -> bool;

private:
  auto getAbsolutePosition(Vec2F relativePosition) const -> Vec2F;

  void tickShared(float dt);
  auto makeNpcCallbacks() -> LuaCallbacks;

  void setupNetStates();
  void getNetStates(bool initial);
  void setNetStates();

  void addChatMessage(String const& message, Json const& config, String const& portrait = "");
  void addEmote(HumanoidEmote const& emote);
  void setDance(std::optional<String> const& danceName);

  auto setItemSlot(String const& slot, ItemDescriptor itemDescriptor) -> bool;

  auto canUseTool() const -> bool;

  void disableWornArmor(bool disable);

  void refreshHumanoidParameters();

  NetElementDynamicGroup<NetHumanoid> m_netHumanoid;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
  NetworkedAnimator::DynamicTarget m_humanoidDynamicTarget;

  NpcVariant m_npcVariant;
  NetElementTopGroup m_netGroup;
  NetElementData<StringList> m_dropPools;

  NetElementData<std::optional<String>> m_uniqueIdNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;

  ClientEntityMode m_clientEntityMode;

  NetElementEnum<Humanoid::State> m_humanoidStateNetState;
  NetElementEnum<HumanoidEmote> m_humanoidEmoteStateNetState;
  NetElementData<std::optional<String>> m_humanoidDanceNetState;

  NetElementData<HumanoidIdentity> m_identityNetState;
  NetElementEvent m_refreshedHumanoidParameters;
  bool m_identityUpdated;

  NetElementData<std::optional<String>> m_deathParticleBurst;

  Ptr<ActorMovementController> m_movementController;
  Ptr<StatusController> m_statusController;
  Ptr<EffectEmitter> m_effectEmitter;

  NetElementBool m_aggressive;

  List<Ptr<BehaviorState>> m_behaviors;
  mutable LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_scriptComponent;

  List<ChatAction> m_pendingChatActions;
  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;
  NetElementData<Json> m_chatConfig;
  bool m_chatMessageUpdated;

  NetElementData<std::optional<String>> m_statusText;
  NetElementBool m_displayNametag;

  HumanoidEmote m_emoteState;
  GameTimer m_emoteCooldownTimer;
  std::optional<String> m_dance;
  GameTimer m_danceCooldownTimer;
  GameTimer m_blinkCooldownTimer;
  Vec2F m_blinkInterval;

  NetElementBool m_isInteractive;

  NetElementData<List<QuestArcDescriptor>> m_offeredQuests;
  NetElementData<StringSet> m_turnInQuests;

  Vec2F m_questIndicatorOffset;

  Ptr<ArmorWearer> m_armor;
  Ptr<ToolUser> m_tools;
  Ptr<Songbook> m_songbook;

  NetElementBool m_disableWornArmor;

  NetElementFloat m_xAimPosition;
  NetElementFloat m_yAimPosition;

  NetElementBool m_shifting;
  NetElementBool m_damageOnTouch;

  int m_hitDamageNotificationLimiter;
  int m_hitDamageNotificationLimit;
};

}// namespace Star
