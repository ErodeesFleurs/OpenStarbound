#pragma once

#include "StarItem.hpp"
#include "StarFireableItem.hpp"
#include "StarBeamItem.hpp"
#include "StarEntityRendering.hpp"
#include "StarPreviewTileTool.hpp"
#include "StarRenderableItem.hpp"
#include "StarPreviewableItem.hpp"
#include "StarCollisionBlock.hpp"
#include "StarAssets.hpp"

namespace Star {

class MaterialDatabase;
using MaterialDatabaseConstPtr = SharedPtr<MaterialDatabase const>;

class MaterialItem;
class Player;
using PlayerPtr = SharedPtr<Player>;

class MaterialItem : public Item, public FireableItem, public PreviewTileTool, public RenderableItem, public PreviewableItem, public BeamItem {
public:
  MaterialItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& settings, MaterialDatabaseConstPtr materialDatabase);
  virtual ~MaterialItem() = default;

  [[nodiscard]] ItemPtr clone() const override;

  void init(ToolUserEntity& owner, ToolHand hand) override;
  void uninit() override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  void render(RenderCallback* renderCallback, EntityRenderLayer renderLayer) override;

  [[nodiscard]] List<Drawable> preview(PlayerPtr const& viewer = {}) const override;
  [[nodiscard]] List<Drawable> dropDrawables() const override;
  [[nodiscard]] List<Drawable> nonRotatedDrawables() const override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void endFire(FireMode mode, bool shifting) override;

  [[nodiscard]] MaterialId materialId() const;
  [[nodiscard]] MaterialHue materialHueShift() const;

  [[nodiscard]] bool canPlace(bool shifting) const;
  [[nodiscard]] bool multiplaceEnabled() const;

  [[nodiscard]] float& blockRadius();
  [[nodiscard]] float& altBlockRadius();
  [[nodiscard]] TileCollisionOverride& collisionOverride();

  [[nodiscard]] List<PreviewTile> previewTiles(bool shifting) const override;
  [[nodiscard]] List<Drawable> const& generatedPreview(Vec2I position = {}) const;
private:
  [[nodiscard]] size_t blockSwap(float radius, TileLayer layer);
  void updatePropertiesFromPlayer(Player& player);
  [[nodiscard]] float calcRadius(bool shifting) const;
  [[nodiscard]] List<Vec2I>& tileArea(float radius, Vec2F const& position) const;
  [[nodiscard]] MaterialHue placementHueShift(Vec2I const& position) const;

  AssetsConstPtr m_assets;
  MaterialId m_material;
  MaterialHue m_materialHueShift;

  float m_blockRadius;
  float m_altBlockRadius;
  bool m_blockSwap;
  bool m_shifting;
  bool m_multiplace;
  StringList m_placeSounds;
  Maybe<Vec2F> m_lastAimPosition;
  TileCollisionOverride m_collisionOverride;

  mutable Vec2F m_lastTileAreaOriginCache;
  mutable float m_lastTileAreaRadiusCache;
  mutable List<Vec2I> m_tileAreasCache;

  mutable Maybe<List<Drawable>> m_generatedPreviewCache;
};

}
