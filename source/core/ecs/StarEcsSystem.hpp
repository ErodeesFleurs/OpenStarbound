#pragma once

#include "StarEcsTypes.hpp"

namespace Star {
namespace ECS {

// Forward declaration
class World;

// Base class for all systems
class System {
public:
  virtual ~System() = default;
  
  // Called when system is added to the world
  virtual void init(World* world) {
    m_world = world;
  }
  
  // Called when system is removed from the world
  virtual void uninit() {
    m_world = nullptr;
  }
  
  // Main update function, called every frame
  virtual void update(float dt) = 0;
  
  // Priority determines update order (lower = earlier)
  virtual int priority() const { return 0; }
  
  // Optional: Called after all systems have been updated
  virtual void postUpdate(float dt) {}
  
  // Optional: Called at fixed intervals for physics
  virtual void fixedUpdate(float dt) {}
  
  // Get the world this system belongs to
  World* world() const { return m_world; }
  
protected:
  World* m_world = nullptr;
};

using SystemPtr = std::unique_ptr<System>;

// System that runs only on specific entity archetypes
template<typename... Components>
class ArchetypeSystem : public System {
public:
  // Override this to process entities with the specified components
  virtual void processEntity(Entity entity, Components&... components, float dt) = 0;
  
  void update(float dt) override;
};

// Marker for systems that should run on a separate thread
class ThreadedSystem : public System {
public:
  // Whether this system can run in parallel with others
  virtual bool canRunParallel() const { return true; }
};

} // namespace ECS
} // namespace Star
