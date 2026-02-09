#pragma once

#include "StarConfig.hpp"
#include "StarEffectEmitter.hpp"
#include "StarEntity.hpp"
#include "StarLuaComponents.hpp"
#include "StarMovementController.hpp"
#include "StarNetElementSystem.hpp"
#include "StarOrderedMap.hpp"
#include "StarParticle.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarScriptedEntity.hpp"
#include "StarStatusEffectEntity.hpp"

import std;

namespace Star {

struct ProjectileConfig;

class Projectile : public virtual Entity, public virtual ScriptedEntity, public virtual PhysicsEntity, public virtual StatusEffectEntity {
public:
  Projectile(Ptr<ProjectileConfig> const& config, Json const& parameters);
  Projectile(Ptr<ProjectileConfig> const& config, DataStreamBuffer& netState, NetCompatibilityRules rules = {});

  auto netStore(NetCompatibilityRules rules = {}) const -> ByteArray;

  auto entityType() const -> EntityType override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto typeName() const -> String;
  auto name() const -> String override;
  auto description() const -> String override;

  auto position() const -> Vec2F override;
  auto metaBoundBox() const -> RectF override;

  auto velocity() const -> Vec2F;

  auto ephemeral() const -> bool override;
  auto clientEntityMode() const -> ClientEntityMode override;
  auto masterOnly() const -> bool override;

  auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  // If the bullet time to live has run out, or if it has collided, etc this
  // will return true.
  auto shouldDestroy() const -> bool override;
  void destroy(RenderCallback* renderCallback) override;

  auto damageSources() const -> List<DamageSource> override;
  void hitOther(EntityId targetEntityId, DamageRequest const& dr) override;

  void update(float dt, std::uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json> override;

  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> override;
  auto evalScript(String const& code) -> std::optional<LuaValue> override;

  auto projectileType() const -> String;

  auto configValue(String const& name, Json const& def = Json()) const -> Json;

  // InitialPosition, InitialDirection, InitialVelocity, PowerMultiplier, and
  // additional status effects must be set before the projectile is added to
  // the world

  auto initialSpeed() const -> float;
  void setInitialSpeed(float speed);

  void setInitialPosition(Vec2F const& position);
  void setInitialDirection(Vec2F const& direction);
  // Overrides internal "speed" parameter
  void setInitialVelocity(Vec2F const& velocity);

  void setReferenceVelocity(std::optional<Vec2F> const& velocity);

  auto powerMultiplier() const -> float;
  void setPowerMultiplier(float multiplier);

  // If trackSource is true, then the projectile will (while the entity exists)
  // attempt to track the change in position of the parent entity and move
  // relative to it.
  void setSourceEntity(EntityId source, bool trackSource);
  auto sourceEntity() const -> EntityId;

  auto statusEffects() const -> List<PersistentStatusEffect> override;
  auto statusEffectArea() const -> PolyF override;

  auto forceRegions() const -> List<PhysicsForceRegion> override;
  auto movingCollisionCount() const -> size_t override;
  auto movingCollision(size_t positionIndex) const -> std::optional<PhysicsMovingCollision> override;

  using Entity::setTeam;

private:
  struct PhysicsForceConfig {
    PhysicsForceRegion forceRegion;
    NetElementBool enabled;
  };

  struct PhysicsCollisionConfig {
    PhysicsMovingCollision movingCollision;
    NetElementBool enabled;
  };

  static auto sparkBlock(World* world, Vec2I const& position, Vec2F const& damageSource) -> List<Particle>;

  auto getFrame() const -> int;
  void setFrame(int frame);
  auto drawableFrame() -> String;

  void processAction(Json const& action);
  void tickShared(float dt);

  void setup();

  auto makeProjectileCallbacks() -> LuaCallbacks;

  void renderPendingRenderables(RenderCallback* renderCallback);

  Ptr<ProjectileConfig> m_config;
  Json m_parameters;

  // used when projectiles are fired from a moving entity and should include its velocity
  std::optional<Vec2F> m_referenceVelocity;

  // Individual projectile parameters.  Defaults come from m_config, but can be
  // overridden by parameters.
  float m_acceleration;
  float m_initialSpeed;
  float m_power;
  float m_powerMultiplier;
  Directives m_imageDirectives;
  String m_imageSuffix;
  Json m_damageTeam;
  String m_damageKind;
  DamageType m_damageType;
  std::optional<String> m_damageRepeatGroup;
  std::optional<float> m_damageRepeatTimeout;

  bool m_rayCheckToSource;
  bool m_falldown;
  bool m_hydrophobic;
  bool m_onlyHitTerrain;

  std::optional<String> m_collisionSound;
  String m_persistentAudioFile;
  Ptr<AudioInstance> m_persistentAudio;

  List<std::tuple<GameTimer, bool, Json>> m_periodicActions;

  NetElementTopGroup m_netGroup;
  Ptr<MovementController> m_movementController;
  Ptr<EffectEmitter> m_effectEmitter;
  float m_timeToLive;

  Line2F m_travelLine;
  EntityId m_sourceEntity;
  bool m_trackSourceEntity;
  Vec2F m_lastEntityPosition;

  int m_bounces;

  int m_frame;
  float m_animationTimer;
  float m_animationCycle;

  // not quite the same thing as m_collision, used for triggering actionOnCollide
  bool m_wasColliding;
  NetElementEvent m_collisionEvent;

  bool m_collision;
  Vec2I m_collisionTile;
  Vec2I m_lastNonCollidingTile;

  mutable LuaMessageHandlingComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptComponent;

  OrderedHashMap<String, PhysicsForceConfig> m_physicsForces;
  OrderedHashMap<String, PhysicsCollisionConfig> m_physicsCollisions;

  List<Variant<Ptr<AudioInstance>, Particle, LightSource>> m_pendingRenderables;
};

}// namespace Star
