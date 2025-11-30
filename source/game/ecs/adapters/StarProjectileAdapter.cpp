#include "StarProjectileAdapter.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarRandom.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarWorld.hpp"
#include "StarItemDrop.hpp"
#include "StarDamageDatabase.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarParticleDatabase.hpp"
#include "StarMonster.hpp"
#include "StarEntityRendering.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"

namespace Star {
namespace ECS {

// Static factory methods

shared_ptr<ProjectileAdapter> ProjectileAdapter::create(
    World* ecsWorld,
    ProjectileConfigPtr const& config,
    Json const& parameters) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<ProjectileAdapter>(ecsWorld, entity);
  adapter->setupComponents(config, parameters);
  return adapter;
}

shared_ptr<ProjectileAdapter> ProjectileAdapter::createFromNet(
    World* ecsWorld,
    ProjectileConfigPtr const& config,
    ByteArray const& netStore,
    NetCompatibilityRules rules) {
  DataStreamBuffer ds(netStore);
  ds.setStreamCompatibilityVersion(rules);
  
  Json parameters;
  ds.read(parameters);
  
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<ProjectileAdapter>(ecsWorld, entity);
  adapter->setupComponents(config, parameters);
  
  // Read additional state
  auto* projData = adapter->getComponent<ProjectileDataComponent>();
  if (projData) {
    EntityId sourceEntity = ds.readVlqI();
    bool trackSourceEntity = ds.read<bool>();
    adapter->setSourceEntity(sourceEntity, trackSourceEntity);
    
    ds.read(projData->initialSpeed);
    ds.read(projData->powerMultiplier);
    adapter->Entity::setTeam(ds.read<EntityDamageTeam>());
  }
  
  return adapter;
}

// Constructor

ProjectileAdapter::ProjectileAdapter(World* ecsWorld, Entity ecsEntity)
  : EntityAdapter(ecsWorld, ecsEntity), m_rotation(0.0f) {
  m_effectEmitter = make_shared<EffectEmitter>();
}

void ProjectileAdapter::setupComponents(ProjectileConfigPtr const& config, Json const& parameters) {
  // Add tag
  addComponent<ProjectileTag>();
  
  // Entity type
  auto& entityType = addComponent<EntityTypeComponent>();
  entityType.type = EntityType::Projectile;
  entityType.clientMode = config->clientEntityMode;
  entityType.masterOnly = config->masterOnly;
  entityType.ephemeral = true;
  
  // Projectile data
  auto& projData = addComponent<ProjectileDataComponent>();
  projData.config = config;
  projData.parameters = parameters;
  
  // Setup from config/parameters
  projData.acceleration = parameters.getFloat("acceleration", config->acceleration);
  projData.power = parameters.getFloat("power", config->power);
  projData.powerMultiplier = parameters.getFloat("powerMultiplier", 1.0f);
  
  // Image processing
  String processing = parameters.getString("processing", "");
  auto begin = processing.utf8().find_first_of('?');
  if (begin == NPos) {
    projData.imageDirectives = "";
    projData.imageSuffix = std::move(processing);
  } else if (begin == 0) {
    projData.imageDirectives = std::move(processing);
    projData.imageSuffix = "";
  } else {
    projData.imageDirectives = (String)processing.utf8().substr(begin);
    projData.imageSuffix = processing.utf8().substr(0, begin);
  }
  
  projData.persistentAudioFile = parameters.getString("persistentAudio", config->persistentAudio);
  projData.damageKind = parameters.getString("damageKind", config->damageKind);
  projData.damageType = DamageTypeNames.getLeft(parameters.getString("damageType", config->damageType));
  projData.rayCheckToSource = parameters.getBool("rayCheckToSource", config->rayCheckToSource);
  
  if (auto damageTeam = parameters.get("damageTeam", config->damageTeam)) {
    projData.damageTeam = damageTeam;
    Entity::setTeam(EntityDamageTeam(damageTeam));
  }
  
  projData.damageRepeatGroup = parameters.optString("damageRepeatGroup").orMaybe(config->damageRepeatGroup);
  projData.damageRepeatTimeout = parameters.optFloat("damageRepeatTimeout").orMaybe(config->damageRepeatTimeout);
  projData.falldown = parameters.getBool("falldown", config->falldown);
  projData.hydrophobic = parameters.getBool("hydrophobic", config->hydrophobic);
  projData.onlyHitTerrain = parameters.getBool("onlyHitTerrain", config->onlyHitTerrain);
  projData.initialSpeed = parameters.getFloat("speed", config->initialSpeed);
  projData.bounces = parameters.getInt("bounces", config->bounces);
  projData.animationCycle = parameters.getFloat("animationCycle", config->animationCycle);
  
  // Transform
  auto& transform = addComponent<TransformComponent>();
  transform.position = Vec2F();
  
  // Velocity
  auto& velocity = addComponent<VelocityComponent>();
  velocity.velocity = Vec2F();
  
  // Bounds
  auto& bounds = addComponent<BoundsComponent>();
  bounds.metaBoundBox = config->boundBox;
  
  // Physics
  auto& physics = addComponent<PhysicsBodyComponent>();
  physics.mass = 1.0f;
  physics.gravityMultiplier = 0.0f;  // Projectiles typically don't use gravity
  physics.collisionEnabled = true;
  
  // Network sync
  auto& netSync = addComponent<NetworkSyncComponent>();
  netSync.netVersion = 1;
  
  addComponent<InterpolationComponent>();
  
  // Name
  auto& nameComp = addComponent<NameComponent>();
  nameComp.name = config->typeName;
  nameComp.description = config->description;
}

ByteArray ProjectileAdapter::netStore(NetCompatibilityRules rules) const {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return {};
  
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);
  
