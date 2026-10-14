module;
import star.json;
import star.id_map;
import star.vector;
import star.spline;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
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
import star.beam_item;
import star.status_effect_item;
#include "StarLuaComponents.hpp"
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
using PreviewTilePtr = std::shared_ptr<PreviewTile>;
using PreviewTileConstPtr = std::shared_ptr<const PreviewTile>;
using PreviewTileWeakPtr = std::weak_ptr<PreviewTile>;
using PreviewTileConstWeakPtr = std::weak_ptr<const PreviewTile>;
using PreviewTileUPtr = std::unique_ptr<PreviewTile>;
using PreviewTileConstUPtr = std::unique_ptr<const PreviewTile>;
class PreviewTileTool;
using PreviewTileToolPtr = std::shared_ptr<PreviewTileTool>;
using PreviewTileToolConstPtr = std::shared_ptr<const PreviewTileTool>;
using PreviewTileToolWeakPtr = std::weak_ptr<PreviewTileTool>;
using PreviewTileToolConstWeakPtr = std::weak_ptr<const PreviewTileTool>;
using PreviewTileToolUPtr = std::unique_ptr<PreviewTileTool>;
using PreviewTileToolConstUPtr = std::unique_ptr<const PreviewTileTool>;
import star.preview_tile_tool;

namespace Star {

class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
class WireConnector;
using WireConnectorPtr = std::shared_ptr<WireConnector>;
using WireConnectorConstPtr = std::shared_ptr<const WireConnector>;
using WireConnectorWeakPtr = std::weak_ptr<WireConnector>;
using WireConnectorConstWeakPtr = std::weak_ptr<const WireConnector>;
using WireConnectorUPtr = std::unique_ptr<WireConnector>;
using WireConnectorConstUPtr = std::unique_ptr<const WireConnector>;
class ToolUserEntity;
using ToolUserEntityPtr = std::shared_ptr<ToolUserEntity>;
using ToolUserEntityConstPtr = std::shared_ptr<const ToolUserEntity>;
using ToolUserEntityWeakPtr = std::weak_ptr<ToolUserEntity>;
using ToolUserEntityConstWeakPtr = std::weak_ptr<const ToolUserEntity>;
using ToolUserEntityUPtr = std::unique_ptr<ToolUserEntity>;
using ToolUserEntityConstUPtr = std::unique_ptr<const ToolUserEntity>;

class MiningTool;
using MiningToolPtr = std::shared_ptr<MiningTool>;
using MiningToolConstPtr = std::shared_ptr<const MiningTool>;
using MiningToolWeakPtr = std::weak_ptr<MiningTool>;
using MiningToolConstWeakPtr = std::weak_ptr<const MiningTool>;
using MiningToolUPtr = std::unique_ptr<MiningTool>;
using MiningToolConstUPtr = std::unique_ptr<const MiningTool>;
class HarvestingTool;
using HarvestingToolPtr = std::shared_ptr<HarvestingTool>;
using HarvestingToolConstPtr = std::shared_ptr<const HarvestingTool>;
using HarvestingToolWeakPtr = std::weak_ptr<HarvestingTool>;
using HarvestingToolConstWeakPtr = std::weak_ptr<const HarvestingTool>;
using HarvestingToolUPtr = std::unique_ptr<HarvestingTool>;
using HarvestingToolConstUPtr = std::unique_ptr<const HarvestingTool>;
class WireTool;
using WireToolPtr = std::shared_ptr<WireTool>;
using WireToolConstPtr = std::shared_ptr<const WireTool>;
using WireToolWeakPtr = std::weak_ptr<WireTool>;
using WireToolConstWeakPtr = std::weak_ptr<const WireTool>;
using WireToolUPtr = std::unique_ptr<WireTool>;
using WireToolConstUPtr = std::unique_ptr<const WireTool>;
class BeamMiningTool;
using BeamMiningToolPtr = std::shared_ptr<BeamMiningTool>;
using BeamMiningToolConstPtr = std::shared_ptr<const BeamMiningTool>;
using BeamMiningToolWeakPtr = std::weak_ptr<BeamMiningTool>;
using BeamMiningToolConstWeakPtr = std::weak_ptr<const BeamMiningTool>;
using BeamMiningToolUPtr = std::unique_ptr<BeamMiningTool>;
using BeamMiningToolConstUPtr = std::unique_ptr<const BeamMiningTool>;
class PaintingBeamTool;
using PaintingBeamToolPtr = std::shared_ptr<PaintingBeamTool>;
using PaintingBeamToolConstPtr = std::shared_ptr<const PaintingBeamTool>;
using PaintingBeamToolWeakPtr = std::weak_ptr<PaintingBeamTool>;
using PaintingBeamToolConstWeakPtr = std::weak_ptr<const PaintingBeamTool>;
using PaintingBeamToolUPtr = std::unique_ptr<PaintingBeamTool>;
using PaintingBeamToolConstUPtr = std::unique_ptr<const PaintingBeamTool>;

class MiningTool : public Item, public SwingableItem, public DurabilityItem {
public:
  MiningTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  ItemPtr clone() const override;

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

