module;
import star.json;
import star.id_map;
import star.vector;
import star.spline;
import star.casting;
import star.strong_typedef;
import star.variant;
import star.rpc_promise;
import star.a_star;
import std;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.maybe;
import star.bi_map;
import star.color;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;

import star.drawable;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.non_rotated_drawables_item;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
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
import star.beam_item;
import star.status_effect_item;
import star.lua_components;
import star.fireable_item;
import star.swingable_item;
import star.durability_item;
import star.pointable_item;
import star.mixer;
import star.entity_rendering_types;
import star.animation;
import star.particle;
import star.light_source;
import star.entity_rendering;
struct PreviewTile;
import star.preview_tile_tool;

namespace Star {

class World;
class WireConnector;
class ToolUserEntity;

class MiningTool : public Item, public SwingableItem, public DurabilityItem {
public:
  MiningTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  List<Drawable> drawables() const override;
  // In pixels, offset from image center
  Vec2F handPosition() const override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  float durabilityStatus() override;

  float getAngle(float aimAngle) override;

private:
  void changeDurability(float amount);

  String m_image;
  int m_frames;
  float m_frameCycle;
  float m_frameTiming;
  List<String> m_animationFrame;
  String m_idleFrame;

  Vec2F m_handPosition;
  float m_blockRadius;
  float m_altBlockRadius;

  StringList m_strikeSounds;
  String m_breakSound;
  float m_toolVolume;
  float m_blockVolume;

  bool m_pointable;
};

class HarvestingTool : public Item, public SwingableItem {
public:
  HarvestingTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  List<Drawable> drawables() const override;
  // In pixels, offset from image center
  Vec2F handPosition() const override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  float getAngle(float aimAngle) override;

private:
  String m_image;
  int m_frames;
  float m_frameCycle;
  float m_frameTiming;
  List<String> m_animationFrame;
  String m_idleFrame;

  Vec2F m_handPosition;

  String m_idleSound;
  StringList m_strikeSounds;
  float m_toolVolume;
  float m_harvestPower;
};


class WireTool : public Item, public FireableItem, public PointableItem, public BeamItem {
public:
  WireTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  List<Drawable> drawables() const override;
  List<Drawable> nonRotatedDrawables() const override;

  void setEnd(EndType type) override;

  // In pixels, offset from image center
  Vec2F handPosition() const override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  float getAngle(float aimAngle) override;

  void setConnector(WireConnector* connector);

private:
  String m_image;
  Vec2F m_handPosition;

  StringList m_strikeSounds;
  float m_toolVolume;

  WireConnector* m_wireConnector;
};

class BeamMiningTool : public Item, public FireableItem, public PreviewTileTool, public PointableItem, public BeamItem {
public:
  BeamMiningTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  List<Drawable> drawables() const override;

  virtual void setEnd(EndType type) override;
  virtual List<PreviewTile> previewTiles(bool shifting) const override;
  virtual List<Drawable> nonRotatedDrawables() const override;
  virtual void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  float getAngle(float angle) override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  List<PersistentStatusEffect> statusEffects() const override;

private:
  float m_blockRadius;
  float m_altBlockRadius;

  float m_tileDamage;
  unsigned m_harvestLevel;
  bool m_canCollectLiquid;

  StringList m_strikeSounds;
  float m_toolVolume;
  float m_blockVolume;

  List<PersistentStatusEffect> m_inhandStatusEffects;
};


class PaintingBeamTool
  : public Item,
    public FireableItem,
    public PreviewTileTool,
    public PointableItem,
    public BeamItem {
public:
  PaintingBeamTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  List<Drawable> drawables() const override;

  void setEnd(EndType type) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  List<PreviewTile> previewTiles(bool shifting) const override;
  void init(ToolUserEntity* owner, ToolHand hand) override;
  List<Drawable> nonRotatedDrawables() const override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  float getAngle(float angle) override;

private:
  List<Color> m_colors;
  List<String> m_colorKeys;
  int m_colorIndex;

  float m_blockRadius;
  float m_altBlockRadius;

  StringList m_strikeSounds;
  float m_toolVolume;
  float m_blockVolume;
};

}

export module star.tools;

export namespace Star {
  using ::Star::World;
  using ::Star::WireConnector;
  using ::Star::ToolUserEntity;
  using ::Star::MiningTool;
  using ::Star::HarvestingTool;
  using ::Star::WireTool;
  using ::Star::BeamMiningTool;
  using ::Star::PaintingBeamTool;
}

export {
  using ::PreviewTile;
}