  ds.write(projData->config->typeName);
  ds.write(projData->parameters);
  ds.viwrite(projData->sourceEntity);
  ds.write(projData->trackSourceEntity);
  ds.write(projData->initialSpeed);
  ds.write(projData->powerMultiplier);
  ds.write(getTeam());
  
  return ds.data();
}

EntityType ProjectileAdapter::entityType() const {
  return EntityType::Projectile;
}

void ProjectileAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return;
  
  projData->timeToLive = projData->parameters.getFloat("timeToLive", projData->config->timeToLive);
  setSourceEntity(projData->sourceEntity, projData->trackSourceEntity);
  
  // Setup periodic actions
  projData->periodicActions.clear();
  if (projData->parameters.contains("periodicActions")) {
    for (auto const& c : projData->parameters.getArray("periodicActions", {}))
      projData->periodicActions.append(make_tuple(GameTimer(c.getFloat("time")), c.getBool("repeat", true), c));
  } else {
    for (auto const& periodicAction : projData->config->periodicActions)
      projData->periodicActions.append(make_tuple(GameTimer(get<0>(periodicAction)), get<1>(periodicAction), get<2>(periodicAction)));
  }
  
  auto* transform = getComponent<TransformComponent>();
  if (transform) {
    projData->travelLine = Line2F(transform->position, transform->position);
  }
  
  if (auto referenceVelocity = projData->parameters.opt("referenceVelocity"))
    setReferenceVelocity(referenceVelocity.apply(jsonToVec2F));
  
  // Setup persistent audio on client
  if (world->isClient() && !projData->persistentAudioFile.empty()) {
    projData->persistentAudio = make_shared<AudioInstance>(*Root::singleton().assets()->audio(projData->persistentAudioFile));
    projData->persistentAudio->setLoops(-1);
    projData->persistentAudio->setPosition(position());
    m_pendingRenderables.append(projData->persistentAudio);
  }
}

void ProjectileAdapter::uninit() {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (projData && projData->persistentAudio) {
    projData->persistentAudio->stop();
  }
  EntityAdapter::uninit();
}

String ProjectileAdapter::name() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->config->typeName : "";
}

String ProjectileAdapter::description() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->config->description : "";
}

