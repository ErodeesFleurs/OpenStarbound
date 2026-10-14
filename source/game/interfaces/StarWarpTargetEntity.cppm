module;
import std;
import star.json;
import star.id_map;
#include "StarStrongTypedef.hpp"
import star.data_stream;
import star.vector;
import star.game_types;
#include "StarCasting.hpp"
import star.bi_map;
import star.list;

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

class WarpTargetEntity;
using WarpTargetEntityPtr = std::shared_ptr<WarpTargetEntity>;
using WarpTargetEntityConstPtr = std::shared_ptr<const WarpTargetEntity>;
using WarpTargetEntityWeakPtr = std::weak_ptr<WarpTargetEntity>;
using WarpTargetEntityConstWeakPtr = std::weak_ptr<const WarpTargetEntity>;
using WarpTargetEntityUPtr = std::unique_ptr<WarpTargetEntity>;
using WarpTargetEntityConstUPtr = std::unique_ptr<const WarpTargetEntity>;

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
