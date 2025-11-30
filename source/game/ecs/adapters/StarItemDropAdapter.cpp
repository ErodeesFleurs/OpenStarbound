#include "StarItemDropAdapter.hpp"
#include "StarItem.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarRandom.hpp"
#include "StarDataStreamExtra.hpp"

namespace Star {
namespace ECS {

// Static factory methods

shared_ptr<ItemDropAdapter> ItemDropAdapter::createRandomizedDrop(
    World* ecsWorld, ItemPtr const& item, Vec2F const& position, bool eternal) {
  return createRandomizedDrop(ecsWorld, item->descriptor(), position, eternal);
}

shared_ptr<ItemDropAdapter> ItemDropAdapter::createRandomizedDrop(
    World* ecsWorld, ItemDescriptor const& itemDescriptor, Vec2F const& position, bool eternal) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<ItemDropAdapter>(ecsWorld, entity, itemDescriptor);
  
  // Randomize position slightly
  Vec2F offset = Vec2F(Random::randf(-0.5f, 0.5f), Random::randf(0.0f, 0.5f));
  adapter->setPosition(position + offset);
  
  // Random initial velocity
  Vec2F velocity = Vec2F(Random::randf(-5.0f, 5.0f), Random::randf(5.0f, 15.0f));
  adapter->setVelocity(velocity);
  
  adapter->setEternal(eternal);
  adapter->setIntangibleTime(0.5f);
  
  return adapter;
}

shared_ptr<ItemDropAdapter> ItemDropAdapter::throwDrop(
    World* ecsWorld, ItemPtr const& item, Vec2F const& position,
    Vec2F const& velocity, Vec2F const& direction, bool eternal) {
  return throwDrop(ecsWorld, item->descriptor(), position, velocity, direction, eternal);
}

shared_ptr<ItemDropAdapter> ItemDropAdapter::throwDrop(
    World* ecsWorld, ItemDescriptor const& itemDescriptor, Vec2F const& position,
    Vec2F const& velocity, Vec2F const& direction, bool eternal) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<ItemDropAdapter>(ecsWorld, entity, itemDescriptor);
  
  adapter->setPosition(position);
  
  // Throw velocity
  Vec2F throwDir = vnorm(direction);
  Vec2F throwVelocity = velocity + throwDir * 15.0f;
  adapter->setVelocity(throwVelocity);
  
  adapter->setEternal(eternal);
  adapter->setIntangibleTime(1.0f);
  
  return adapter;
}

// Constructor implementations

ItemDropAdapter::ItemDropAdapter(World* ecsWorld, Entity ecsEntity)
  : EntityAdapter(ecsWorld, ecsEntity) {
  // Load config
  m_config = Root::singleton().assets()->json("/itemdrop.config");
  m_defaultBoundBox = jsonToRectF(m_config.get("boundBox"));
  m_afterTakenLife = m_config.getFloat("afterTakenLife", 1.0f);
  m_overheadTime = m_config.getFloat("overheadTime", 1.0f);
  m_velocityApproach = m_config.getFloat("velocityApproach", 50.0f);
  m_overheadApproach = m_config.getFloat("overheadApproach", 20.0f);
  m_overheadOffset = jsonToVec2F(m_config.get("overheadOffset", JsonArray{0.0f, 2.0f}));
  m_ageItemsEvery = m_config.getDouble("ageItemsEvery", 10.0);
}

ItemDropAdapter::ItemDropAdapter(World* ecsWorld, Entity ecsEntity, ItemPtr item)
  : ItemDropAdapter(ecsWorld, ecsEntity) {
  setupComponents(item->descriptor(), false);
  m_cachedItem = item;
}

ItemDropAdapter::ItemDropAdapter(World* ecsWorld, Entity ecsEntity, ItemDescriptor const& itemDescriptor)
  : ItemDropAdapter(ecsWorld, ecsEntity) {
  setupComponents(itemDescriptor, false);
}

