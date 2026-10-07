module;
#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarList.hpp"
#include "StarMathCommon.hpp"
#include "StarRandom.hpp"
import star.periodic;
#include "StarInterpolation.hpp"
import star.periodic_function;
#include "StarNetElementSystem.hpp"
#include "StarSet.hpp"
#include "StarColor.hpp"
#include "StarLua.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"


#include "StarLuaRoot.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.force_regions;
import star.physics_entity;


namespace Star {

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

export module star.physics_object;

export namespace Star {
  using ::Star::PhysicsObject;
}
