#pragma once

#include "StarBeamItem.hpp"
#include "StarCollisionBlock.hpp"
#include "StarConfig.hpp"
#include "StarEntityRendering.hpp"
#include "StarFireableItem.hpp"
#include "StarItem.hpp"
#include "StarPreviewTileTool.hpp"
#include "StarPreviewableItem.hpp"
#include "StarRenderableItem.hpp"

import std;

namespace Star {

class MaterialItem : public Item, public FireableItem, public PreviewTileTool, public RenderableItem, public PreviewableItem, public BeamItem {
public:
  MaterialItem(Json const& config, String const& directory, Json const& settings);
  ~MaterialItem() override = default;

  auto clone() const -> Ptr<Item> override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void uninit() override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  void render(RenderCallback* renderCallback, EntityRenderLayer renderLayer) override;

  auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> override;
  auto dropDrawables() const -> List<Drawable> override;
  auto nonRotatedDrawables() const -> List<Drawable> override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void endFire(FireMode mode, bool shifting) override;

  auto materialId() const -> MaterialId;
  auto materialHueShift() const -> MaterialHue;

  auto canPlace(bool shifting) const -> bool;
  auto multiplaceEnabled() const -> bool;

  auto blockRadius() -> float&;
  auto altBlockRadius() -> float&;
  auto collisionOverride() -> TileCollisionOverride&;

  auto previewTiles(bool shifting) const -> List<PreviewTile> override;
  auto generatedPreview(Vec2I position = {}) const -> List<Drawable> const&;

private:
  auto blockSwap(float radius, TileLayer layer) -> size_t;
  void updatePropertiesFromPlayer(Player* player);
  auto calcRadius(bool shifting) const -> float;
  auto tileArea(float radius, Vec2F const& position) const -> List<Vec2I>&;
  auto placementHueShift(Vec2I const& position) const -> MaterialHue;

  MaterialId m_material;
  MaterialHue m_materialHueShift;

  float m_blockRadius;
  float m_altBlockRadius;
  bool m_blockSwap;
  bool m_shifting;
  bool m_multiplace;
  StringList m_placeSounds;
  std::optional<Vec2F> m_lastAimPosition;
  TileCollisionOverride m_collisionOverride;

  mutable Vec2F m_lastTileAreaOriginCache;
  mutable float m_lastTileAreaRadiusCache;
  mutable List<Vec2I> m_tileAreasCache;

  mutable std::optional<List<Drawable>> m_generatedPreviewCache;
};

}// namespace Star
