#pragma once

#include "StarBeamItem.hpp"
#include "StarConfig.hpp"
#include "StarFireableItem.hpp"
#include "StarItem.hpp"
#include "StarPreviewTileTool.hpp"

namespace Star {

class LiquidItem : public Item, public FireableItem, public PreviewTileTool, public BeamItem {
public:
  LiquidItem(Json const& config, String const& directory, Json const& settings);
  ~LiquidItem() override = default;

  auto clone() const -> Ptr<Item> override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto nonRotatedDrawables() const -> List<Drawable> override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  auto liquidId() const -> LiquidId;
  auto liquidQuantity() const -> float;

  auto previewTiles(bool shifting) const -> List<PreviewTile> override;

  auto canPlace(bool shifting) const -> bool;
  auto canPlaceAtTile(Vec2I pos) const -> bool;
  auto multiplaceEnabled() const -> bool;

private:
  LiquidId m_liquidId;
  float m_quantity;

  float m_blockRadius;
  float m_altBlockRadius;
  bool m_shifting;
};

}// namespace Star
