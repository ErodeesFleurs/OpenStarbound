module;

#include "StarObject.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarJsonExtra.hpp"
#include "StarInterpolation.hpp"
#include "StarRoot.hpp"
#include "StarObjectDatabase.hpp"
#include "StarLuaConverters.hpp"

export module star.physics_object;

export namespace Star {

class PhysicsObject : public Object, public virtual PhysicsEntity {
public:
  PhysicsObject(ObjectConfigConstPtr config, Json const& parameters = Json());

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  void update(float dt, uint64_t currentStep) override;

  RectF metaBoundBox() const override;

  List<PhysicsForceRegion> forceRegions() const override;

  size_t movingCollisionCount() const override;
  Maybe<PhysicsMovingCollision> movingCollision(size_t positionIndex) const override;

private:
  struct PhysicsForceConfig {
    PhysicsForceRegion forceRegion;
    NetElementBool enabled;
  };

  struct PhysicsCollisionConfig {
    PhysicsMovingCollision movingCollision;
    NetElementFloat xPosition;
    NetElementFloat yPosition;
    NetElementBool enabled;
  };

  OrderedHashMap<String, PhysicsForceConfig> m_physicsForces;
  OrderedHashMap<String, PhysicsCollisionConfig> m_physicsCollisions;

  RectF m_metaBoundBox;
};

}

namespace Star {

PhysicsObject::PhysicsObject(ObjectConfigConstPtr config, Json const& parameters) : Object(std::move(config), parameters) {
  for (auto const& p : configValue("physicsForces", JsonObject()).iterateObject()) {
    auto& forceConfig = m_physicsForces[p.first];

    forceConfig.forceRegion = jsonToPhysicsForceRegion(p.second);
    forceConfig.enabled.set(p.second.getBool("enabled", true));
  }

  for (auto const& p : configValue("physicsCollisions", JsonObject()).iterateObject()) {
    auto& collisionConfig = m_physicsCollisions[p.first];
    collisionConfig.movingCollision = PhysicsMovingCollision::fromJson(p.second);
    collisionConfig.xPosition.set(take(collisionConfig.movingCollision.position[0]));
    collisionConfig.yPosition.set(take(collisionConfig.movingCollision.position[1]));
    collisionConfig.enabled.set(p.second.getBool("enabled", true));
  }

  m_physicsForces.sortByKey();
  for (auto& p : m_physicsForces)
    m_netGroup.addNetElement(&p.second.enabled);

  m_physicsCollisions.sortByKey();
  for (auto& p : m_physicsCollisions) {
    m_netGroup.addNetElement(&p.second.xPosition);
    m_netGroup.addNetElement(&p.second.yPosition);
    p.second.xPosition.setInterpolator(lerp<float, float>);
    p.second.yPosition.setInterpolator(lerp<float, float>);
    m_netGroup.addNetElement(&p.second.enabled);
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
  for (auto const& p : m_physicsForces) {
    PhysicsForceRegion forceRegion = p.second.forceRegion;
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
  for (auto const& p : m_physicsForces) {
    if (p.second.enabled.get()) {
      PhysicsForceRegion forceRegion = p.second.forceRegion;
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
