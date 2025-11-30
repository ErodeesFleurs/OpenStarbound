#include "StarEntityAdapter.hpp"

namespace Star {
namespace ECS {

EntityAdapter::EntityAdapter(World* ecsWorld, Entity ecsEntity)
  : m_ecsWorld(ecsWorld), m_ecsEntity(ecsEntity) {
}

EntityAdapter::~EntityAdapter() {
  // Don't destroy the ECS entity here - let the ECS World manage its lifecycle
}

void EntityAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);
  
  // Set up network sync component
  if (auto* netSync = getComponent<NetworkSyncComponent>()) {
    netSync->isMaster = (mode == EntityMode::Master);
  }
}

void EntityAdapter::uninit() {
  Entity::uninit();
}

pair<ByteArray, uint64_t> EntityAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  auto* netSync = getComponent<NetworkSyncComponent>();
  if (!netSync) {
    return {{}, 0};
  }
  
  // Basic implementation - subclasses should override for proper serialization
  netSync->isDirty = false;
  return {{}, netSync->netVersion};
}

void EntityAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  auto* interp = getComponent<InterpolationComponent>();
  if (interp && interp->enabled) {
    interp->interpolationTime = interpolationTime;
  }
}

void EntityAdapter::enableInterpolation(float extrapolationHint) {
  auto* interp = getComponent<InterpolationComponent>();
  if (interp) {
    interp->enabled = true;
    interp->extrapolationHint = extrapolationHint;
  }
}

void EntityAdapter::disableInterpolation() {
  auto* interp = getComponent<InterpolationComponent>();
  if (interp) {
    interp->enabled = false;
  }
}

Vec2F EntityAdapter::position() const {
  auto* transform = getComponent<TransformComponent>();
  return transform ? transform->position : Vec2F();
}

RectF EntityAdapter::metaBoundBox() const {
  auto* bounds = getComponent<BoundsComponent>();
  return bounds ? bounds->metaBoundBox : RectF();
}

RectF EntityAdapter::collisionArea() const {
  auto* bounds = getComponent<BoundsComponent>();
  return bounds ? bounds->collisionArea : RectF();
}

bool EntityAdapter::ephemeral() const {
  auto* entityType = getComponent<EntityTypeComponent>();
  return entityType ? entityType->ephemeral : true;
}

ClientEntityMode EntityAdapter::clientEntityMode() const {
  auto* entityType = getComponent<EntityTypeComponent>();
  return entityType ? entityType->clientMode : ClientEntityMode::ClientSlaveOnly;
}

bool EntityAdapter::masterOnly() const {
  auto* entityType = getComponent<EntityTypeComponent>();
  return entityType ? entityType->masterOnly : false;
}

String EntityAdapter::name() const {
  auto* nameComp = getComponent<NameComponent>();
  return nameComp ? nameComp->name : "";
}

String EntityAdapter::description() const {
  auto* nameComp = getComponent<NameComponent>();
  return nameComp ? nameComp->description : "";
}

List<LightSource> EntityAdapter::lightSources() const {
  auto* lights = getComponent<LightSourceComponent>();
  return lights ? lights->sources : List<LightSource>();
}

List<DamageSource> EntityAdapter::damageSources() const {
  auto* damage = getComponent<DamageSourceComponent>();
  return damage ? damage->damageSources : List<DamageSource>();
}

Maybe<HitType> EntityAdapter::queryHit(DamageSource const& source) const {
  auto* receiver = getComponent<DamageReceiverComponent>();
  if (!receiver || !receiver->hitPoly) {
    return {};
  }
  
  // Basic hit detection - can be expanded
  return HitType::Hit;
}

Maybe<PolyF> EntityAdapter::hitPoly() const {
  auto* receiver = getComponent<DamageReceiverComponent>();
  return receiver ? receiver->hitPoly : Maybe<PolyF>();
}

List<DamageNotification> EntityAdapter::applyDamage(DamageRequest const& damage) {
  auto* health = getComponent<HealthComponent>();
  auto* receiver = getComponent<DamageReceiverComponent>();
  
  if (!health || !receiver) {
    return {};
  }
  
  // Create damage notification
  DamageNotification notification;
  notification.sourceEntityId = damage.sourceEntityId;
  notification.targetEntityId = entityId();
  notification.position = position();
  notification.damageDealt = damage.damage;
  notification.healthLost = std::min(damage.damage, health->currentHealth);
  notification.hitType = HitType::Hit;
  notification.damageSourceKind = damage.damageSourceKind;
  notification.targetMaterialKind = "flesh";
  
  // Apply damage to health
  health->damage(damage.damage);
  
  return {notification};
}

List<DamageNotification> EntityAdapter::selfDamageNotifications() {
  auto* receiver = getComponent<DamageReceiverComponent>();
  if (!receiver) {
    return {};
  }
  return receiver->pullDamage();
}

void EntityAdapter::hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) {
  // Default implementation - can be overridden
}

void EntityAdapter::damagedOther(DamageNotification const& damage) {
  // Default implementation - can be overridden
}

bool EntityAdapter::shouldDestroy() const {
  // Check for death tag or dead health
  if (hasComponent<DeadTag>()) {
    return true;
  }
  
  auto* health = getComponent<HealthComponent>();
  if (health && health->dead) {
    return true;
  }
  
  return false;
}

void EntityAdapter::destroy(RenderCallback* renderCallback) {
  // Default implementation - can be overridden for death effects
}

Maybe<Json> EntityAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  auto* script = getComponent<ScriptComponent>();
  if (script && !script->scripts.empty()) {
    // Script handling would go here
  }
  return {};
}

void EntityAdapter::update(float dt, uint64_t currentStep) {
  // Update is handled by ECS systems
  // Subclasses can override for entity-specific logic
}

void EntityAdapter::render(RenderCallback* renderer) {
  auto* sprite = getComponent<SpriteComponent>();
  auto* transform = getComponent<TransformComponent>();
  
  if (!sprite || !transform || !sprite->visible) {
    return;
  }
  
  // Create drawable from sprite component
  if (!sprite->imagePath.empty()) {
    Drawable drawable = Drawable::makeImage(sprite->imagePath, 1.0f / TilePixels, sprite->centered, transform->position);
    
    if (!sprite->directives.empty()) {
      drawable.imagePart().addDirectives(sprite->directives, true);
    }
    
    if (transform->rotation != 0.0f) {
      drawable.rotate(transform->rotation);
    }
    
    if (transform->scale != Vec2F{1.0f, 1.0f}) {
      drawable.scale(transform->scale);
    }
    
    drawable.fullbright = sprite->fullbright;
    drawable.color = sprite->color;
    
    renderer->addDrawable(std::move(drawable), RenderLayerObject);
  }
}

void EntityAdapter::renderLightSources(RenderCallback* renderer) {
  auto* lights = getComponent<LightSourceComponent>();
  auto* transform = getComponent<TransformComponent>();
  
  if (!lights || !transform) {
    return;
  }
  
  for (auto const& light : lights->sources) {
    LightSource worldLight = light;
    worldLight.position += transform->position;
    renderer->addLightSource(std::move(worldLight));
  }
}

void EntityAdapter::markNetworkDirty() {
  auto* netSync = getComponent<NetworkSyncComponent>();
  if (netSync) {
    netSync->markDirty();
  }
}

} // namespace ECS
} // namespace Star