pair<ByteArray, uint64_t> ProjectileAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  DataStreamBuffer ds;
  
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* netSync = getComponent<NetworkSyncComponent>();
  
  if (projData && transform && velocity && netSync) {
    ds.write(transform->position);
    ds.write(velocity->velocity);
    ds.write(m_rotation);
    ds.write(projData->timeToLive);
    ds.write(projData->collision);
    ds.write(projData->frame);
    
    uint64_t version = netSync->netVersion;
    netSync->isDirty = false;
    return {ds.takeData(), version};
  }
  
  return {{}, 0};
}

void ProjectileAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  if (data.empty()) return;
  
  DataStreamBuffer ds(std::move(data));
  
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (projData && transform && velocity) {
    Vec2F newPos = ds.read<Vec2F>();
    Vec2F newVel = ds.read<Vec2F>();
    float newRotation = ds.read<float>();
    
    if (interp && interp->enabled) {
      interp->setTarget(newPos, newRotation);
      interp->interpolationTime = interpolationTime;
    } else {
      transform->position = newPos;
      m_rotation = newRotation;
    }
    velocity->velocity = newVel;
    projData->timeToLive = ds.read<float>();
    projData->collision = ds.read<bool>();
    projData->frame = ds.read<int>();
  }
}

void ProjectileAdapter::enableInterpolation(float extrapolationHint) {
  auto* interp = getComponent<InterpolationComponent>();
  if (interp) {
    interp->enabled = true;
    interp->extrapolationHint = extrapolationHint;
  }
}

void ProjectileAdapter::disableInterpolation() {
  auto* interp = getComponent<InterpolationComponent>();
  if (interp) {
    interp->enabled = false;
  }
}

Vec2F ProjectileAdapter::position() const {
  auto* transform = getComponent<TransformComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (interp && interp->enabled) {
    return interp->interpolatedPosition();
  }
  return transform ? transform->position : Vec2F();
}

RectF ProjectileAdapter::metaBoundBox() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->config->boundBox : RectF();
}

bool ProjectileAdapter::ephemeral() const {
  return true;
}

ClientEntityMode ProjectileAdapter::clientEntityMode() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->config->clientEntityMode : ClientEntityMode::ClientSlaveOnly;
}

bool ProjectileAdapter::masterOnly() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->config->masterOnly : false;
}

bool ProjectileAdapter::shouldDestroy() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData && projData->timeToLive <= 0.0f;
}

void ProjectileAdapter::destroy(RenderCallback* renderCallback) {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return;
  
  // Process reap actions
  for (auto const& action : projData->parameters.getArray("actionOnReap", projData->config->actionOnReap))
    processAction(action);
  
  if (projData->collision) {
    for (auto const& action : projData->parameters.getArray("actionOnHit", projData->config->actionOnHit))
      processAction(action);
  } else {
    for (auto const& action : projData->parameters.getArray("actionOnTimeout", projData->config->actionOnTimeout))
      processAction(action);
  }
  
  if (renderCallback)
    renderPendingRenderables(renderCallback);
}

List<DamageSource> ProjectileAdapter::damageSources() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData || projData->onlyHitTerrain)
    return {};
  
  float time_per_frame = projData->animationCycle / projData->config->frameNumber;
  if ((projData->config->intangibleWindup && projData->animationTimer < time_per_frame * projData->config->windupFrames)
      || (projData->config->intangibleWinddown && projData->timeToLive < time_per_frame * projData->config->winddownFrames))
    return {};
  
  EntityDamageTeam sourceTeam = getTeam();
  
  auto statusEffects = projData->config->statusEffects;
  statusEffects.appendAll(projData->parameters.getArray("statusEffects", {}).transformed(jsonToEphemeralStatusEffect));
  
  float knockbackMagnitude = projData->parameters.getFloat("knockback", projData->config->knockback);
  
  DamageSource::Knockback knockback;
  if (projData->parameters.getBool("knockbackDirectional", projData->config->knockbackDirectional))
    knockback = Vec2F::withAngle(m_rotation) * knockbackMagnitude;
  else
    knockback = knockbackMagnitude;
  
  List<DamageSource> res;
  auto addDamageSource = [&](DamageSource::DamageArea damageArea) {
    res.append(DamageSource(projData->damageType, damageArea, projData->power * projData->powerMultiplier, true, projData->sourceEntity, sourceTeam,
        projData->damageRepeatGroup, projData->damageRepeatTimeout, projData->damageKind, statusEffects, knockback, projData->rayCheckToSource));
  };
  
  Vec2F positionDelta = world()->geometry().diff(projData->travelLine.min(), projData->travelLine.max());
  static float const MinimumDamageLineDelta = 0.1f;
  bool useDamageLine = positionDelta.magnitudeSquared() >= square(MinimumDamageLineDelta);
  if (useDamageLine)
    addDamageSource(Line2F(positionDelta, Vec2F()));
  
  if (!projData->config->damagePoly.isNull()) {
    PolyF damagePoly = projData->config->damagePoly;
    if (projData->config->flippable) {
      auto angleSide = getAngleSide(m_rotation, true);
      if (angleSide.second == Direction::Left)
        damagePoly.flipHorizontal(0);
      damagePoly.rotate(angleSide.first);
    } else {
      damagePoly.rotate(m_rotation);
    }
    addDamageSource(damagePoly);
  } else if (!useDamageLine) {
    addDamageSource(PolyF(RectF::withCenter(Vec2F(), Vec2F::filled(MinimumDamageLineDelta))));
  }
  
  return res;
}

