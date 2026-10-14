module;
#include "StarConfig.hpp"
import star.set;
import star.format;
import star.json;
import star.id_map;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
import star.poly;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.maybe;
import star.variant;
import star.rpc_promise;
import star.a_star;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.animation;
import star.particle;
import star.interaction_types;


import star.tile_damage;

import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;

import star.interactive_entity;
import star.collision_block;

import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"

import star.force_regions;

import star.world;

import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;

import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;

import star.mobile_entity;

import star.actor_entity;

import star.tool_user_entity;

namespace Star {

struct ToolUserItemExceptionTag {
  static constexpr char const* name() { return "ToolUserItemException"; }
};
using ToolUserItemException = StarError<ToolUserItemExceptionTag, StarException>;

STAR_CLASS(ToolUserItem);

// FIXME: You know what another name for an item that a tool user uses is?  A
// Tool.  Three words when one will do, rename.
class ToolUserItem {
public:
  ToolUserItem();
  virtual ~ToolUserItem() = default;

  // Owner must be initialized when a ToolUserItem is initialized and
  // uninitialized before the owner is uninitialized.
  virtual void init(ToolUserEntity* owner, ToolHand hand);
  virtual void uninit();

  // Default implementation does nothing
  virtual void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves);

  // Default implementations return empty list
  virtual List<DamageSource> damageSources() const;
  virtual List<PolyF> shieldPolys() const;
  virtual List<PhysicsForceRegion> forceRegions() const;

  bool initialized() const;

  // owner, entityMode, hand, and world throw ToolUserException if
  // initialized() is false
  ToolUserEntity* owner() const;
  EntityMode entityMode() const;
  ToolHand hand() const;
  World* world() const;

private:
  ToolUserEntity* m_owner;
  Maybe<ToolHand> m_hand;
};

}

export module star.tool_user_item;

export namespace Star {
  using ::Star::ToolUserItemExceptionTag;
  using ::Star::ToolUserItemException;
  using ::Star::ToolUserItem;
  using ::Star::ToolUserItemPtr;
  using ::Star::ToolUserItemConstPtr;
  using ::Star::ToolUserItemWeakPtr;
  using ::Star::ToolUserItemConstWeakPtr;
  using ::Star::ToolUserItemUPtr;
  using ::Star::ToolUserItemConstUPtr;
}
