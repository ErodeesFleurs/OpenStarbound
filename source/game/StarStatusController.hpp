#pragma once

#include "StarDamage.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNetElementDynamicGroup.hpp"
#include "StarNetElementExt.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarObserverStream.hpp"
#include "StarStatCollection.hpp"
#include "StarStatusEffectDatabase.hpp"

import std;

namespace Star {

class StatusController : public NetElement {
public:
  StatusController(Json const& config);

  auto diskStore() const -> Json;
  void diskLoad(Json const& store);

  auto statusProperty(String const& name, Json const& def = Json()) const -> Json;
  void setStatusProperty(String const& name, Json value);

  auto statNames() const -> StringList;
  auto stat(String const& statName) const -> float;
  // Returns true if the stat is strictly greater than zero
  auto statPositive(String const& statName) const -> bool;

  auto resourceNames() const -> StringList;
  auto isResource(String const& resourceName) const -> bool;
  auto resource(String const& resourceName) const -> float;
  // Returns true if the resource is strictly greater than zero
  auto resourcePositive(String const& resourceName) const -> bool;

  void setResource(String const& resourceName, float value);
  void modifyResource(String const& resourceName, float amount);

  auto giveResource(String const& resourceName, float amount) -> float;

  auto consumeResource(String const& resourceName, float amount) -> bool;
  auto overConsumeResource(String const& resourceName, float amount) -> bool;

  auto resourceLocked(String const& resourceName) const -> bool;
  void setResourceLocked(String const& resourceName, bool locked);

  // Resetting a resource also clears any locked states
  void resetResource(String const& resourceName);
  void resetAllResources();

  auto resourceMax(String const& resourceName) const -> std::optional<float>;
  auto resourcePercentage(String const& resourceName) const -> std::optional<float>;
  auto setResourcePercentage(String const& resourceName, float resourcePercentage) -> float;
  auto modifyResourcePercentage(String const& resourceName, float resourcePercentage) -> float;

  auto getPersistentEffects(String const& statEffectCategory) const -> List<PersistentStatusEffect>;
  void addPersistentEffect(String const& statEffectCategory, PersistentStatusEffect const& persistentEffect);
  void addPersistentEffects(String const& statEffectCategory, List<PersistentStatusEffect> const& persistentEffects);
  void setPersistentEffects(String const& statEffectCategory, List<PersistentStatusEffect> const& persistentEffects);
  void clearPersistentEffects(String const& statEffectCategory);
  void clearAllPersistentEffects();

  void addEphemeralEffect(EphemeralStatusEffect const& effect, std::optional<EntityId> sourceEntityId = {});
  void addEphemeralEffects(List<EphemeralStatusEffect> const& effectList, std::optional<EntityId> sourceEntityId = {});
  // Will have no effect if the unique effect is not applied ephemerally
  auto removeEphemeralEffect(UniqueStatusEffect const& uniqueEffect) -> bool;
  void clearEphemeralEffects();

  auto appliesEnvironmentStatusEffects() const -> bool;
  void setAppliesEnvironmentStatusEffects(bool appliesEnvironmentStatusEffects);

  // All unique stat effects, whether applied ephemerally or persistently, and
  // their remaining durations.
  auto activeUniqueStatusEffectSummary() const -> ActiveUniqueStatusEffectSummary;

  auto uniqueStatusEffectActive(String const& effectName) const -> bool;

  auto primaryDirectives() const -> const Directives&;
  void setPrimaryDirectives(Directives const& directives);

  // damage request and notification methods should only be called on the master controller.
  auto applyDamageRequest(DamageRequest const& damageRequest) -> List<DamageNotification>;
  void hitOther(EntityId targetEntityId, DamageRequest damageRequest);
  void damagedOther(DamageNotification damageNotification);
  auto pullSelfDamageNotifications() -> List<DamageNotification>;
  void applySelfDamageRequest(DamageRequest dr);

  // Pulls recent incoming and outgoing damage notifications.  In order for
  // multiple viewers keep track of notifications and avoid duplicates, the
  // damage notifications are indexed by a monotonically increasing 'step'
  // value.  Every call will return the recent damage notifications, along with
  // another step value to pass into the function on the next call to get
  // damage notifications SINCE the first call.  If since is 0, returns all
  // recent notifications available.
  auto damageTakenSince(std::uint64_t since = 0) const -> std::pair<List<DamageNotification>, std::uint64_t>;
  auto inflictedHitsSince(std::uint64_t since = 0) const -> std::pair<List<std::pair<EntityId, DamageRequest>>, std::uint64_t>;
  auto inflictedDamageSince(std::uint64_t since = 0) const -> std::pair<List<DamageNotification>, std::uint64_t>;

