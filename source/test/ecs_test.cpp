#include "ecs/StarEcs.hpp"
#include "StarString.hpp"

#include "gtest/gtest.h"

using namespace Star;
using namespace Star::ECS;

// Simple test components
struct Position {
  float x = 0.0f;
  float y = 0.0f;
};

struct Velocity {
  float dx = 0.0f;
  float dy = 0.0f;
};

struct Health {
  float current = 100.0f;
  float max = 100.0f;
};

struct Name {
  String name;
};

// Test system
class MovementSystem : public System {
public:
  void update(float dt) override {
    for (auto [entity, pos, vel] : m_world->view<Position, Velocity>()) {
      pos.x += vel.dx * dt;
      pos.y += vel.dy * dt;
    }
    m_updateCount++;
  }
  
  int priority() const override { return 100; }
  
  int updateCount() const { return m_updateCount; }

private:
  int m_updateCount = 0;
};

// Basic entity tests
TEST(EcsTest, CreateEntity) {
  World world;
  
  Entity e1 = world.createEntity();
  Entity e2 = world.createEntity();
  
  EXPECT_NE(e1, NullEntity);
  EXPECT_NE(e2, NullEntity);
  EXPECT_NE(e1, e2);
  EXPECT_TRUE(world.isAlive(e1));
  EXPECT_TRUE(world.isAlive(e2));
}

TEST(EcsTest, DestroyEntity) {
  World world;
  
  Entity e1 = world.createEntity();
  EXPECT_TRUE(world.isAlive(e1));
  
  world.destroyEntity(e1);
  EXPECT_FALSE(world.isAlive(e1));
}

TEST(EcsTest, EntityReuse) {
  World world;
  
  Entity e1 = world.createEntity();
  uint32_t index1 = unpackEntity(e1).index;
  
  world.destroyEntity(e1);
  
  Entity e2 = world.createEntity();
  uint32_t index2 = unpackEntity(e2).index;
  
  // Index should be reused
  EXPECT_EQ(index1, index2);
  
  // But entities should be different (different generation)
  EXPECT_NE(e1, e2);
  
  // Old entity should no longer be alive
  EXPECT_FALSE(world.isAlive(e1));
  EXPECT_TRUE(world.isAlive(e2));
}

// Component tests
TEST(EcsTest, AddComponent) {
  World world;
  Entity e = world.createEntity();
  
  auto& pos = world.addComponent<Position>(e, 10.0f, 20.0f);
  
  EXPECT_EQ(pos.x, 10.0f);
  EXPECT_EQ(pos.y, 20.0f);
  EXPECT_TRUE(world.hasComponent<Position>(e));
}

TEST(EcsTest, GetComponent) {
  World world;
  Entity e = world.createEntity();
  
  world.addComponent<Position>(e, 10.0f, 20.0f);
  
  Position* pos = world.getComponent<Position>(e);
  ASSERT_NE(pos, nullptr);
  EXPECT_EQ(pos->x, 10.0f);
  EXPECT_EQ(pos->y, 20.0f);
}

TEST(EcsTest, GetNonexistentComponent) {
  World world;
  Entity e = world.createEntity();
  
  Velocity* vel = world.getComponent<Velocity>(e);
  EXPECT_EQ(vel, nullptr);
}

TEST(EcsTest, RemoveComponent) {
  World world;
  Entity e = world.createEntity();
  
  world.addComponent<Position>(e, 10.0f, 20.0f);
  EXPECT_TRUE(world.hasComponent<Position>(e));
  
  world.removeComponent<Position>(e);
  EXPECT_FALSE(world.hasComponent<Position>(e));
}

TEST(EcsTest, MultipleComponents) {
  World world;
  Entity e = world.createEntity();
  
  world.addComponent<Position>(e, 10.0f, 20.0f);
  world.addComponent<Velocity>(e, 1.0f, 2.0f);
  world.addComponent<Health>(e);
  
  EXPECT_TRUE(world.hasComponent<Position>(e));
  EXPECT_TRUE(world.hasComponent<Velocity>(e));
  EXPECT_TRUE(world.hasComponent<Health>(e));
  EXPECT_FALSE(world.hasComponent<Name>(e));
}

TEST(EcsTest, HasComponents) {
  World world;
  Entity e = world.createEntity();
  
  world.addComponent<Position>(e, 10.0f, 20.0f);
  world.addComponent<Velocity>(e, 1.0f, 2.0f);
  
  EXPECT_TRUE((world.hasComponents<Position, Velocity>(e)));
  EXPECT_FALSE((world.hasComponents<Position, Velocity, Health>(e)));
}

TEST(EcsTest, ComponentDestroyed) {
  World world;
  Entity e = world.createEntity();
  
  world.addComponent<Position>(e, 10.0f, 20.0f);
  EXPECT_TRUE(world.hasComponent<Position>(e));
  
  world.destroyEntity(e);
  
  // After entity destruction, the component should be removed
  // The entity is no longer alive, so we can't query it
  EXPECT_FALSE(world.isAlive(e));
}

