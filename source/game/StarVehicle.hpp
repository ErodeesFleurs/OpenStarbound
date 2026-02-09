#pragma once

#include "StarConfig.hpp"
#include "StarEntity.hpp"
#include "StarException.hpp"
#include "StarLoungingEntities.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarMovementController.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarScriptedEntity.hpp"

import std;

namespace Star {

using VehicleException = ExceptionDerived<"VehicleException">;

class Vehicle : public virtual LoungeableEntity, public virtual InteractiveEntity, public virtual PhysicsEntity, public virtual ScriptedEntity {
public:
  Vehicle(Json baseConfig, String path, Json dynamicConfig);

  auto name() const -> String override;

  auto baseConfig() const -> Json;
  auto dynamicConfig() const -> Json;

  auto diskStore() const -> Json;
  void diskLoad(Json diskStore);

  auto entityType() const -> EntityType override;
  auto clientEntityMode() const -> ClientEntityMode override;

  auto damageSources() const -> List<DamageSource> override;
  auto queryHit(DamageSource const& source) const -> std::optional<HitType> override;
  auto hitPoly() const -> std::optional<PolyF> override;

  auto applyDamage(DamageRequest const& damage) -> List<DamageNotification> override;
  auto selfDamageNotifications() -> List<DamageNotification> override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto position() const -> Vec2F override;
  auto metaBoundBox() const -> RectF override;
  auto collisionArea() const -> RectF override;
  auto velocity() const -> Vec2F;

  auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  void update(float dt, std::uint64_t currentStep) override;

  void render(RenderCallback* renderer) override;

  void renderLightSources(RenderCallback* renderer) override;

  auto lightSources() const -> List<LightSource> override;

  auto shouldDestroy() const -> bool override;
  void destroy(RenderCallback* renderCallback) override;

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json> override;

  auto interactiveBoundBox() const -> RectF override;
  auto isInteractive() const -> bool override;
  auto interact(InteractRequest const& request) -> InteractAction override;

  auto anchorCount() const -> size_t override;
  auto loungeAnchor(size_t positionIndex) const -> ConstPtr<LoungeAnchor> override;
  void loungeControl(size_t positionIndex, LoungeControl loungeControl) override;
  void loungeAim(size_t positionIndex, Vec2F const& aimPosition) override;

  auto forceRegions() const -> List<PhysicsForceRegion> override;
  auto movingCollisionCount() const -> size_t override;
  auto movingCollision(size_t positionIndex) const -> std::optional<PhysicsMovingCollision> override;

  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> override;
  auto evalScript(String const& code) -> std::optional<LuaValue> override;

  void setPosition(Vec2F const& position);

private:
  struct MasterControlState {
    Set<ConnectionId> slavesHeld;
    bool masterHeld;
  };

  struct LoungePositionConfig {
    // The NetworkedAnimator part and part property which should control the
    // lounge position.
    String part;
    String partAnchor;
    std::optional<Vec2F> exitBottomOffset;
    JsonObject armorCosmeticOverrides;
    std::optional<String> cursorOverride;
    std::optional<bool> suppressTools;
    bool cameraFocus;

    NetElementBool enabled;
    NetElementEnum<LoungeOrientation> orientation;
    NetElementData<std::optional<String>> emote;
    NetElementData<std::optional<String>> dance;
    NetElementData<std::optional<String>> directives;
    NetElementData<List<PersistentStatusEffect>> statusEffects;

    Map<LoungeControl, MasterControlState> masterControlState;
    Vec2F masterAimPosition;

    Set<LoungeControl> slaveOldControls;
    Vec2F slaveOldAimPosition;
    Set<LoungeControl> slaveNewControls;
    Vec2F slaveNewAimPosition;
  };

  struct MovingCollisionConfig {
    PhysicsMovingCollision movingCollision;
    std::optional<String> attachToPart;
    NetElementBool enabled;
  };

  struct ForceRegionConfig {
    PhysicsForceRegion forceRegion;
    std::optional<String> attachToPart;
    NetElementBool enabled;
  };

  struct DamageSourceConfig {
    DamageSource damageSource;
    std::optional<String> attachToPart;
    NetElementBool enabled;
  };

  enum class VehicleLayer { Back,
                            Passenger,
                            Front };

  auto renderLayer(VehicleLayer vehicleLayer) const -> EntityRenderLayer;

  auto makeVehicleCallbacks() -> LuaCallbacks;
  auto configValue(String const& name, Json def = {}) const -> Json;

  String m_typeName;
  Json m_baseConfig;
  String m_path;
  Json m_dynamicConfig;
  RectF m_boundBox;
  float m_slaveControlTimeout = 0.0f;
  bool m_receiveExtraControls;
  OrderedHashMap<String, LoungePositionConfig> m_loungePositions;
  OrderedHashMap<String, MovingCollisionConfig> m_movingCollisions;
  OrderedHashMap<String, ForceRegionConfig> m_forceRegions;

  ClientEntityMode m_clientEntityMode;

  NetElementTopGroup m_netGroup;
  NetElementBool m_interactive;
  MovementController m_movementController;
  NetworkedAnimator m_networkedAnimator;
  NetworkedAnimator::DynamicTarget m_networkedAnimatorDynamicTarget;
  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>> m_scriptComponent;

  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;

  Map<ConnectionId, GameTimer> m_aliveMasterConnections;
  bool m_shouldDestroy = false;
  NetElementData<EntityDamageTeam> m_damageTeam;
  OrderedHashMap<String, DamageSourceConfig> m_damageSources;

  EntityRenderLayer m_baseRenderLayer;
  std::optional<EntityRenderLayer> m_overrideRenderLayer;

  GameTimer m_slaveHeartbeatTimer;
};

}// namespace Star