  ItemPtr clone() const override;

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

  ItemPtr clone() const override;

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

  ItemPtr clone() const override;

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

  ItemPtr clone() const override;

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
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::WireConnector;
  using ::Star::WireConnectorPtr;
  using ::Star::WireConnectorConstPtr;
  using ::Star::WireConnectorWeakPtr;
  using ::Star::WireConnectorConstWeakPtr;
  using ::Star::WireConnectorUPtr;
  using ::Star::WireConnectorConstUPtr;
  using ::Star::ToolUserEntity;
  using ::Star::ToolUserEntityPtr;
  using ::Star::ToolUserEntityConstPtr;
  using ::Star::ToolUserEntityWeakPtr;
  using ::Star::ToolUserEntityConstWeakPtr;
  using ::Star::ToolUserEntityUPtr;
  using ::Star::ToolUserEntityConstUPtr;
  using ::Star::MiningTool;
  using ::Star::MiningToolPtr;
  using ::Star::MiningToolConstPtr;
  using ::Star::MiningToolWeakPtr;
  using ::Star::MiningToolConstWeakPtr;
  using ::Star::MiningToolUPtr;
  using ::Star::MiningToolConstUPtr;
  using ::Star::HarvestingTool;
  using ::Star::HarvestingToolPtr;
  using ::Star::HarvestingToolConstPtr;
  using ::Star::HarvestingToolWeakPtr;
  using ::Star::HarvestingToolConstWeakPtr;
  using ::Star::HarvestingToolUPtr;
  using ::Star::HarvestingToolConstUPtr;
  using ::Star::WireTool;
  using ::Star::WireToolPtr;
  using ::Star::WireToolConstPtr;
  using ::Star::WireToolWeakPtr;
  using ::Star::WireToolConstWeakPtr;
  using ::Star::WireToolUPtr;
  using ::Star::WireToolConstUPtr;
  using ::Star::BeamMiningTool;
  using ::Star::BeamMiningToolPtr;
  using ::Star::BeamMiningToolConstPtr;
  using ::Star::BeamMiningToolWeakPtr;
  using ::Star::BeamMiningToolConstWeakPtr;
  using ::Star::BeamMiningToolUPtr;
  using ::Star::BeamMiningToolConstUPtr;
  using ::Star::PaintingBeamTool;
  using ::Star::PaintingBeamToolPtr;
  using ::Star::PaintingBeamToolConstPtr;
  using ::Star::PaintingBeamToolWeakPtr;
  using ::Star::PaintingBeamToolConstWeakPtr;
  using ::Star::PaintingBeamToolUPtr;
  using ::Star::PaintingBeamToolConstUPtr;
}

export {
  using ::PreviewTile;
  using ::PreviewTilePtr;
  using ::PreviewTileConstPtr;
  using ::PreviewTileWeakPtr;
  using ::PreviewTileConstWeakPtr;
  using ::PreviewTileUPtr;
  using ::PreviewTileConstUPtr;
  using ::PreviewTileTool;
  using ::PreviewTileToolPtr;
  using ::PreviewTileToolConstPtr;
  using ::PreviewTileToolWeakPtr;
  using ::PreviewTileToolConstWeakPtr;
  using ::PreviewTileToolUPtr;
  using ::PreviewTileToolConstUPtr;
}