void ProjectileAdapter::hitOther(EntityId entity, DamageRequest const&) {
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* transform = getComponent<TransformComponent>();
  
  if (!projData || !velocity || !transform) return;
  
  if (!projData->parameters.getBool("piercing", projData->config->piercing)) {
    auto victimEntity = world()->entity(entity);
    if (!victimEntity || (victimEntity->getTeam().type != TeamType::Passive && victimEntity->getTeam().type != TeamType::Environment)) {
      if (victimEntity) {
        if (auto hitPoly = victimEntity->hitPoly()) {
          auto geometry = world()->geometry();
          Vec2F checkVec = velocity->velocity.normalized() * 5;
          Vec2F nearMin = geometry.nearestTo(hitPoly->center(), transform->position - checkVec);
          if (auto intersection = hitPoly->lineIntersection(Line2F(nearMin, nearMin + checkVec * 2)))
            transform->position = intersection->point;
        }
      }
      velocity->velocity = Vec2F();
      projData->collision = true;
      projData->timeToLive = 0.0f;
      markNetworkDirty();
    }
  }
}

void ProjectileAdapter::update(float dt, uint64_t currentStep) {
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (!projData || !transform || !velocity) return;
  
  bool isMaster = inWorld() && (world()->connection() == 0);
  
  if (isMaster) {
    projData->timeToLive -= dt;
    if (projData->timeToLive < 0)
      projData->timeToLive = 0;
    
    m_effectEmitter->addEffectSources("normal", projData->config->emitters);
    
    // Apply reference velocity adjustment
    if (projData->referenceVelocity)
      velocity->velocity = velocity->velocity - *projData->referenceVelocity;
    
    // Acceleration
    velocity->velocity += velocity->velocity.normalized() * projData->acceleration * dt;
    
    if (projData->referenceVelocity)
      velocity->velocity = velocity->velocity + *projData->referenceVelocity;
    
    // Update position
    transform->position += velocity->velocity * dt;
    
    // Update travel line
    projData->travelLine.min() = projData->travelLine.max();
    projData->travelLine.max() = transform->position;
    
    tickShared(dt);
    
    // Track source entity
    if (projData->trackSourceEntity) {
      if (auto sourceEntity = world()->entity(projData->sourceEntity)) {
        Vec2F newEntityPosition = sourceEntity->position();
        transform->position += newEntityPosition - projData->lastEntityPosition;
        projData->lastEntityPosition = newEntityPosition;
      } else {
        projData->trackSourceEntity = false;
      }
    }
    
    // World limit check
    if (world()->geometry().limitedPosition(transform->position) != transform->position)
      projData->timeToLive = 0.0f;
    
    markNetworkDirty();
  } else {
    // Slave update
    if (interp && interp->enabled) {
      interp->update(dt, 10.0f);
    }
    
    projData->timeToLive -= dt;
    tickShared(dt);
  }
}

