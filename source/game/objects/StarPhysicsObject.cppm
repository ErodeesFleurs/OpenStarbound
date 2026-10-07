module;
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarList.hpp"

#include "StarObject.hpp"
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.force_regions;
import star.physics_entity;

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
