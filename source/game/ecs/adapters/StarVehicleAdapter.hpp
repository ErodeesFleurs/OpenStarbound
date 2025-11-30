#pragma once

// ECS Vehicle Adapter for OpenStarbound
// This adapter implements the Vehicle entity using ECS components
// Vehicles are movable entities that players can mount and control
// (cars, boats, mechs, ships, etc.)

#include "StarEntityAdapter.hpp"
#include "StarVehicleDatabase.hpp"
#include "StarLoungingEntities.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarScriptedEntity.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarMovementController.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarGameTimers.hpp"

namespace Star {
namespace ECS {

// Lounge position configuration (matching original Vehicle)
struct VehicleLoungePositionConfig {
  String part;
  String partAnchor;
  Maybe<Vec2F> exitBottomOffset;
  JsonObject armorCosmeticOverrides;
  Maybe<String> cursorOverride;
  Maybe<bool> suppressTools;
  bool cameraFocus = false;
  
  bool enabled = true;
  LoungeOrientation orientation = LoungeOrientation::None;
  Maybe<String> emote;
  Maybe<String> dance;
  Maybe<String> directives;
  List<PersistentStatusEffect> statusEffects;
  
  // Control state
  Set<LoungeControl> masterControls;
  Vec2F masterAimPosition;
  Set<LoungeControl> slaveOldControls;
  Vec2F slaveOldAimPosition;
  Set<LoungeControl> slaveNewControls;
  Vec2F slaveNewAimPosition;
};

// Moving collision configuration
struct VehicleMovingCollisionConfig {
  PhysicsMovingCollision movingCollision;
  Maybe<String> attachToPart;
  bool enabled = true;
};

// Force region configuration
struct VehicleForceRegionConfig {
  PhysicsForceRegion forceRegion;
  Maybe<String> attachToPart;
  bool enabled = true;
};

// Damage source configuration
struct VehicleDamageSourceConfig {
  DamageSource damageSource;
  Maybe<String> attachToPart;
  bool enabled = true;
};

// Vehicle rendering layer
enum class VehicleLayerType {
  Back,
  Passenger,
  Front
};

// Vehicle-specific component storing all vehicle state
struct VehicleDataComponent {
  // Configuration
  String typeName;
  Json baseConfig;
  String path;
  Json dynamicConfig;
  
  // Bounds
  RectF boundBox;
  
  // Lounge positions
  OrderedHashMap<String, VehicleLoungePositionConfig> loungePositions;
  
  // Physics
  OrderedHashMap<String, VehicleMovingCollisionConfig> movingCollisions;
  OrderedHashMap<String, VehicleForceRegionConfig> forceRegions;
  
  // Damage
  EntityDamageTeam damageTeam;
  OrderedHashMap<String, VehicleDamageSourceConfig> damageSources;
  
  // Rendering
  EntityRenderLayer baseRenderLayer = RenderLayerVehicle;
  Maybe<EntityRenderLayer> overrideRenderLayer;
  
  // Network/control
  float slaveControlTimeout = 0.0f;
  bool receiveExtraControls = false;
  Map<ConnectionId, GameTimer> aliveMasterConnections;
  GameTimer slaveHeartbeatTimer;
  
  // State
  bool shouldDestroy = false;
  bool interactive = true;
  ClientEntityMode clientEntityMode = ClientEntityMode::ClientSlaveOnly;
  
