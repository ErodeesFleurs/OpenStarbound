module;
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"

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
