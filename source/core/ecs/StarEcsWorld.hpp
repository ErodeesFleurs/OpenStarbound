#pragma once

#include "StarEcsTypes.hpp"
#include "StarEcsComponent.hpp"
#include "StarEcsSystem.hpp"
#include "StarEcsView.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"

#include <memory>
#include <algorithm>
#include <queue>

namespace Star {
namespace ECS {

// The World is the main container for all entities, components, and systems
class World {
public:
  World();
  ~World();
  
  // Entity management
  
  // Create a new entity
  Entity createEntity();
  
  // Destroy an entity and all its components
  void destroyEntity(Entity entity);
  
  // Check if an entity is still alive
  bool isAlive(Entity entity) const;
  
  // Get all living entities
  List<Entity> entities() const;
  
  // Get entity count
  size_t entityCount() const { return m_livingEntities; }
  
  // Component management
  
  // Add a component to an entity
  template<typename T, typename... Args>
  T& addComponent(Entity entity, Args&&... args) {
    if (!isAlive(entity)) {
      throw EcsException("Cannot add component to dead entity");
    }
    
    ComponentArray<T>* array = getOrCreateComponentArray<T>();
    array->insert(entity, T{std::forward<Args>(args)...});
    return *array->get(entity);
  }
  
  // Add component with existing value
  template<typename T>
  T& setComponent(Entity entity, T component) {
    if (!isAlive(entity)) {
      throw EcsException("Cannot add component to dead entity");
    }
    
    ComponentArray<T>* array = getOrCreateComponentArray<T>();
    array->insert(entity, std::move(component));
    return *array->get(entity);
  }
  
  // Remove a component from an entity
  template<typename T>
  void removeComponent(Entity entity) {
    ComponentArray<T>* array = getComponentArray<T>();
    if (array) {
      array->remove(entity);
    }
  }
  
  // Get a component from an entity (returns nullptr if not found)
  template<typename T>
  T* getComponent(Entity entity) {
    ComponentArray<T>* array = getComponentArray<T>();
    return array ? array->get(entity) : nullptr;
  }
  
  template<typename T>
  T const* getComponent(Entity entity) const {
    ComponentArray<T> const* array = getComponentArray<T>();
    return array ? array->get(entity) : nullptr;
  }
  
  // Check if entity has a component
  template<typename T>
  bool hasComponent(Entity entity) const {
    ComponentArray<T> const* array = getComponentArray<T>();
    return array && array->has(entity);
  }
  
  // Check if entity has all specified components
  template<typename... Components>
  bool hasComponents(Entity entity) const {
    return (hasComponent<Components>(entity) && ...);
  }
  
  // Get component array for direct iteration
  template<typename T>
  ComponentArray<T>* getComponentArray() {
    auto typeId = getComponentTypeId<T>();
    auto it = m_componentArrays.find(typeId);
    if (it == m_componentArrays.end()) {
      return nullptr;
    }
    return static_cast<ComponentArray<T>*>(it->second.get());
  }
  
  template<typename T>
  ComponentArray<T> const* getComponentArray() const {
    auto typeId = getComponentTypeId<T>();
    auto it = m_componentArrays.find(typeId);
    if (it == m_componentArrays.end()) {
      return nullptr;
    }
    return static_cast<ComponentArray<T> const*>(it->second.get());
  }
  
  // View creation for querying entities
  
  // Create a view for entities with specific components
  template<typename... Components>
  View<Components...> view() {
    return View<Components...>(this);
  }
  
  // Create a simple view for single component
  template<typename T>
  SingleView<T> singleView() {
    return SingleView<T>(getOrCreateComponentArray<T>());
  }
  
  // System management
  
  // Add a system to the world
  template<typename T, typename... Args>
  T* addSystem(Args&&... args) {
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = system.get();
    
    system->init(this);
    
    // Insert in priority order
    auto it = std::lower_bound(m_systems.begin(), m_systems.end(), system,
      [](auto const& a, auto const& b) {
        return a->priority() < b->priority();
      });
    
    m_systems.insert(it, std::move(system));
    return ptr;
  }
  
  // Remove a system by type
  template<typename T>
  void removeSystem() {
    m_systems.erase(
      std::remove_if(m_systems.begin(), m_systems.end(),
        [](auto const& system) {
          return dynamic_cast<T*>(system.get()) != nullptr;
        }),
      m_systems.end());
  }
  
  // Get a system by type
  template<typename T>
  T* getSystem() {
    for (auto& system : m_systems) {
      if (auto ptr = dynamic_cast<T*>(system.get())) {
        return ptr;
      }
    }
    return nullptr;
  }
  
  // Update all systems
  void update(float dt);
  
  // Clear all entities and components
  void clear();
  
private:
  template<typename T>
  ComponentArray<T>* getOrCreateComponentArray() {
    auto typeId = getComponentTypeId<T>();
    auto it = m_componentArrays.find(typeId);
    if (it == m_componentArrays.end()) {
      auto array = std::make_unique<ComponentArray<T>>();
      auto* ptr = array.get();
      m_componentArrays[typeId] = std::move(array);
      return ptr;
    }
    return static_cast<ComponentArray<T>*>(it->second.get());
  }
  
  // Entity management
  std::vector<uint32_t> m_entityGenerations; // Generation counter per entity index
  std::queue<uint32_t> m_freeIndices; // Recycled entity indices
  uint32_t m_nextIndex = 1; // Start at 1 so 0 can be NullEntity
  size_t m_livingEntities = 0;
  
  // Component storage
  HashMap<ComponentTypeId, std::unique_ptr<IComponentArray>> m_componentArrays;
  
  // Systems
  std::vector<SystemPtr> m_systems;
};

// View implementation

template<typename... Components>
View<Components...>::View(World* world) : m_world(world) {
  // Get the smallest component array to iterate over
  // This is an optimization to reduce iterations
  
  // For now, just get all entities
  // A more sophisticated implementation would use the smallest set
  m_entities = world->entities();
}

template<typename... Components>
bool View<Components...>::Iterator::isValid() const {
  Entity entity = (*m_entities)[m_index];
  return m_world->hasComponents<Components...>(entity);
}

template<typename... Components>
std::tuple<Entity, Components&...> View<Components...>::Iterator::operator*() {
  Entity entity = (*m_entities)[m_index];
  return std::tie(entity, *m_world->getComponent<Components>(entity)...);
}

template<typename... Components>
typename View<Components...>::Iterator View<Components...>::begin() {
  return Iterator(m_world, &m_entities, 0);
}

template<typename... Components>
typename View<Components...>::Iterator View<Components...>::end() {
  return Iterator(m_world, &m_entities, m_entities.size());
}

template<typename... Components>
size_t View<Components...>::count() const {
  size_t n = 0;
  for (Entity entity : m_entities) {
    if (m_world->hasComponents<Components...>(entity)) {
      ++n;
    }
  }
  return n;
}

template<typename... Components>
template<typename Func>
void View<Components...>::each(Func&& func) {
  for (auto it = begin(); it != end(); ++it) {
    std::apply(func, *it);
  }
}

// ArchetypeSystem implementation
template<typename... Components>
void ArchetypeSystem<Components...>::update(float dt) {
  for (auto [entity, components...] : m_world->template view<Components...>()) {
    processEntity(entity, components..., dt);
  }
}

} // namespace ECS
} // namespace Star