void ItemDropAdapter::setupComponents(ItemDescriptor const& itemDescriptor, bool eternal) {
  // Add entity type tag
  addComponent<ItemDropTag>();
  
  // Entity type component
  auto& entityType = addComponent<EntityTypeComponent>();
  entityType.type = EntityType::ItemDrop;
  entityType.clientMode = ClientEntityMode::ClientSlaveOnly;
  entityType.ephemeral = true;
  
  // Transform component
  auto& transform = addComponent<TransformComponent>();
  transform.position = Vec2F();
  
  // Velocity component
  auto& velocity = addComponent<VelocityComponent>();
  velocity.velocity = Vec2F();
  
  // Bounds component
  auto& bounds = addComponent<BoundsComponent>();
  bounds.metaBoundBox = m_defaultBoundBox;
  bounds.collisionArea = m_defaultBoundBox;
  
  // Physics body
  auto& physics = addComponent<PhysicsBodyComponent>();
  physics.mass = 1.0f;
  physics.gravityMultiplier = 1.0f;
  physics.collisionEnabled = true;
  physics.gravityEnabled = true;
  physics.groundFriction = 20.0f;
  physics.airFriction = 0.0f;
  
  // ItemDrop data
  auto& dropData = addComponent<ItemDropDataComponent>();
  dropData.itemDescriptor = itemDescriptor;
  dropData.eternal = eternal;
  dropData.mode = ItemDropDataComponent::Mode::Intangible;
  
  // Network sync - start at version 1 (0 is invalid/uninitialized)
  auto& netSync = addComponent<NetworkSyncComponent>();
  netSync.netVersion = 1;
  
  // Interpolation
  addComponent<InterpolationComponent>();
  
  // Name from item
  auto& nameComp = addComponent<NameComponent>();
  if (auto item = this->item()) {
    nameComp.name = item->friendlyName();
    nameComp.description = item->description();
  }
}

Json ItemDropAdapter::diskStore() const {
  auto* dropData = getComponent<ItemDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  
  if (!dropData || !transform) {
    return {};
  }
  
  JsonObject result;
  result["item"] = dropData->itemDescriptor.toJson();
  result["position"] = jsonFromVec2F(transform->position);
  result["velocity"] = velocity ? jsonFromVec2F(velocity->velocity) : JsonArray{0.0f, 0.0f};
  result["eternal"] = dropData->eternal;
  result["intangibleTime"] = dropData->intangibleTime;
  result["dropAge"] = dropData->dropAge;
  
  return result;
}

ByteArray ItemDropAdapter::netStore(NetCompatibilityRules rules) const {
  DataStreamBuffer ds;
  
  auto* dropData = getComponent<ItemDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  
  if (dropData && transform) {
    ds.write(dropData->itemDescriptor);
    ds.write(transform->position);
  }
  
  return ds.takeData();
}

EntityType ItemDropAdapter::entityType() const {
  return EntityType::ItemDrop;
}

void ItemDropAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  updateCollisionPoly();
}

void ItemDropAdapter::uninit() {
  EntityAdapter::uninit();
}

pair<ByteArray, uint64_t> ItemDropAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  // Serialize current state for networking
  DataStreamBuffer ds;
  
  auto* dropData = getComponent<ItemDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* netSync = getComponent<NetworkSyncComponent>();
  
  if (dropData && transform && velocity && netSync) {
    ds.write(static_cast<uint8_t>(dropData->mode));
    ds.write(dropData->owningEntity);
    ds.write(dropData->itemDescriptor);
    ds.write(transform->position);
    ds.write(velocity->velocity);
    
    uint64_t version = netSync->netVersion;
    netSync->isDirty = false;
    return {ds.takeData(), version};
  }
  
  return {{}, 0};
}

void ItemDropAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  if (data.empty()) return;
  
  DataStreamBuffer ds(std::move(data));
  
  auto* dropData = getComponent<ItemDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (dropData && transform && velocity) {
    dropData->mode = static_cast<ItemDropDataComponent::Mode>(ds.read<uint8_t>());
    dropData->owningEntity = ds.read<EntityId>();
    dropData->itemDescriptor = ds.read<ItemDescriptor>();
    
    Vec2F newPos = ds.read<Vec2F>();
    Vec2F newVel = ds.read<Vec2F>();
    
    if (interp && interp->enabled) {
      // ItemDrops don't have rotation, pass 0.0f
      interp->setTarget(newPos, 0.0f);
      interp->interpolationTime = interpolationTime;
    } else {
      transform->position = newPos;
    }
    velocity->velocity = newVel;
    
    // Clear cached item when descriptor changes
    m_cachedItem = nullptr;
  }
}

Vec2F ItemDropAdapter::position() const {
  auto* transform = getComponent<TransformComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (interp && interp->enabled) {
    return interp->interpolatedPosition();
  }
  return transform ? transform->position : Vec2F();
}

RectF ItemDropAdapter::metaBoundBox() const {
  auto* bounds = getComponent<BoundsComponent>();
  return bounds ? bounds->metaBoundBox : m_defaultBoundBox;
}

RectF ItemDropAdapter::collisionArea() const {
  auto* bounds = getComponent<BoundsComponent>();
  return bounds ? bounds->collisionArea : RectF();
}

bool ItemDropAdapter::ephemeral() const {
  return true;
}

bool ItemDropAdapter::shouldDestroy() const {
  auto* dropData = getComponent<ItemDropDataComponent>();
  return dropData && dropData->mode == ItemDropDataComponent::Mode::Dead;
}

void ItemDropAdapter::update(float dt, uint64_t currentStep) {
  auto* dropData = getComponent<ItemDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (!dropData || !transform || !velocity) return;
  
  bool isMaster = inWorld() && (world()->connection() == 0);
  
  // Update interpolation with default rate
  if (interp && interp->enabled) {
    interp->update(dt, 10.0f); // Use default interpolation rate
  }
  
  // Master-only logic
  if (isMaster) {
    // Update intangible timer
    if (dropData->mode == ItemDropDataComponent::Mode::Intangible) {
      dropData->intangibleTime -= dt;
      if (dropData->intangibleTime <= 0) {
        dropData->mode = ItemDropDataComponent::Mode::Available;
        markNetworkDirty();
      }
    }
    
    // Update drop age
    if (!dropData->eternal) {
      dropData->dropAge += dt;
      
      // Check max age from config
      float maxAge = m_config.getFloat("disappearTime", 300.0f);
      if (dropData->dropAge >= maxAge) {
        dropData->mode = ItemDropDataComponent::Mode::Dead;
        markNetworkDirty();
      }
    }
    
    // Update taken state
    if (dropData->mode == ItemDropDataComponent::Mode::Taken) {
      updateTaken(true, dt);
    }
  }
  
  // Physics update is handled by MovementSystem
}

void ItemDropAdapter::updateTaken(bool master, float dt) {
  auto* dropData = getComponent<ItemDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  
  if (!dropData || !transform || !velocity) return;
  
  dropData->takenTimer += dt;
  
  if (dropData->takenTimer >= m_afterTakenLife) {
    dropData->mode = ItemDropDataComponent::Mode::Dead;
    markNetworkDirty();
    return;
  }
  
  // Move towards owning entity
  if (dropData->owningEntity != NullEntityId && inWorld()) {
    if (auto owner = world()->entity(dropData->owningEntity)) {
      Vec2F targetPos = owner->position() + m_overheadOffset;
      Vec2F diff = targetPos - transform->position;
      velocity->velocity = diff * m_velocityApproach;
    }
  }
}

