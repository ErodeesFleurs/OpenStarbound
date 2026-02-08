#pragma once

#include "StarBeamItem.hpp"
#include "StarConfig.hpp"
#include "StarDurabilityItem.hpp"
#include "StarFireableItem.hpp"
#include "StarItem.hpp"
#include "StarPointableItem.hpp"
#include "StarPreviewTileTool.hpp"
#include "StarSwingableItem.hpp"

namespace Star {

class WireConnector;

class MiningTool : public Item, public SwingableItem, public DurabilityItem {
public:
  MiningTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;
  // In pixels, offset from image center
  auto handPosition() const -> Vec2F override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto durabilityStatus() -> float override;

  auto getAngle(float aimAngle) -> float override;

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

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;
  // In pixels, offset from image center
  auto handPosition() const -> Vec2F override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  auto getAngle(float aimAngle) -> float override;

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

class Flashlight : public Item, public PointableItem, public ToolUserItem {
public:
  Flashlight(Json const& config, String const& directory, Json const& parameters = JsonObject());

  [[nodiscard]] auto clone() const -> Ptr<Item> override;

  [[nodiscard]] auto drawables() const -> List<Drawable> override;

  [[nodiscard]] auto lightSources() const -> List<LightSource>;

private:
  String m_image;
  Vec2F m_handPosition;
  Vec2F m_lightPosition;
  Color m_lightColor;
  float m_beamWidth;
  float m_ambientFactor;
};

class WireTool : public Item, public FireableItem, public PointableItem, public BeamItem {
public:
  WireTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  auto clone() const -> Ptr<Item> override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto drawables() const -> List<Drawable> override;
  auto nonRotatedDrawables() const -> List<Drawable> override;

  void setEnd(EndType type) override;

  // In pixels, offset from image center
  auto handPosition() const -> Vec2F override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  auto getAngle(float aimAngle) -> float override;

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

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;

  void setEnd(EndType type) override;
  auto previewTiles(bool shifting) const -> List<PreviewTile> override;
  auto nonRotatedDrawables() const -> List<Drawable> override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  auto getAngle(float angle) -> float override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto statusEffects() const -> List<PersistentStatusEffect> override;

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

class TillingTool : public Item, public SwingableItem {
public:
  TillingTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;
  // In pixels, offset from image center
  auto handPosition() const -> Vec2F override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  auto getAngle(float aimAngle) -> float override;

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
};

class PaintingBeamTool
    : public Item,
      public FireableItem,
      public PreviewTileTool,
      public PointableItem,
      public BeamItem {
public:
  PaintingBeamTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;

  void setEnd(EndType type) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  auto previewTiles(bool shifting) const -> List<PreviewTile> override;
  void init(ToolUserEntity* owner, ToolHand hand) override;
  auto nonRotatedDrawables() const -> List<Drawable> override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  auto getAngle(float angle) -> float override;

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

}// namespace Star
