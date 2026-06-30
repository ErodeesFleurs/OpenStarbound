#pragma once

#include "StarActorMovementController.hpp"
#include "StarArmorWearer.hpp"
#include "StarAssets.hpp"
#include "StarChattyEntity.hpp"
#include "StarDamageBarEntity.hpp"
#include "StarEffectEmitter.hpp"
#include "StarEmoteEntity.hpp"
#include "StarEntity.hpp"
#include "StarEntitySplash.hpp"
#include "StarHumanoid.hpp"
#include "StarInteractiveEntity.hpp"
#include "StarItemBag.hpp"
#include "StarLoungingEntities.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNametagEntity.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNpcDatabase.hpp"
#include "StarObjectDatabase.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarPortraitEntity.hpp"
#include "StarScriptedEntity.hpp"
#include "StarToolUser.hpp"
#include "StarToolUserEntity.hpp"

namespace Star {

class Songbook;
using SongbookPtr = SharedPtr<Songbook>;
class Item;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class RenderCallback;
class Npc;
class StatusController;
using StatusControllerPtr = SharedPtr<StatusController>;

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
  Npc(AssetsConstPtr assets, NpcDatabaseConstPtr npcDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, SpeciesDatabaseConstPtr speciesDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, NpcVariant const& npcVariant, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, LiquidsDatabaseConstPtr liquidsDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase);
  Npc(AssetsConstPtr assets, NpcDatabaseConstPtr npcDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, SpeciesDatabaseConstPtr speciesDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, NpcVariant const& npcVariant, Json const& initialState, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, LiquidsDatabaseConstPtr liquidsDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase);

  [[nodiscard]] Json diskStore() const;
  [[nodiscard]] ByteArray netStore(NetCompatibilityRules rules = {});

  [[nodiscard]] EntityType entityType() const override;
  [[nodiscard]] ClientEntityMode clientEntityMode() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  [[nodiscard]] Vec2F position() const override;
  [[nodiscard]] RectF metaBoundBox() const override;

  [[nodiscard]] Vec2F mouthOffset(bool ignoreAdjustments = true) const;
  [[nodiscard]] Vec2F feetOffset() const;
  [[nodiscard]] Vec2F headArmorOffset() const;
  [[nodiscard]] Vec2F chestArmorOffset() const;
  [[nodiscard]] Vec2F legsArmorOffset() const;
  [[nodiscard]] Vec2F backArmorOffset() const;

  [[nodiscard]] RectF collisionArea() const override;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  [[nodiscard]] String description() const override;
  [[nodiscard]] String species() const override;
  [[nodiscard]] Gender gender() const;
  [[nodiscard]] String npcType() const;

  [[nodiscard]] Json scriptConfigParameter(String const& parameterName, Json const& defaultValue = Json()) const;

  [[nodiscard]] Maybe<HitType> queryHit(DamageSource const& source) const override;
  [[nodiscard]] Maybe<PolyF> hitPoly() const override;

  void damagedOther(DamageNotification const& damage) override;

  [[nodiscard]] List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  [[nodiscard]] List<DamageNotification> selfDamageNotifications() override;

  [[nodiscard]] bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, uint64_t currentVersion) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  void setPosition(Vec2F const& pos);

  [[nodiscard]] float maxHealth() const override;
  [[nodiscard]] float health() const override;
  [[nodiscard]] DamageBarType damageBar() const override;

  [[nodiscard]] List<Drawable> portrait(PortraitMode mode) const override;
  [[nodiscard]] String name() const override;
  [[nodiscard]] Maybe<String> statusText() const override;
  [[nodiscard]] bool displayNametag() const override;
  [[nodiscard]] Vec3B nametagColor() const override;
  [[nodiscard]] Vec2F nametagOrigin() const override;
  [[nodiscard]] String nametag() const override;

  [[nodiscard]] bool aggressive() const;

  [[nodiscard]] Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  [[nodiscard]] Maybe<LuaValue> evalScript(String const& code) override;

  [[nodiscard]] Vec2F mouthPosition() const override;
  [[nodiscard]] Vec2F mouthPosition(bool ignoreAdjustments) const override;
  [[nodiscard]] List<ChatAction> pullPendingChatActions() override;

  [[nodiscard]] bool isInteractive() const override;
  [[nodiscard]] InteractAction interact(InteractRequest const& request) override;
  [[nodiscard]] RectF interactiveBoundBox() const override;

  [[nodiscard]] Maybe<EntityAnchorState> loungingIn() const override;

  [[nodiscard]] List<QuestArcDescriptor> offeredQuests() const override;
  [[nodiscard]] StringSet turnInQuests() const override;
  [[nodiscard]] Vec2F questIndicatorPosition() const override;

  [[nodiscard]] List<LightSource> lightSources() const override;

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  [[nodiscard]] Vec2F armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const override;
  [[nodiscard]] Vec2F handOffset(ToolHand hand, Direction facingDirection) const override;
  [[nodiscard]] Vec2F handPosition(ToolHand hand, Vec2F const& handOffset = {}) const override;
  [[nodiscard]] ItemPtr handItem(ToolHand hand) const override;
  [[nodiscard]] Vec2F armAdjustment() const override;
  [[nodiscard]] Vec2F velocity() const override;
  [[nodiscard]] Vec2F aimPosition() const override;
  [[nodiscard]] float interactRadius() const override;
  [[nodiscard]] Direction facingDirection() const override;
  [[nodiscard]] Direction walkingDirection() const override;
  [[nodiscard]] bool isAdmin() const override;
  [[nodiscard]] Color favoriteColor() const override;
  [[nodiscard]] float beamGunRadius() const override;
  void addParticles(List<Particle> const& particles) override;
  void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) override;
  [[nodiscard]] bool inToolRange() const override;
  [[nodiscard]] bool inToolRange(Vec2F const& position) const override;
  void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) override;
  [[nodiscard]] ActiveUniqueStatusEffectSummary activeUniqueStatusEffectSummary() const override;
  [[nodiscard]] float powerMultiplier() const override;
  [[nodiscard]] bool fullEnergy() const override;
  [[nodiscard]] float energy() const override;
  [[nodiscard]] bool energyLocked() const override;
  [[nodiscard]] bool consumeEnergy(float energy) override;
  void queueUIMessage(String const& message) override;
  [[nodiscard]] bool instrumentPlaying() override;
  void instrumentEquipped(String const& instrumentKind) override;
  void interact(InteractAction const& action) override;
  void addEffectEmitters(StringSet const& emitters) override;
  void requestEmote(String const& emote) override;
  [[nodiscard]] ActorMovementController* movementController() override;
  [[nodiscard]] StatusController* statusController() override;
  [[nodiscard]] Songbook* songbook();
  void setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) override;

  void playEmote(HumanoidEmote emote) override;

  [[nodiscard]] List<DamageSource> damageSources() const override;

  [[nodiscard]] List<PhysicsForceRegion> forceRegions() const override;

  using Entity::setUniqueId;

  [[nodiscard]] HumanoidIdentity const& identity() const;
  void updateIdentity();
  void setIdentity(HumanoidIdentity identity);

  void setHumanoidParameter(String key, Maybe<Json> value);
  [[nodiscard]] Maybe<Json> getHumanoidParameter(String key);
  void setHumanoidParameters(JsonObject parameters);
  [[nodiscard]] JsonObject getHumanoidParameters();

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

  [[nodiscard]] HumanoidPtr humanoid();
  [[nodiscard]] HumanoidPtr humanoid() const;

  [[nodiscard]] bool forceNude() const;

