module;
import std;
import star.string;
import star.json;
import star.id_map;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
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

import star.tool_user_item;

import star.status_effect_item;
#include "StarLuaComponents.hpp"

import star.fireable_item;

namespace Star {

class SwingableItem;
using SwingableItemPtr = std::shared_ptr<SwingableItem>;
using SwingableItemConstPtr = std::shared_ptr<const SwingableItem>;
using SwingableItemWeakPtr = std::weak_ptr<SwingableItem>;
using SwingableItemConstWeakPtr = std::weak_ptr<const SwingableItem>;
using SwingableItemUPtr = std::unique_ptr<SwingableItem>;
using SwingableItemConstUPtr = std::unique_ptr<const SwingableItem>;

class SwingableItem : public FireableItem {
public:
  SwingableItem();
  SwingableItem(Json const& params);
  virtual ~SwingableItem() {}

  // These can be different
  // Default implementation is the same though
  virtual float getAngleDir(float aimAngle, Direction facingDirection);
  virtual float getAngle(float aimAngle);
  virtual float getItemAngle(float aimAngle);
  virtual String getArmFrame();

  virtual List<Drawable> drawables() const = 0;

  void setParams(Json const& params);

protected:
  float m_swingStart;
  float m_swingFinish;
  float m_swingAimFactor;
  Maybe<float> m_coolingDownAngle;
};

}

export module star.swingable_item;

export namespace Star {
  using ::Star::SwingableItem;
  using ::Star::SwingableItemPtr;
  using ::Star::SwingableItemConstPtr;
  using ::Star::SwingableItemWeakPtr;
  using ::Star::SwingableItemConstWeakPtr;
  using ::Star::SwingableItemUPtr;
  using ::Star::SwingableItemConstUPtr;
}
