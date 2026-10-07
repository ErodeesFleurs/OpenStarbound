#pragma once
#include "StarIdMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarNetElementSystem.hpp"
#include "StarLua.hpp"
#include "StarJson.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"

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
import star.scripted_entity;
import star.status_effect_entity;
import star.movement_controller;
import star.animation;
import star.particle;
#include "StarLuaComponents.hpp"

import star.effect_emitter;

namespace Star {

STAR_CLASS(World);
STAR_CLASS(RenderCallback);
STAR_STRUCT(ProjectileConfig);

STAR_CLASS(Projectile);

class Projectile : public virtual MobileEntity, public virtual ScriptedEntity, public virtual PhysicsEntity, public virtual StatusEffectEntity {
public:
  Projectile(ProjectileConfigPtr const& config, Json const& parameters);
  Projectile(ProjectileConfigPtr const& config, DataStreamBuffer& netState, NetCompatibilityRules rules = {});

  ByteArray netStore(NetCompatibilityRules rules = {}) const;

  EntityType entityType() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  String typeName() const;
  String name() const override;
  String description() const override;

  Vec2F position() const override;
  RectF metaBoundBox() const override;

  Vec2F velocity() const;

  bool ephemeral() const override;
  ClientEntityMode clientEntityMode() const override;
  bool masterOnly() const override;

  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  // If the bullet time to live has run out, or if it has collided, etc this
  // will return true.
  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  List<DamageSource> damageSources() const override;
  void hitOther(EntityId targetEntityId, DamageRequest const& dr) override;

  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;

  String projectileType() const;
  
  Json configValue(String const& name, Json const& def = Json()) const;

  // InitialPosition, InitialDirection, InitialVelocity, PowerMultiplier, and
  // additional status effects must be set before the projectile is added to
  // the world

  float initialSpeed() const;
  void setInitialSpeed(float speed);

  void setInitialPosition(Vec2F const& position);
  void setInitialDirection(Vec2F const& direction);
  // Overrides internal "speed" parameter
  void setInitialVelocity(Vec2F const& velocity);

  void setReferenceVelocity(Maybe<Vec2F> const& velocity);

  float powerMultiplier() const;
  void setPowerMultiplier(float multiplier);

  // If trackSource is true, then the projectile will (while the entity exists)
  // attempt to track the change in position of the parent entity and move
  // relative to it.
  void setSourceEntity(EntityId source, bool trackSource);
  EntityId sourceEntity() const;

  List<PersistentStatusEffect> statusEffects() const override;
  PolyF statusEffectArea() const override;

  List<PhysicsForceRegion> forceRegions() const override;
  size_t movingCollisionCount() const override;
  Maybe<PhysicsMovingCollision> movingCollision(size_t positionIndex) const override;

  using Entity::setTeam;
  
  MovementController* movementController() override;

private:
  struct PhysicsForceConfig {
    PhysicsForceRegion forceRegion;
    NetElementBool enabled;
  };

  struct PhysicsCollisionConfig {
    PhysicsMovingCollision movingCollision;
    NetElementBool enabled;
  };

  static List<Particle> sparkBlock(World* world, Vec2I const& position, Vec2F const& damageSource);

  int getFrame() const;
  void setFrame(int frame);
  String drawableFrame();

  void processAction(Json const& action);
  void tickShared(float dt);

  void setup();

  LuaCallbacks makeProjectileCallbacks();

  void renderPendingRenderables(RenderCallback* renderCallback);

  ProjectileConfigPtr m_config;
  Json m_parameters;

  // used when projectiles are fired from a moving entity and should include its velocity
  Maybe<Vec2F> m_referenceVelocity;

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
  Maybe<String> m_damageRepeatGroup;
  Maybe<float> m_damageRepeatTimeout;

  bool m_rayCheckToSource;
  bool m_falldown;
  bool m_hydrophobic;
  bool m_onlyHitTerrain;

  Maybe<String> m_collisionSound;
  String m_persistentAudioFile;
  AudioInstancePtr m_persistentAudio;

  List<tuple<GameTimer, bool, Json>> m_periodicActions;

  NetElementTopGroup m_netGroup;
  MovementControllerPtr m_movementController;
  EffectEmitterPtr m_effectEmitter;
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

  List<Variant<AudioInstancePtr, Particle, LightSource>> m_pendingRenderables;
};

}
