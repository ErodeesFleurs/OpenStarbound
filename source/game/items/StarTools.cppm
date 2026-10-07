module;
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarSpline.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarConfig.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarGameTypes.hpp"
#include "StarDirectives.hpp"

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
STAR_STRUCT(PreviewTile);
STAR_CLASS(PreviewTileTool);
import star.preview_tile_tool;

namespace Star {

STAR_CLASS(World);
STAR_CLASS(WireConnector);
STAR_CLASS(ToolUserEntity);

STAR_CLASS(MiningTool);
STAR_CLASS(HarvestingTool);
STAR_CLASS(WireTool);
STAR_CLASS(BeamMiningTool);
STAR_CLASS(PaintingBeamTool);

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
