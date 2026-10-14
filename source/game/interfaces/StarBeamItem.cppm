module;
import std;
import star.set;
import star.json;
import star.id_map;
import star.vector;
import star.spline;
import star.game_types;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.maybe;
import star.variant;
import star.rpc_promise;
import star.a_star;


import star.drawable;

import star.non_rotated_drawables_item;

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

namespace Star {

class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<const Item>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<const Item>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<const Item>;
class ToolUserEntity;
using ToolUserEntityPtr = std::shared_ptr<ToolUserEntity>;
using ToolUserEntityConstPtr = std::shared_ptr<const ToolUserEntity>;
using ToolUserEntityWeakPtr = std::weak_ptr<ToolUserEntity>;
using ToolUserEntityConstWeakPtr = std::weak_ptr<const ToolUserEntity>;
using ToolUserEntityUPtr = std::unique_ptr<ToolUserEntity>;
using ToolUserEntityConstUPtr = std::unique_ptr<const ToolUserEntity>;
class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;

class BeamItem;
using BeamItemPtr = std::shared_ptr<BeamItem>;
using BeamItemConstPtr = std::shared_ptr<const BeamItem>;
using BeamItemWeakPtr = std::weak_ptr<BeamItem>;
using BeamItemConstWeakPtr = std::weak_ptr<const BeamItem>;
using BeamItemUPtr = std::unique_ptr<BeamItem>;
using BeamItemConstUPtr = std::unique_ptr<const BeamItem>;

class BeamItem : public virtual NonRotatedDrawablesItem, public virtual ToolUserItem {
public:
  enum class EndType { Invalid = -1, Object, Tile, TileGroup, Wire };

  BeamItem(Json config);
  virtual ~BeamItem() = default;

  virtual void init(ToolUserEntity* owner, ToolHand hand) override;
  virtual void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  virtual List<Drawable> nonRotatedDrawables() const override;

  virtual float getAngle(float angle);
  virtual List<Drawable> drawables() const;
  virtual Vec2F handPosition() const;
  virtual Vec2F firePosition() const;
  virtual void setRange(float range);
  virtual float getAppropriateOpacity() const;
  virtual void setEnd(EndType type);

protected:
  List<Drawable> beamDrawables(bool canPlace = true) const;

  String m_image;
  StringList m_endImages;
  EndType m_endType;

  float m_segmentsPerUnit;
  float m_nearControlPointElasticity;
  float m_farControlPointElasticity;
  float m_nearControlPointDistance;
  Vec2F m_handPosition;
  Vec2F m_firePosition;
  float m_range;

  float m_targetSegmentRun;
  float m_minBeamWidth;
  float m_maxBeamWidth;
  float m_beamWidthDev;
  float m_minBeamJitter;
  float m_maxBeamJitter;
  float m_beamJitterDev;
  float m_minBeamTrans;
  float m_maxBeamTrans;
  float m_beamTransDev;
  int m_minBeamLines;
  int m_maxBeamLines;
  float m_innerBrightnessScale;
  float m_firstStripeThickness;
  float m_secondStripeThickness;
  Color m_color;

  mutable bool m_inRangeLastUpdate;
  mutable Color m_lastUpdateColor;
  mutable float m_particleGenerateCooldown;

  CSplineF m_beamCurve;
};

}

export module star.beam_item;

export namespace Star {
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::ToolUserEntity;
  using ::Star::ToolUserEntityPtr;
  using ::Star::ToolUserEntityConstPtr;
  using ::Star::ToolUserEntityWeakPtr;
  using ::Star::ToolUserEntityConstWeakPtr;
  using ::Star::ToolUserEntityUPtr;
  using ::Star::ToolUserEntityConstUPtr;
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::BeamItem;
  using ::Star::BeamItemPtr;
  using ::Star::BeamItemConstPtr;
  using ::Star::BeamItemWeakPtr;
  using ::Star::BeamItemConstWeakPtr;
  using ::Star::BeamItemUPtr;
  using ::Star::BeamItemConstUPtr;
}
