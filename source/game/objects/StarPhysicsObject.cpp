#include "StarPhysicsObject.hpp"
#include "StarJsonExtra.hpp"
#include "StarInterpolation.hpp"
#include "StarRoot.hpp"
#include "StarObjectDatabase.hpp"
#include "StarLuaConverters.hpp"

namespace Star {

PhysicsObject::PhysicsObject(ObjectConfigConstPtr config, Json const& parameters) : Object(std::move(config), parameters) {
  for (auto const& [forceName, forceJson] : configValue("physicsForces", JsonObject()).iterateObject()) {
    auto& forceConfig = m_physicsForces[forceName];

    forceConfig.forceRegion = jsonToPhysicsForceRegion(forceJson);
    forceConfig.enabled.set(forceJson.getBool("enabled", true));
  }

  for (auto const& [collisionName, collisionJson] : configValue("physicsCollisions", JsonObject()).iterateObject()) {
    auto& collisionConfig = m_physicsCollisions[collisionName];
    collisionConfig.movingCollision = PhysicsMovingCollision::fromJson(collisionJson);
    collisionConfig.xPosition.set(take(collisionConfig.movingCollision.position[0]));
    collisionConfig.yPosition.set(take(collisionConfig.movingCollision.position[1]));
    collisionConfig.enabled.set(collisionJson.getBool("enabled", true));
  }

  m_physicsForces.sortByKey();
  for (auto& [forceName, forceConfig] : m_physicsForces)
    m_netGroup.addNetElement(&forceConfig.enabled);

  m_physicsCollisions.sortByKey();
  for (auto& [collisionName, collisionConfig] : m_physicsCollisions) {
    m_netGroup.addNetElement(&collisionConfig.xPosition);
    m_netGroup.addNetElement(&collisionConfig.yPosition);
    collisionConfig.xPosition.setInterpolator(lerp<float, float>);
    collisionConfig.yPosition.setInterpolator(lerp<float, float>);
    m_netGroup.addNetElement(&collisionConfig.enabled);
  }
}

void PhysicsObject::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void PhysicsObject::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

void PhysicsObject::init(World* world, EntityId entityId, EntityMode mode) {
  if (mode == EntityMode::Master) {
    LuaCallbacks physicsCallbacks;
    physicsCallbacks.registerCallback("setForceEnabled", [this](String const& force, bool enabled) {
        m_physicsForces.get(force).enabled.set(enabled);
      });
    physicsCallbacks.registerCallback("setCollisionPosition", [this](String const& collision, Vec2F const& pos) {
        auto& collisionConfig = m_physicsCollisions.get(collision);
        collisionConfig.xPosition.set(pos[0]);
        collisionConfig.yPosition.set(pos[1]);
      });
    physicsCallbacks.registerCallback("setCollisionEnabled", [this](String const& collision, bool const& enabled) {
        auto& collisionConfig = m_physicsCollisions.get(collision);
        collisionConfig.enabled.set(enabled);
      });
    m_scriptComponent.addCallbacks("physics", std::move(physicsCallbacks));
  }
  Object::init(world, entityId, mode);
  m_metaBoundBox = Object::metaBoundBox();
  for (auto const& [_, forceRegionConfig] : m_physicsForces) {
    PhysicsForceRegion forceRegion = forceRegionConfig.forceRegion;
    forceRegion.call([pos = position()](auto& fr) { fr.translate(pos); });
    m_metaBoundBox.combine(forceRegion.call([](auto& fr) { return fr.boundBox(); }));
  }
}

void PhysicsObject::uninit() {
  m_scriptComponent.removeCallbacks("physics");
  Object::uninit();
}

void PhysicsObject::update(float dt, uint64_t currentStep) {
  Object::update(dt, currentStep);
  if (isSlave())
    m_netGroup.tickNetInterpolation(dt);
}

RectF PhysicsObject::metaBoundBox() const {
  return m_metaBoundBox;
}

List<PhysicsForceRegion> PhysicsObject::forceRegions() const {
  List<PhysicsForceRegion> forces;
  for (auto const& [_, forceRegionConfig] : m_physicsForces) {
    if (forceRegionConfig.enabled.get()) {
      PhysicsForceRegion forceRegion = forceRegionConfig.forceRegion;
      forceRegion.call([pos = position()](auto& fr) { fr.translate(pos); });
      forces.append(std::move(forceRegion));
    }
  }
  return forces;
}

size_t PhysicsObject::movingCollisionCount() const {
  return m_physicsCollisions.size();
}

Maybe<PhysicsMovingCollision> PhysicsObject::movingCollision(size_t positionIndex) const {
  auto const& collisionConfig = m_physicsCollisions.valueAt(positionIndex);
  if (!collisionConfig.enabled.get())
    return {};
  PhysicsMovingCollision collision = collisionConfig.movingCollision;
  collision.translate(position() + Vec2F(collisionConfig.xPosition.get(), collisionConfig.yPosition.get()));
  return collision;
}

}
