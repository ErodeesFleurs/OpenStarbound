#pragma once

#include "StarDamage.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNetElementExt.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarObserverStream.hpp"
#include "StarStatCollection.hpp"
#include "StarStatusEffectDatabase.hpp"

namespace Star {

class LiquidsDatabase;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;
class Assets;
using AssetsConstPtr = SharedPtr<Assets const>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

class StatusController;
using StatusControllerPtr = SharedPtr<StatusController>;

class StatusController : public NetElement {
public:
  StatusController(Json const& config, AssetsConstPtr assets, LiquidsDatabaseConstPtr liquidsDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  StatusController(Entity& parentEntity, ActorMovementController& movementController, AssetsConstPtr assets, LiquidsDatabaseConstPtr liquidsDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] Json diskStore() const;
  void diskLoad(Json const& store);

  [[nodiscard]] Json statusProperty(String const& name, Json const& def = Json()) const;
  void setStatusProperty(String const& name, Json value);

  [[nodiscard]] StringList statNames() const;
  [[nodiscard]] float stat(String const& statName) const;
  // Returns true if the stat is strictly greater than zero
  [[nodiscard]] bool statPositive(String const& statName) const;

  [[nodiscard]] StringList resourceNames() const;
  [[nodiscard]] bool isResource(String const& resourceName) const;
  [[nodiscard]] float resource(String const& resourceName) const;
  // Returns true if the resource is strictly greater than zero
  [[nodiscard]] bool resourcePositive(String const& resourceName) const;

  void setResource(String const& resourceName, float value);
  void modifyResource(String const& resourceName, float amount);

  [[nodiscard]] float giveResource(String const& resourceName, float amount);

  [[nodiscard]] bool consumeResource(String const& resourceName, float amount);
  [[nodiscard]] bool overConsumeResource(String const& resourceName, float amount);

  [[nodiscard]] bool resourceLocked(String const& resourceName) const;
  void setResourceLocked(String const& resourceName, bool locked);

  // Resetting a resource also clears any locked states
  void resetResource(String const& resourceName);
  void resetAllResources();

  [[nodiscard]] Maybe<float> resourceMax(String const& resourceName) const;
  [[nodiscard]] Maybe<float> resourcePercentage(String const& resourceName) const;
  [[nodiscard]] float setResourcePercentage(String const& resourceName, float resourcePercentage);
  [[nodiscard]] float modifyResourcePercentage(String const& resourceName, float resourcePercentage);

  [[nodiscard]] List<PersistentStatusEffect> getPersistentEffects(String const& statEffectCategory) const;
  void addPersistentEffect(String const& statEffectCategory, PersistentStatusEffect const& persistentEffect);
  void addPersistentEffects(String const& statEffectCategory, List<PersistentStatusEffect> const& persistentEffects);
  void setPersistentEffects(String const& statEffectCategory, List<PersistentStatusEffect> const& persistentEffects);
  void clearPersistentEffects(String const& statEffectCategory);
  void clearAllPersistentEffects();

  void addEphemeralEffect(EphemeralStatusEffect const& effect, Maybe<EntityId> sourceEntityId = {});
  void addEphemeralEffects(List<EphemeralStatusEffect> const& effectList, Maybe<EntityId> sourceEntityId = {});
  // Will have no effect if the unique effect is not applied ephemerally
  [[nodiscard]] bool removeEphemeralEffect(UniqueStatusEffect const& uniqueEffect);
  void clearEphemeralEffects();

  [[nodiscard]] bool appliesEnvironmentStatusEffects() const;
  void setAppliesEnvironmentStatusEffects(bool appliesEnvironmentStatusEffects);

  // All unique stat effects, whether applied ephemerally or persistently, and
  // their remaining durations.
  [[nodiscard]] ActiveUniqueStatusEffectSummary activeUniqueStatusEffectSummary() const;

  [[nodiscard]] bool uniqueStatusEffectActive(String const& effectName) const;

  [[nodiscard]] const Directives& primaryDirectives() const;
  void setPrimaryDirectives(Directives const& directives);

  // damage request and notification methods should only be called on the master controller.
  [[nodiscard]] List<DamageNotification> applyDamageRequest(DamageRequest const& damageRequest);
  void hitOther(EntityId targetEntityId, DamageRequest damageRequest);
  void damagedOther(DamageNotification damageNotification);
  [[nodiscard]] List<DamageNotification> pullSelfDamageNotifications();
  void applySelfDamageRequest(DamageRequest dr);

  // Pulls recent incoming and outgoing damage notifications.  In order for
  // multiple viewers keep track of notifications and avoid duplicates, the
  // damage notifications are indexed by a monotonically increasing 'step'
  // value.  Every call will return the recent damage notifications, along with
  // another step value to pass into the function on the next call to get
  // damage notifications SINCE the first call.  If since is 0, returns all
  // recent notifications available.
  [[nodiscard]] pair<List<DamageNotification>, uint64_t> damageTakenSince(uint64_t since = 0) const;
  [[nodiscard]] pair<List<pair<EntityId, DamageRequest>>, uint64_t> inflictedHitsSince(uint64_t since = 0) const;
  [[nodiscard]] pair<List<DamageNotification>, uint64_t> inflictedDamageSince(uint64_t since = 0) const;

  void init(Entity& parentEntity, ActorMovementController& movementController);
  void uninit();

  void initNetVersion(NetElementVersion const* version = nullptr) override;

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  void enableNetInterpolation(float extrapolationHint = 0.0f) override;
  void disableNetInterpolation() override;
  void tickNetInterpolation(float dt) override;

  [[nodiscard]] bool writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
  void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  void blankNetDelta(float interpolationTime) override;

  void tickMaster(float dt);
  void tickSlave(float dt);

  [[nodiscard]] const DirectivesGroup& parentDirectives() const;
  [[nodiscard]] List<Drawable> drawables() const;
  [[nodiscard]] List<LightSource> lightSources() const;
  [[nodiscard]] List<OverheadBar> overheadBars();
  [[nodiscard]] bool toolUsageSuppressed() const;

  // new audios and particles will only be generated on the client
  [[nodiscard]] List<AudioInstancePtr> pullNewAudios();
  [[nodiscard]] List<Particle> pullNewParticles();

  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

private:
  using StatScript = LuaMessageHandlingComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>;

  struct EffectAnimator : public NetElement {
    EffectAnimator(Maybe<String> animationConfig = {}, AssetsConstPtr assets = {}, ParticleDatabaseConstPtr particleDatabase = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {});

    void initNetVersion(NetElementVersion const* version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    [[nodiscard]] bool writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    Maybe<String> animationConfig;
    AssetsConstPtr assets;
    ParticleDatabaseConstPtr particleDatabase;
    ImageMetadataDatabaseConstPtr imageMetadataDatabase;
    NetworkedAnimator animator;
    NetworkedAnimator::DynamicTarget dynamicTarget;
  };
  using EffectAnimatorGroup = NetElementDynamicGroup<EffectAnimator>;

  struct UniqueEffectMetadata : public NetElementSyncGroup {
    UniqueEffectMetadata();
    UniqueEffectMetadata(UniqueStatusEffect effect, Maybe<float> duration, Maybe<EntityId> sourceEntityId);

    void netElementsNeedLoad(bool full) override;
    void netElementsNeedStore() override;

    UniqueStatusEffect effect;
    Maybe<float> duration;
    NetElementFloat durationNetState;
    NetElementFloat maxDuration;

    // If the sourceEntityId is not set here, this implies that the cause of
    // the unique effect was the owning entity.
    NetElementData<Maybe<EntityId>> sourceEntityId;
  };
  using UniqueEffectMetadataGroup = NetElementDynamicGroup<UniqueEffectMetadata>;

  struct PersistentEffectCategory {
    Maybe<StatModifierGroupId> modifierEffectsGroupId;
    List<StatModifier> statModifiers;
    HashSet<UniqueStatusEffect> uniqueEffects;
  };

  struct UniqueEffectInstance {
    UniqueStatusEffectConfig effectConfig;
    Directives parentDirectives;
    HashSet<StatModifierGroupId> modifierGroups;
    StatScript script;
    UniqueEffectMetadataGroup::ElementId metadataId;
    EffectAnimatorGroup::ElementId animatorId;
    bool toolUsageSuppressed;
  };

  void updateAnimators(float dt);
  void updatePersistentUniqueEffects();

  [[nodiscard]] float defaultUniqueEffectDuration(UniqueStatusEffect const& name) const;
  [[nodiscard]] bool addUniqueEffect(UniqueStatusEffect const& effect, Maybe<float> duration, Maybe<EntityId> sourceEntityId);
  void removeUniqueEffect(UniqueStatusEffect const& name);

  void initPrimaryScript();
  void uninitPrimaryScript();

  void initUniqueEffectScript(UniqueEffectInstance& uniqueEffect);
  void uninitUniqueEffectScript(UniqueEffectInstance& uniqueEffect);

  [[nodiscard]] LuaCallbacks makeUniqueEffectCallbacks(UniqueEffectInstance& uniqueEffect);

  NetElementGroup m_netGroup;
  StatCollection m_statCollection;
  NetElementOverride<NetElementHashMap<String, Json>> m_statusProperties;
  NetElementData<DirectivesGroup> m_parentDirectives;
  NetElementBool m_toolUsageSuppressed;

  UniqueEffectMetadataGroup m_uniqueEffectMetadata;
  EffectAnimatorGroup m_effectAnimators;

  Entity* m_parentEntity;
  ActorMovementController* m_movementController;
  AssetsConstPtr m_assets;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;

  // Members below are only valid on the master entity

  // there are two magic keys used for this map: 'entities' and 'environment' for StatusEffectEntity
  // and environmentally applied persistent status effects, respectively
  StringMap<PersistentEffectCategory> m_persistentEffects;
  StableHashMap<UniqueStatusEffect, UniqueEffectInstance> m_uniqueEffects;
  float m_minimumLiquidStatusEffectPercentage;
  bool m_appliesEnvironmentStatusEffects;
  bool m_appliesWeatherStatusEffects;
  GameTimer m_environmentStatusEffectUpdateTimer;

  Maybe<String> m_primaryAnimationConfig;
  StatScript m_primaryScript;
  Directives m_primaryDirectives;
  EffectAnimatorGroup::ElementId m_primaryAnimatorId;

  List<DamageNotification> m_pendingSelfDamageNotifications;

  ObserverStream<pair<EntityId, DamageRequest>> m_recentHitsGiven;
  ObserverStream<DamageNotification> m_recentDamageGiven;
  ObserverStream<DamageNotification> m_recentDamageTaken;
};

}// namespace Star