// View tests
TEST(EcsTest, ViewIteration) {
  World world;
  
  // Create entities with Position
  Entity e1 = world.createEntity();
  world.addComponent<Position>(e1, 1.0f, 1.0f);
  
  Entity e2 = world.createEntity();
  world.addComponent<Position>(e2, 2.0f, 2.0f);
  
  Entity e3 = world.createEntity();
  world.addComponent<Position>(e3, 3.0f, 3.0f);
  world.addComponent<Velocity>(e3, 1.0f, 0.0f);
  
  // Count entities with Position
  int count = 0;
  for (auto [entity, pos] : world.view<Position>()) {
    count++;
  }
  EXPECT_EQ(count, 3);
  
  // Count entities with Position and Velocity
  count = 0;
  for (auto [entity, pos, vel] : world.view<Position, Velocity>()) {
    count++;
  }
  EXPECT_EQ(count, 1);
}

TEST(EcsTest, ViewModification) {
  World world;
  
  Entity e = world.createEntity();
  world.addComponent<Position>(e, 0.0f, 0.0f);
  
  // Modify through view
  for (auto [entity, pos] : world.view<Position>()) {
    pos.x = 100.0f;
    pos.y = 200.0f;
  }
  
  // Verify modification
  Position* pos = world.getComponent<Position>(e);
  ASSERT_NE(pos, nullptr);
  EXPECT_EQ(pos->x, 100.0f);
  EXPECT_EQ(pos->y, 200.0f);
}

// System tests
TEST(EcsTest, AddSystem) {
  World world;
  
  auto* movement = world.addSystem<MovementSystem>();
  ASSERT_NE(movement, nullptr);
  
  auto* retrieved = world.getSystem<MovementSystem>();
  EXPECT_EQ(movement, retrieved);
}

TEST(EcsTest, SystemUpdate) {
  World world;
  
  Entity e = world.createEntity();
  world.addComponent<Position>(e, 0.0f, 0.0f);
  world.addComponent<Velocity>(e, 10.0f, 20.0f);
  
  auto* movement = world.addSystem<MovementSystem>();
  
  // Update for 1 second
  world.update(1.0f);
  
  Position* pos = world.getComponent<Position>(e);
  ASSERT_NE(pos, nullptr);
  EXPECT_FLOAT_EQ(pos->x, 10.0f);
  EXPECT_FLOAT_EQ(pos->y, 20.0f);
  EXPECT_EQ(movement->updateCount(), 1);
  
  // Update for another 0.5 seconds
  world.update(0.5f);
  
  EXPECT_FLOAT_EQ(pos->x, 15.0f);
  EXPECT_FLOAT_EQ(pos->y, 30.0f);
  EXPECT_EQ(movement->updateCount(), 2);
}

// Component array tests
TEST(EcsTest, ComponentArrayIteration) {
  World world;
  
  Entity e1 = world.createEntity();
  Entity e2 = world.createEntity();
  Entity e3 = world.createEntity();
  
  world.addComponent<Position>(e1, 1.0f, 1.0f);
  world.addComponent<Position>(e2, 2.0f, 2.0f);
  world.addComponent<Position>(e3, 3.0f, 3.0f);
  
  auto* array = world.getComponentArray<Position>();
  ASSERT_NE(array, nullptr);
  EXPECT_EQ(array->size(), 3);
  
  float sum = 0.0f;
  for (auto [entity, pos] : *array) {
    sum += pos.x;
  }
  EXPECT_FLOAT_EQ(sum, 6.0f);
}

TEST(EcsTest, SingleView) {
  World world;
  
  Entity e1 = world.createEntity();
  Entity e2 = world.createEntity();
  
  world.addComponent<Position>(e1, 1.0f, 0.0f);
  world.addComponent<Position>(e2, 2.0f, 0.0f);
  world.addComponent<Velocity>(e2, 1.0f, 0.0f);
  
  auto view = world.singleView<Position>();
  EXPECT_EQ(view.size(), 2);
  EXPECT_FALSE(view.empty());
}

// Clear tests
TEST(EcsTest, ClearWorld) {
  World world;
  
  Entity e1 = world.createEntity();
  Entity e2 = world.createEntity();
  
  world.addComponent<Position>(e1, 1.0f, 1.0f);
  world.addComponent<Position>(e2, 2.0f, 2.0f);
  world.addSystem<MovementSystem>();
  
  EXPECT_EQ(world.entityCount(), 2);
  
  world.clear();
  
  EXPECT_EQ(world.entityCount(), 0);
  EXPECT_EQ(world.getSystem<MovementSystem>(), nullptr);
}