void ItemDropAdapter::render(RenderCallback* renderCallback) {
  auto* transform = getComponent<TransformComponent>();
  if (!transform) return;
  
  ItemPtr currentItem = item();
  if (!currentItem) return;
  
  Vec2F pos = position();
  List<Drawable> drawables = currentItem->dropDrawables();
  
  for (auto& drawable : drawables) {
    drawable.translate(pos);
    renderCallback->addDrawable(std::move(drawable), RenderLayerItemDrop);
  }
}

void ItemDropAdapter::renderLightSources(RenderCallback* renderCallback) {
  // ItemDrops don't typically have light sources, but could be extended
  EntityAdapter::renderLightSources(renderCallback);
}

ItemPtr ItemDropAdapter::item() const {
  if (!m_cachedItem) {
    auto* dropData = getComponent<ItemDropDataComponent>();
    if (dropData && !dropData->itemDescriptor.isNull()) {
      m_cachedItem = Root::singleton().itemDatabase()->item(dropData->itemDescriptor);
    }
  }
  return m_cachedItem;
}

void ItemDropAdapter::setEternal(bool eternal) {
  auto* dropData = getComponent<ItemDropDataComponent>();
  if (dropData) {
    dropData->eternal = eternal;
  }
}

void ItemDropAdapter::setIntangibleTime(float intangibleTime) {
  auto* dropData = getComponent<ItemDropDataComponent>();
  if (dropData) {
    dropData->intangibleTime = intangibleTime;
    if (intangibleTime > 0 && dropData->mode == ItemDropDataComponent::Mode::Available) {
      dropData->mode = ItemDropDataComponent::Mode::Intangible;
      markNetworkDirty();
    }
  }
}

ItemPtr ItemDropAdapter::takeBy(EntityId entityId, float timeOffset) {
  auto* dropData = getComponent<ItemDropDataComponent>();
  if (!dropData || dropData->mode != ItemDropDataComponent::Mode::Available) {
    return nullptr;
  }
  
  dropData->mode = ItemDropDataComponent::Mode::Taken;
  dropData->owningEntity = entityId;
  dropData->takenTimer = timeOffset;
  markNetworkDirty();
  
  return item();
}

ItemPtr ItemDropAdapter::take() {
  auto* dropData = getComponent<ItemDropDataComponent>();
  if (!dropData || dropData->mode != ItemDropDataComponent::Mode::Available) {
    return nullptr;
  }
  
  dropData->mode = ItemDropDataComponent::Mode::Dead;
  markNetworkDirty();
  
  return item();
}

bool ItemDropAdapter::canTake() const {
  auto* dropData = getComponent<ItemDropDataComponent>();
  return dropData && dropData->mode == ItemDropDataComponent::Mode::Available;
}

void ItemDropAdapter::setPosition(Vec2F const& position) {
  auto* transform = getComponent<TransformComponent>();
  if (transform) {
    transform->position = position;
    markNetworkDirty();
  }
}

Vec2F ItemDropAdapter::velocity() const {
  auto* vel = getComponent<VelocityComponent>();
  return vel ? vel->velocity : Vec2F();
}

void ItemDropAdapter::setVelocity(Vec2F const& velocity) {
  auto* vel = getComponent<VelocityComponent>();
  if (vel) {
    vel->velocity = velocity;
    markNetworkDirty();
  }
}

void ItemDropAdapter::updateCollisionPoly() {
  // Update collision based on item drawables
  ItemPtr currentItem = item();
  if (!currentItem) return;
  
  List<Drawable> drawables = currentItem->dropDrawables();
  if (drawables.empty()) return;
  
  // Calculate bounds from drawables
  RectF bounds = RectF::null();
  for (auto const& drawable : drawables) {
    bounds.combine(drawable.boundBox(false));
  }
  
  if (!bounds.isNull()) {
    auto* boundsComp = getComponent<BoundsComponent>();
    if (boundsComp) {
      boundsComp->metaBoundBox = bounds.padded(0.5f);
      boundsComp->collisionArea = bounds;
    }
  }
}

} // namespace ECS
} // namespace Star
