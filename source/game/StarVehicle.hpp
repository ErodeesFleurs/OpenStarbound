#pragma once

#include "StarAssets.hpp"
#include "StarEntity.hpp"
#include "StarLoungingEntities.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarMovementController.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarScriptedEntity.hpp"

namespace Star {

struct VehicleExceptionTag {
  static constexpr char const* typeName = "VehicleException";
};
using VehicleException = TypedException<StarException, VehicleExceptionTag>;
class Vehicle;
using VehiclePtr = SharedPtr<Vehicle>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

class Vehicle : public virtual LoungeableEntity, public virtual InteractiveEntity, public virtual PhysicsEntity, public virtual ScriptedEntity {
public:
  Vehicle(AssetsConstPtr assets, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json baseConfig, String path, Json dynamicConfig);

  [[nodiscard]] String name() const override;

  [[nodiscard]] Json baseConfig() const;
  [[nodiscard]] Json dynamicConfig() const;

  [[nodiscard]] Json diskStore() const;
  void diskLoad(Json diskStore);

  [[nodiscard]] EntityType entityType() const override;
  [[nodiscard]] ClientEntityMode clientEntityMode() const override;

  [[nodiscard]] List<DamageSource> damageSources() const override;
  [[nodiscard]] Maybe<HitType> queryHit(DamageSource const& source) const override;
  [[nodiscard]] Maybe<PolyF> hitPoly() const override;

  [[nodiscard]] List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  [[nodiscard]] List<DamageNotification> selfDamageNotifications() override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  [[nodiscard]] Vec2F position() const override;
  [[nodiscard]] RectF metaBoundBox() const override;
  [[nodiscard]] RectF collisionArea() const override;
  [[nodiscard]] Vec2F velocity() const;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderer) override;

  void renderLightSources(RenderCallback* renderer) override;

  [[nodiscard]] List<LightSource> lightSources() const override;

  [[nodiscard]] bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  [[nodiscard]] RectF interactiveBoundBox() const override;
  [[nodiscard]] bool isInteractive() const override;
  [[nodiscard]] InteractAction interact(InteractRequest const& request) override;

  [[nodiscard]] size_t anchorCount() const override;
  [[nodiscard]] LoungeAnchorConstPtr loungeAnchor(size_t positionIndex) const override;
  void loungeControl(size_t positionIndex, LoungeControl loungeControl) override;
  void loungeAim(size_t positionIndex, Vec2F const& aimPosition) override;

  [[nodiscard]] List<PhysicsForceRegion> forceRegions() const override;
  [[nodiscard]] size_t movingCollisionCount() const override;
  [[nodiscard]] Maybe<PhysicsMovingCollision> movingCollision(size_t positionIndex) const override;

  [[nodiscard]] Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  [[nodiscard]] Maybe<LuaValue> evalScript(String const& code) override;

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

  enum class VehicleLayer { Back,
                            Passenger,
                            Front };

  [[nodiscard]] EntityRenderLayer renderLayer(VehicleLayer vehicleLayer) const;

  [[nodiscard]] LuaCallbacks makeVehicleCallbacks();
  [[nodiscard]] Json configValue(String const& name, Json def = {}) const;

  String m_typeName;
  Json m_baseConfig;
  ParticleDatabaseConstPtr m_particleDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
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

}// namespace Star