  // Animation
  StringMap<Json> scriptedAnimationParameters;
};

// Tag component for identifying vehicles
struct VehicleTag {};

// Vehicle adapter that wraps ECS entity to implement Vehicle interfaces
class VehicleAdapter : public EntityAdapter,
                       public virtual LoungeableEntity,
                       public virtual InteractiveEntity,
                       public virtual PhysicsEntity,
                       public virtual ScriptedEntity {
public:
  // Create from config
  static shared_ptr<VehicleAdapter> create(
    World* ecsWorld,
    Json baseConfig,
    String path,
    Json dynamicConfig);
  
  // Create from disk store
  static shared_ptr<VehicleAdapter> createFromDiskStore(
    World* ecsWorld,
    Json const& diskStore);
  
  // Construct from existing ECS entity
  VehicleAdapter(World* ecsWorld, Entity ecsEntity);
  
  // Accessors
  String name() const override;
  Json baseConfig() const;
  Json dynamicConfig() const;
  
  // Serialization
  Json diskStore() const;
  void diskLoad(Json diskStore);
  
  // Entity interface
  EntityType entityType() const override;
  ClientEntityMode clientEntityMode() const override;
  
  List<DamageSource> damageSources() const override;
  Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;
  
  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  List<DamageNotification> selfDamageNotifications() override;
  
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  Vec2F position() const override;
  RectF metaBoundBox() const override;
  RectF collisionArea() const override;
  Vec2F velocity() const;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;
  
  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderer) override;
  void renderLightSources(RenderCallback* renderer) override;
  
  List<LightSource> lightSources() const override;
  
  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;
  
  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;
  
  // InteractiveEntity interface
  RectF interactiveBoundBox() const override;
  bool isInteractive() const override;
  InteractAction interact(InteractRequest const& request) override;
  
  // LoungeableEntity interface
  size_t anchorCount() const override;
  LoungeAnchorConstPtr loungeAnchor(size_t positionIndex) const override;
  void loungeControl(size_t positionIndex, LoungeControl loungeControl) override;
  void loungeAim(size_t positionIndex, Vec2F const& aimPosition) override;
  
  // PhysicsEntity interface
  List<PhysicsForceRegion> forceRegions() const override;
  size_t movingCollisionCount() const override;
  Maybe<PhysicsMovingCollision> movingCollision(size_t positionIndex) const override;
  
  // ScriptedEntity interface
  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;
  
  void setPosition(Vec2F const& position);

private:
  EntityRenderLayer renderLayer(VehicleLayerType vehicleLayer) const;
  
  LuaCallbacks makeVehicleCallbacks();
  Json configValue(String const& name, Json def = {}) const;
  
  void setupNetStates();
  void getNetStates();
  void setNetStates();
  
  // Movement
  MovementController m_movementController;
  
  // Animation
  NetworkedAnimator m_networkedAnimator;
  NetworkedAnimator::DynamicTarget m_networkedAnimatorDynamicTarget;
  
  // Scripting
  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>> m_scriptComponent;
  LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  
  // Network state
  NetElementTopGroup m_netGroup;
  NetElementBool m_interactiveNetState;
  NetElementData<EntityDamageTeam> m_damageTeamNetState;
  NetElementHashMap<String, Json> m_scriptedAnimationParametersNetState;
  
  // Per-lounge position network state
  struct LoungePositionNetState {
    NetElementBool enabled;
    NetElementEnum<LoungeOrientation> orientation;
    NetElementData<Maybe<String>> emote;
    NetElementData<Maybe<String>> dance;
    NetElementData<Maybe<String>> directives;
    NetElementData<List<PersistentStatusEffect>> statusEffects;
  };
  OrderedHashMap<String, LoungePositionNetState> m_loungePositionNetStates;
  
  // Per-moving collision network state
  OrderedHashMap<String, NetElementBool> m_movingCollisionEnabledNetStates;
  
  // Per-force region network state
  OrderedHashMap<String, NetElementBool> m_forceRegionEnabledNetStates;
  
  // Per-damage source network state
  OrderedHashMap<String, NetElementBool> m_damageSourceEnabledNetStates;
  
  // Cached lounge anchors
  mutable Map<size_t, LoungeAnchorPtr> m_loungeAnchorCache;
};

using VehicleAdapterPtr = shared_ptr<VehicleAdapter>;

} // namespace ECS
} // namespace Star
