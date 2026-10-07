module;
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarColor.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarLua.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.periodic_function;
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;
import star.physics_entity;
import star.mobile_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.movement_controller;
#include "StarLuaComponents.hpp"
import star.anchorable_entity;
import star.entity_rendering_types;
import star.lounging_entities;
import star.scripted_entity;
#include "StarLuaAnimationComponent.hpp"

namespace Star {

struct VehicleExceptionTag {
  static constexpr char const* name() { return "VehicleException"; }
};
using VehicleException = StarError<VehicleExceptionTag, StarException>;
STAR_CLASS(Vehicle);

class Vehicle : 
  public virtual MobileEntity,
  public virtual LoungeableEntity, 
  public virtual InteractiveEntity, 
  public virtual PhysicsEntity, 
  public virtual ScriptedEntity {
public:
  Vehicle(Json baseConfig, String path, Json dynamicConfig);

  String name() const override;

  Json baseConfig() const;
  Json dynamicConfig() const;

  Json diskStore() const;
  void diskLoad(Json diskStore);

  EntityType entityType() const override;
  ClientEntityMode clientEntityMode() const override;

  List<DamageSource> damageSources() const override;
  Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;

  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  List<DamageNotification> selfDamageNotifications() override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
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

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  RectF interactiveBoundBox() const override;
  bool isInteractive() const override;
  InteractAction interact(InteractRequest const& request) override;

  size_t anchorCount() const override;
  LoungeAnchorConstPtr loungeAnchor(size_t positionIndex) const override;
  void loungeControl(size_t positionIndex, LoungeControl loungeControl) override;
  void loungeAim(size_t positionIndex, Vec2F const& aimPosition) override;

  List<PhysicsForceRegion> forceRegions() const override;
  size_t movingCollisionCount() const override;
  Maybe<PhysicsMovingCollision> movingCollision(size_t positionIndex) const override;

  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;
  
  MovementController* movementController() override;

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
    Maybe<Vec2F> exitBottomOffset;
    JsonObject armorCosmeticOverrides;
    Maybe<String> cursorOverride;
    Maybe<bool> suppressTools;
    bool cameraFocus;

    NetElementBool enabled;
    NetElementEnum<LoungeOrientation> orientation;
    NetElementData<Maybe<String>> emote;
    NetElementData<Maybe<String>> dance;
    NetElementData<Maybe<String>> directives;
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
    Maybe<String> attachToPart;
    NetElementBool enabled;
  };

  struct ForceRegionConfig {
    PhysicsForceRegion forceRegion;
    Maybe<String> attachToPart;
    NetElementBool enabled;
  };

  struct DamageSourceConfig {
    DamageSource damageSource;
    Maybe<String> attachToPart;
    NetElementBool enabled;
  };

  enum class VehicleLayer { Back, Passenger, Front };

  EntityRenderLayer renderLayer(VehicleLayer vehicleLayer) const;

  LuaCallbacks makeVehicleCallbacks();
  Json configValue(String const& name, Json def = {}) const;

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
  Maybe<EntityRenderLayer> m_overrideRenderLayer;
  
  GameTimer m_slaveHeartbeatTimer;
};

}

export module star.vehicle;

export namespace Star {
  using ::Star::VehicleExceptionTag;
  using ::Star::VehicleException;
  using ::Star::Vehicle;
  using ::Star::VehiclePtr;
  using ::Star::VehicleConstPtr;
  using ::Star::VehicleWeakPtr;
  using ::Star::VehicleConstWeakPtr;
  using ::Star::VehicleUPtr;
  using ::Star::VehicleConstUPtr;
}