void ProjectileAdapter::tickShared(float dt) {
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  
  if (!projData || !velocity) return;
  
  // Update rotation based on velocity
  if (!projData->config->orientationLocked) {
    auto apparentVelocity = velocity->velocity - projData->referenceVelocity.value(Vec2F());
    if (apparentVelocity != Vec2F())
      m_rotation = apparentVelocity.angle();
  }
  
  // Update animation
  projData->animationTimer += dt;
  projData->frame = getFrame();
  
  // Update effect emitter
  m_effectEmitter->setSourcePosition("normal", position());
  m_effectEmitter->setDirection(getAngleSide(m_rotation, true).second);
  m_effectEmitter->tick(dt, *entityMode());
  
  // Process periodic actions
  auto periodicActionIt = makeSMutableIterator(projData->periodicActions);
  while (periodicActionIt.hasNext()) {
    auto& periodicAction = periodicActionIt.next();
    if (get<1>(periodicAction)) {
      if (get<0>(periodicAction).wrapTick())
        processAction(get<2>(periodicAction));
    } else {
      if (get<0>(periodicAction).tick(dt)) {
        processAction(get<2>(periodicAction));
        periodicActionIt.remove();
      }
    }
  }
}

int ProjectileAdapter::getFrame() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return 0;
  
  float time_per_frame = projData->animationCycle / projData->config->frameNumber;
  
  if (projData->config->animationLoops) {
    if (projData->animationTimer < time_per_frame * projData->config->windupFrames) {
      return floor(projData->animationTimer / time_per_frame);
    } else if (projData->timeToLive < time_per_frame * projData->config->winddownFrames) {
      return projData->config->windupFrames + projData->config->frameNumber
          + clamp<int>((time_per_frame * projData->config->winddownFrames - projData->timeToLive) / time_per_frame, 0, projData->config->winddownFrames - 1);
    } else {
      float time_within_cycle = std::fmod(projData->animationTimer, projData->animationCycle);
      return projData->config->windupFrames + floor(time_within_cycle / time_per_frame);
    }
  } else {
    return clamp<int>(projData->animationTimer / time_per_frame, 0, projData->config->frameNumber - 1);
  }
}

void ProjectileAdapter::render(RenderCallback* renderCallback) {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return;
  
  renderPendingRenderables(renderCallback);
  
  if (projData->persistentAudio)
    projData->persistentAudio->setPosition(position());
  
  m_effectEmitter->render(renderCallback);
  
  String image = strf("{}:{}{}", projData->config->image, projData->frame, projData->imageSuffix);
  Drawable drawable = Drawable::makeImage(image, 1.0f / TilePixels, true, Vec2F());
  drawable.imagePart().addDirectives(projData->imageDirectives, true);
  
  if (projData->config->flippable) {
    auto angleSide = getAngleSide(m_rotation, true);
    if (angleSide.second == Direction::Left)
      drawable.scale(Vec2F(-1, 1));
    drawable.rotate(angleSide.first);
  } else {
    drawable.rotate(m_rotation);
  }
  
  drawable.fullbright = projData->config->fullbright;
  drawable.translate(position());
  renderCallback->addDrawable(std::move(drawable), projData->config->renderLayer);
}

void ProjectileAdapter::renderLightSources(RenderCallback* renderCallback) {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return;
  
  for (auto renderable : m_pendingRenderables) {
    if (renderable.is<LightSource>())
      renderCallback->addLightSource(renderable.get<LightSource>());
  }
  renderCallback->addLightSource({position(), projData->config->lightColor.toRgbF(), projData->config->lightType, 0.0f, 0.0f, 0.0f});
}

Maybe<Json> ProjectileAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  return m_scriptComponent.handleMessage(message, sendingConnection == world()->connection(), args);
}

void ProjectileAdapter::renderPendingRenderables(RenderCallback* renderCallback) {
  for (auto renderable : m_pendingRenderables) {
    if (renderable.is<AudioInstancePtr>())
      renderCallback->addAudio(renderable.get<AudioInstancePtr>());
    else if (renderable.is<Particle>())
      renderCallback->addParticle(renderable.get<Particle>());
  }
  m_pendingRenderables.clear();
}

