#include "StarEcsWorld.hpp"

namespace Star {
namespace ECS {

World::World() = default;

World::~World() {
  clear();
}

Entity World::createEntity() {
  uint32_t index;
  uint32_t generation;
  
  if (!m_freeIndices.empty()) {
    // Reuse a recycled index
    index = m_freeIndices.front();
    m_freeIndices.pop();
    generation = m_entityGenerations[index];
  } else {
    // Allocate new index
    index = m_nextIndex++;
    generation = 0;
    
    // Grow generation array if needed
    if (index >= m_entityGenerations.size()) {
      m_entityGenerations.resize(index + 1, 0);
    }
    m_entityGenerations[index] = generation;
  }
  
  ++m_livingEntities;
  
  return packEntity({index, generation});
}

void World::destroyEntity(Entity entity) {
  if (!isAlive(entity)) {
    return;
  }
  
  auto version = unpackEntity(entity);
  
  // Notify all component arrays to remove this entity's components
  for (auto& [typeId, array] : m_componentArrays) {
    array->entityDestroyed(entity);
  }
  
  // Increment generation to invalidate the entity
  m_entityGenerations[version.index]++;
  
  // Recycle the index
  m_freeIndices.push(version.index);
  
  --m_livingEntities;
}

bool World::isAlive(Entity entity) const {
  if (entity == NullEntity) {
    return false;
  }
  
  auto version = unpackEntity(entity);
  
  if (version.index >= m_entityGenerations.size()) {
    return false;
  }
  
  return m_entityGenerations[version.index] == version.generation;
}

List<Entity> World::entities() const {
  List<Entity> result;
  result.reserve(m_livingEntities);
  
  for (uint32_t i = 1; i < m_entityGenerations.size(); ++i) {
    Entity entity = packEntity({i, m_entityGenerations[i]});
    
    // Check if this index is not in the free list
    // (Simple check: if generation matches and not recycled)
    // This is a simplified check - a more robust implementation
    // would track living entities explicitly
    bool isFree = false;
    std::queue<uint32_t> tempQueue = m_freeIndices;
    while (!tempQueue.empty()) {
      if (tempQueue.front() == i) {
        isFree = true;
        break;
      }
      tempQueue.pop();
    }
    
    if (!isFree) {
      result.append(entity);
    }
  }
  
  return result;
}

void World::update(float dt) {
  for (auto& system : m_systems) {
    system->update(dt);
  }
  
  for (auto& system : m_systems) {
    system->postUpdate(dt);
  }
}

void World::clear() {
  // Uninit all systems
  for (auto& system : m_systems) {
    system->uninit();
  }
  m_systems.clear();
  
  // Clear all component arrays
  m_componentArrays.clear();
  
  // Reset entity management
  m_entityGenerations.clear();
  m_freeIndices = std::queue<uint32_t>();
  m_nextIndex = 1;
  m_livingEntities = 0;
}

} // namespace ECS
} // namespace Star
