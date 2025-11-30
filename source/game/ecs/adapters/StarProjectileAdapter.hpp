#pragma once

// ECS Projectile Adapter for OpenStarbound
// This adapter implements the Projectile entity using ECS components

#include "StarEntityAdapter.hpp"
#include "StarProjectileDatabase.hpp"
#include "StarEffectEmitter.hpp"
#include "StarLuaComponents.hpp"
#include "StarLine.hpp"
#include "StarOrderedMap.hpp"

namespace Star {
namespace ECS {

// Projectile-specific component storing all projectile state
struct ProjectileDataComponent {
  // Config
  ProjectileConfigPtr config;
  Json parameters;
  
  // Movement
  float acceleration = 0.0f;
  float initialSpeed = 0.0f;
  Maybe<Vec2F> referenceVelocity;
  
  // Damage
  float power = 0.0f;
  float powerMultiplier = 1.0f;
  String damageKind;
  DamageType damageType = DamageType::NoDamage;
  Maybe<String> damageRepeatGroup;
  Maybe<float> damageRepeatTimeout;
  Json damageTeam;
  
  // Physics
  bool rayCheckToSource = false;
  bool falldown = false;
  bool hydrophobic = false;
  bool onlyHitTerrain = false;
  int bounces = 0;
  
  // State
  float timeToLive = 0.0f;
  bool collision = false;
  bool wasColliding = false;
  Vec2I collisionTile;
  Vec2I lastNonCollidingTile;
  Line2F travelLine;
  
  // Source tracking
  EntityId sourceEntity = NullEntityId;
  bool trackSourceEntity = false;
  Vec2F lastEntityPosition;
  
  // Animation
  int frame = 0;
  float animationTimer = 0.0f;
  float animationCycle = 1.0f;
  Directives imageDirectives;
  String imageSuffix;
  
  // Audio
  String persistentAudioFile;
  AudioInstancePtr persistentAudio;
  
  // Periodic actions
  List<tuple<GameTimer, bool, Json>> periodicActions;
};

// Projectile adapter that wraps ECS entity
class ProjectileAdapter : public EntityAdapter {
public:
  // Create from config and parameters
  static shared_ptr<ProjectileAdapter> create(
    World* ecsWorld,
    ProjectileConfigPtr const& config,
    Json const& parameters);
  
  // Create from network data
  static shared_ptr<ProjectileAdapter> createFromNet(
    World* ecsWorld,
    ProjectileConfigPtr const& config,
    ByteArray const& netStore,
    NetCompatibilityRules rules = {});
  
  // Construct from existing ECS entity
  ProjectileAdapter(World* ecsWorld, Entity ecsEntity);
  
  // Serialization
  ByteArray netStore(NetCompatibilityRules rules = {}) const;
  
  // Entity interface
  EntityType entityType() const override;
  
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  String name() const override;
  String description() const override;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;
  
  Vec2F position() const override;
  RectF metaBoundBox() const override;
  
  bool ephemeral() const override;
  ClientEntityMode clientEntityMode() const override;
  bool masterOnly() const override;
  
  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;
  
  List<DamageSource> damageSources() const override;
  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  
  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;
  
  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;
  
  // Projectile-specific methods
  String projectileType() const;
  Json configValue(String const& name, Json const& def = Json()) const;
  
  Vec2F velocity() const;
  void setVelocity(Vec2F const& velocity);
  
  float initialSpeed() const;
  void setInitialSpeed(float speed);
  
  void setInitialPosition(Vec2F const& position);
  void setInitialDirection(Vec2F const& direction);
  void setInitialVelocity(Vec2F const& velocity);
  
  void setReferenceVelocity(Maybe<Vec2F> const& velocity);
  
  float powerMultiplier() const;
  void setPowerMultiplier(float multiplier);
  
  void setSourceEntity(EntityId source, bool trackSource);
  EntityId sourceEntity() const;
  
  float rotation() const;
  void setRotation(float rotation);

private:
  void setupComponents(ProjectileConfigPtr const& config, Json const& parameters);
  void processAction(Json const& action);
  void tickShared(float dt);
  int getFrame() const;
  void renderPendingRenderables(RenderCallback* renderCallback);
  
  // Rotation stored separately
  float m_rotation = 0.0f;
  
  // Effect emitter
  EffectEmitterPtr m_effectEmitter;
  
  // Pending renderables
  List<Variant<AudioInstancePtr, Particle, LightSource>> m_pendingRenderables;
  
  // Script component (optional)
  mutable LuaMessageHandlingComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptComponent;
};

using ProjectileAdapterPtr = shared_ptr<ProjectileAdapter>;

} // namespace ECS
} // namespace Star