private:
  [[nodiscard]] Vec2F getAbsolutePosition(Vec2F relativePosition) const;

  void tickShared(float dt);
  [[nodiscard]] LuaCallbacks makeNpcCallbacks();

  void setupNetStates();
  void getNetStates(bool initial);
  void setNetStates();

  void addChatMessage(String const& message, Json const& config, String const& portrait = "");
  void addEmote(HumanoidEmote const& emote);
  void setDance(Maybe<String> const& danceName);

  [[nodiscard]] bool setItemSlot(String const& slot, ItemDescriptor itemDescriptor);

  [[nodiscard]] bool canUseTool() const;

  void disableWornArmor(bool disable);

  void refreshHumanoidParameters();

  NetElementDynamicGroup<NetHumanoid> m_netHumanoid;
  AssetsConstPtr m_assets;
  NpcDatabaseConstPtr m_npcDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  DanceDatabaseConstPtr m_danceDatabase;
  EmoteProcessorConstPtr m_emoteProcessor;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
  NetworkedAnimator::DynamicTarget m_humanoidDynamicTarget;

  NpcVariant m_npcVariant;
  NetElementTopGroup m_netGroup;
  NetElementData<StringList> m_dropPools;

  NetElementData<Maybe<String>> m_uniqueIdNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;

  ClientEntityMode m_clientEntityMode;

  NetElementEnum<Humanoid::State> m_humanoidStateNetState;
  NetElementEnum<HumanoidEmote> m_humanoidEmoteStateNetState;
  NetElementData<Maybe<String>> m_humanoidDanceNetState;

  NetElementData<HumanoidIdentity> m_identityNetState;
  NetElementEvent m_refreshedHumanoidParameters;
  bool m_identityUpdated;

  NetElementData<Maybe<String>> m_deathParticleBurst;

  ActorMovementControllerPtr m_movementController;
  StatusControllerPtr m_statusController;
  EffectEmitterPtr m_effectEmitter;

  NetElementBool m_aggressive;

  List<BehaviorStatePtr> m_behaviors;
  mutable LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_scriptComponent;

  List<ChatAction> m_pendingChatActions;
  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;
  NetElementData<Json> m_chatConfig;
  bool m_chatMessageUpdated;

  NetElementData<Maybe<String>> m_statusText;
  NetElementBool m_displayNametag;

  HumanoidEmote m_emoteState;
  GameTimer m_emoteCooldownTimer;
  Maybe<String> m_dance;
  GameTimer m_danceCooldownTimer;
  GameTimer m_blinkCooldownTimer;
  Vec2F m_blinkInterval;

  NetElementBool m_isInteractive;

  NetElementData<List<QuestArcDescriptor>> m_offeredQuests;
  NetElementData<StringSet> m_turnInQuests;

  Vec2F m_questIndicatorOffset;

  ArmorWearerPtr m_armor;
  ToolUserPtr m_tools;
  SongbookPtr m_songbook;

  NetElementBool m_disableWornArmor;

  NetElementFloat m_xAimPosition;
  NetElementFloat m_yAimPosition;

  NetElementBool m_shifting;
  NetElementBool m_damageOnTouch;

  int m_hitDamageNotificationLimiter;
  int m_hitDamageNotificationLimit;
};

}// namespace Star
