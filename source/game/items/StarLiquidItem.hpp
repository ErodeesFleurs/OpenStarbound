#pragma once

#include "StarItem.hpp"
#include "StarFireableItem.hpp"
#include "StarBeamItem.hpp"
#include "StarEntityRendering.hpp"
#include "StarPreviewTileTool.hpp"
#include "StarAssets.hpp"

namespace Star {

class LiquidsDatabase;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;
class LiquidItem;

class LiquidItem : public Item, public FireableItem, public PreviewTileTool, public BeamItem {
public:
  LiquidItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& settings, LiquidsDatabaseConstPtr liquidsDatabase);
  virtual ~LiquidItem() = default;

  [[nodiscard]] ItemPtr clone() const override;

  void init(ToolUserEntity& owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  [[nodiscard]] List<Drawable> nonRotatedDrawables() const override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  [[nodiscard]] LiquidId liquidId() const;
  [[nodiscard]] float liquidQuantity() const;

  [[nodiscard]] List<PreviewTile> previewTiles(bool shifting) const override;

  [[nodiscard]] bool canPlace(bool shifting) const;
  [[nodiscard]] bool canPlaceAtTile(Vec2I pos) const;
  [[nodiscard]] bool multiplaceEnabled() const;

private:
  LiquidId m_liquidId;
  float m_quantity;

  float m_blockRadius;
  float m_altBlockRadius;
  bool m_shifting;
};

}
