module;
#include "StarIdMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
#include "StarPoly.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"

import star.uuid;
import star.celestial_coordinate;
import star.warping;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.tile_damage;

import star.interaction_types;

import star.item_descriptor;
import star.quest_descriptor;

import star.interactive_entity;
import star.collision_block;

import star.tile_entity;

namespace Star {

STAR_CLASS(WarpTargetEntity);

class WarpTargetEntity : public virtual TileEntity {
public:
  // Foot position for things teleporting onto this entity, relative to root
  // position.
  virtual Vec2F footPosition() const = 0;
};

}

export module star.warp_target_entity;

export namespace Star {
  using ::Star::WarpTargetEntity;
  using ::Star::WarpTargetEntityPtr;
  using ::Star::WarpTargetEntityConstPtr;
  using ::Star::WarpTargetEntityWeakPtr;
  using ::Star::WarpTargetEntityConstWeakPtr;
  using ::Star::WarpTargetEntityUPtr;
  using ::Star::WarpTargetEntityConstUPtr;
}