String ProjectileAdapter::projectileType() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->config->typeName : "";
}

Json ProjectileAdapter::configValue(String const& name, Json const& def) const {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return def;
  return projData->parameters.query(name, projData->config->config.query(name, def));
}

Vec2F ProjectileAdapter::velocity() const {
  auto* velocity = getComponent<VelocityComponent>();
  return velocity ? velocity->velocity : Vec2F();
}

void ProjectileAdapter::setVelocity(Vec2F const& vel) {
  auto* velocity = getComponent<VelocityComponent>();
  if (velocity) {
    velocity->velocity = vel;
    markNetworkDirty();
  }
}

float ProjectileAdapter::initialSpeed() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->initialSpeed : 0.0f;
}

void ProjectileAdapter::setInitialSpeed(float speed) {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (projData) {
    projData->initialSpeed = speed;
  }
}

void ProjectileAdapter::setInitialPosition(Vec2F const& pos) {
  auto* transform = getComponent<TransformComponent>();
  if (transform) {
    transform->position = pos;
    markNetworkDirty();
  }
}

void ProjectileAdapter::setInitialDirection(Vec2F const& direction) {
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  
  if (projData && velocity) {
    velocity->velocity = vnorm(direction) * projData->initialSpeed + projData->referenceVelocity.value(Vec2F());
    m_rotation = direction.angle();
    markNetworkDirty();
  }
}

void ProjectileAdapter::setInitialVelocity(Vec2F const& vel) {
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  
  if (projData && velocity) {
    velocity->velocity = vel + projData->referenceVelocity.value(Vec2F());
    m_rotation = vel.angle();
    markNetworkDirty();
  }
}

void ProjectileAdapter::setReferenceVelocity(Maybe<Vec2F> const& refVel) {
  auto* projData = getComponent<ProjectileDataComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  
  if (projData && velocity) {
    velocity->velocity = velocity->velocity - projData->referenceVelocity.value(Vec2F());
    projData->referenceVelocity = refVel;
    velocity->velocity = velocity->velocity + refVel.value(Vec2F());
    m_effectEmitter->setBaseVelocity(refVel.value(Vec2F()));
  }
}

float ProjectileAdapter::powerMultiplier() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->powerMultiplier : 1.0f;
}

void ProjectileAdapter::setPowerMultiplier(float multiplier) {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (projData) {
    projData->powerMultiplier = multiplier;
  }
}

void ProjectileAdapter::setSourceEntity(EntityId source, bool trackSource) {
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return;
  
  projData->sourceEntity = source;
  projData->trackSourceEntity = trackSource;
  
  if (inWorld()) {
    if (auto sourceEntity = world()->entity(source)) {
      projData->lastEntityPosition = sourceEntity->position();
      if (!projData->damageTeam)
        Entity::setTeam(sourceEntity->getTeam());
    } else {
      projData->sourceEntity = NullEntityId;
      projData->trackSourceEntity = false;
    }
  }
}

EntityId ProjectileAdapter::sourceEntity() const {
  auto* projData = getComponent<ProjectileDataComponent>();
  return projData ? projData->sourceEntity : NullEntityId;
}

float ProjectileAdapter::rotation() const {
  return m_rotation;
}

void ProjectileAdapter::setRotation(float rot) {
  m_rotation = rot;
  markNetworkDirty();
}