  void init(Entity* parentEntity, ActorMovementController* movementController);
  void uninit();

  void initNetVersion(NetElementVersion const* version = nullptr) override;

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  void enableNetInterpolation(float extrapolationHint = 0.0f) override;
  void disableNetInterpolation() override;
  void tickNetInterpolation(float dt) override;

  auto writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const -> bool override;
  void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  void blankNetDelta(float interpolationTime) override;

  void tickMaster(float dt);
  void tickSlave(float dt);

  auto parentDirectives() const -> const DirectivesGroup&;
  auto drawables() const -> List<Drawable>;
  auto lightSources() const -> List<LightSource>;
  auto overheadBars() -> List<OverheadBar>;
  auto toolUsageSuppressed() const -> bool;

  // new audios and particles will only be generated on the client
  auto pullNewAudios() -> List<Ptr<AudioInstance>>;
  auto pullNewParticles() -> List<Particle>;

  auto receiveMessage(String const& message, bool localMessage, JsonArray const& args = {}) -> std::optional<Json>;

private:
  using StatScript = LuaMessageHandlingComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>;

  struct EffectAnimator : public NetElement {
    EffectAnimator(std::optional<String> animationConfig = {});

    void initNetVersion(NetElementVersion const* version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    auto writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const -> bool override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    std::optional<String> animationConfig;
    NetworkedAnimator animator;
    NetworkedAnimator::DynamicTarget dynamicTarget;
  };
  using EffectAnimatorGroup = NetElementDynamicGroup<EffectAnimator>;

  struct UniqueEffectMetadata : public NetElementSyncGroup {
    UniqueEffectMetadata();
    UniqueEffectMetadata(UniqueStatusEffect effect, std::optional<float> duration, std::optional<EntityId> sourceEntityId);

    void netElementsNeedLoad(bool full) override;
    void netElementsNeedStore() override;

    UniqueStatusEffect effect;
    std::optional<float> duration;
    NetElementFloat durationNetState;
    NetElementFloat maxDuration;

    // If the sourceEntityId is not set here, this implies that the cause of
    // the unique effect was the owning entity.
    NetElementData<std::optional<EntityId>> sourceEntityId;
  };
  using UniqueEffectMetadataGroup = NetElementDynamicGroup<UniqueEffectMetadata>;

  struct PersistentEffectCategory {
    std::optional<StatModifierGroupId> modifierEffectsGroupId;
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

  auto defaultUniqueEffectDuration(UniqueStatusEffect const& name) const -> float;
  auto addUniqueEffect(UniqueStatusEffect const& effect, std::optional<float> duration, std::optional<EntityId> sourceEntityId) -> bool;
  void removeUniqueEffect(UniqueStatusEffect const& name);

  void initPrimaryScript();
  void uninitPrimaryScript();

  void initUniqueEffectScript(UniqueEffectInstance& uniqueEffect);
  void uninitUniqueEffectScript(UniqueEffectInstance& uniqueEffect);

  auto makeUniqueEffectCallbacks(UniqueEffectInstance& uniqueEffect) -> LuaCallbacks;

  NetElementGroup m_netGroup;
  StatCollection m_statCollection;
  NetElementOverride<NetElementHashMap<String, Json>> m_statusProperties;
  NetElementData<DirectivesGroup> m_parentDirectives;
  NetElementBool m_toolUsageSuppressed;

  UniqueEffectMetadataGroup m_uniqueEffectMetadata;
  EffectAnimatorGroup m_effectAnimators;

  Entity* m_parentEntity;
  ActorMovementController* m_movementController;

  // Members below are only valid on the master entity

  // there are two magic keys used for this map: 'entities' and 'environment' for StatusEffectEntity
  // and environmentally applied persistent status effects, respectively
  StringMap<PersistentEffectCategory> m_persistentEffects;
  StableHashMap<UniqueStatusEffect, UniqueEffectInstance> m_uniqueEffects;
  float m_minimumLiquidStatusEffectPercentage;
  bool m_appliesEnvironmentStatusEffects;
  bool m_appliesWeatherStatusEffects;
  GameTimer m_environmentStatusEffectUpdateTimer;

  std::optional<String> m_primaryAnimationConfig;
  StatScript m_primaryScript;
  Directives m_primaryDirectives;
  EffectAnimatorGroup::ElementId m_primaryAnimatorId;

  List<DamageNotification> m_pendingSelfDamageNotifications;

  ObserverStream<std::pair<EntityId, DamageRequest>> m_recentHitsGiven;
  ObserverStream<DamageNotification> m_recentDamageGiven;
  ObserverStream<DamageNotification> m_recentDamageTaken;
};

}// namespace Star
