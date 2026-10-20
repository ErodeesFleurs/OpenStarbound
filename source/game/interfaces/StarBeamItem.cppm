module;
import std;
import star.type_identity;
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
import star.casting;
import star.strong_typedef;
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
import star.lua_root;

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
class Root;

class Item;
class ToolUserEntity;
class World;

class BeamItem : public virtual NonRotatedDrawablesItem, public virtual ToolUserItem {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<BeamItem, NonRotatedDrawablesItem, ToolUserItem>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<BeamItem>();
  }

  enum class EndType { Invalid = -1, Object, Tile, TileGroup, Wire };

  BeamItem(Root& root, Json config);
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
  Root& beamRoot() const;
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

private:
  Root* m_beamRoot;
};

}

export module star.beam_item;

export namespace Star {
  using ::Star::Item;
  using ::Star::ToolUserEntity;
  using ::Star::World;
  using ::Star::BeamItem;
}