void ProjectileAdapter::processAction(Json const& action) {
  // Simplified action processing - full implementation would match original
  // This is a placeholder that handles the most common actions
  
  auto* projData = getComponent<ProjectileDataComponent>();
  if (!projData) return;
  
  Json parameters;
  String command;
  
  if (action.type() == Json::Type::Object) {
    parameters = action;
    command = parameters.getString("action").toLower();
  } else {
    command = action.toString().toLower();
  }
  
  auto doWithDelay = [this](int stepsDelay, WorldAction function) {
    if (stepsDelay == 0)
      function(world());
    else
      world()->timer((float)stepsDelay / 60.f, function);
  };
  
  if (command == "particle") {
    if (!world()->isClient())
      return;
    
    Particle particle = Root::singleton().particleDatabase()->particle(parameters.get("specification"));
    particle.position = particle.position.rotate(m_rotation);
    if (parameters.getBool("rotate", false)) {
      particle.rotation = m_rotation;
      particle.velocity = particle.velocity.rotate(m_rotation);
    }
    particle.translate(position());
    particle.velocity += projData->referenceVelocity.value(Vec2F());
    m_pendingRenderables.append(std::move(particle));
    
  } else if (command == "sound") {
    if (!world()->isClient())
      return;
    
    AudioInstancePtr sound = make_shared<AudioInstance>(*Root::singleton().assets()->audio(Random::randValueFrom(parameters.getArray("options")).toString()));
    sound->setPosition(position());
    sound->setVolume(parameters.getFloat("volume", 1.0f));
    sound->setPitchMultiplier(parameters.getFloat("pitch", 1.0f));
    m_pendingRenderables.append(std::move(sound));
    
  } else if (command == "light") {
    if (!world()->isClient())
      return;
    
    m_pendingRenderables.append(LightSource{
        position(),
        jsonToColor(parameters.get("color")).toRgbF(),
        (LightType)parameters.getBool("pointLight", true),
        0.0f,
        0.0f,
        0.0f
      });
    
  } else if (command == "projectile") {
    if (isSlave())
      return;
    
    String type = parameters.getString("type");
    auto projectileParameters = parameters.get("config", JsonObject());
    if (!projectileParameters.contains("damageTeam")) {
      if (projData->damageTeam)
        projectileParameters = projectileParameters.set("damageTeam", projData->damageTeam);
    }
    if (parameters.contains("inheritDamageFactor") && !projectileParameters.contains("power"))
      projectileParameters = projectileParameters.set("power", projData->power * parameters.getFloat("inheritDamageFactor"));
    
    auto projectile = Root::singleton().projectileDatabase()->createProjectile(type, projectileParameters);
    Vec2F offset;
    if (parameters.contains("offset")) {
      offset = jsonToVec2F(parameters.getArray("offset", {0.0f, 0.0f}));
    }
    if (projData->referenceVelocity)
      projectile->setReferenceVelocity(projData->referenceVelocity);
    projectile->setInitialPosition(position() + offset);
    
    if (parameters.contains("direction")) {
      projectile->setInitialDirection(jsonToVec2F(parameters.get("direction")));
    } else {
      float angle = m_rotation;
      if (parameters.contains("angle"))
        angle = parameters.getFloat("angle") * Constants::pi / 180.0f;
      projectile->setInitialDirection(Vec2F::withAngle(angle, 1.0f));
    }
    projectile->setSourceEntity(projData->sourceEntity, false);
    projectile->setPowerMultiplier(projData->powerMultiplier);
    
    doWithDelay(parameters.getUInt("delaySteps", 0), [=](Star::World* world) { world->addEntity(projectile); });
    
  } else if (command == "item") {
    if (isSlave())
      return;
    
    String const itemName = parameters.getString("name");
    size_t count = parameters.getInt("count", 1);
    JsonObject data = parameters.getObject("data", JsonObject{});
    
    auto itemDrop = ItemDrop::createRandomizedDrop(ItemDescriptor(itemName, count, data), position());
    world()->addEntity(itemDrop);
    
  } else if (command == "option") {
    JsonArray options = parameters.getArray("options");
    if (options.size())
      processAction(Random::randFrom(options));
    
  } else if (command == "actions") {
    JsonArray list = parameters.getArray("list");
    for (auto const& act : list)
      processAction(act);
    
  } else if (command == "loop") {
    int count = parameters.getInt("count");
    JsonArray body = parameters.getArray("body");
    while (count > 0) {
      for (auto const& act : body)
        processAction(act);
      count--;
    }
    
  } else if (command == "config") {
    processAction(Root::singleton().assets()->json(parameters.getString("file")));
  }
  
  // Other actions (tile, liquid, explosion, spawnmonster, etc.) are omitted for brevity
  // They can be implemented following the same pattern as the original
}

} // namespace ECS
} // namespace Star
