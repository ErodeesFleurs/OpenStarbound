#pragma once

#include "StarDungeonGenerator.hpp"
#include "StarIAssets.hpp"

namespace Star {

namespace Dungeon {

  class ImagePartReader;
  class ImageTileset;
  using ImageTilesetConstPtr = SharedPtr<ImageTileset const>;

  class ImagePartReader : public PartReader {
  public:
    ImagePartReader(IAssetsConstPtr assets, ImageTilesetConstPtr tileset) : m_assets(std::move(assets)), m_tileset(std::move(tileset)) {}

    virtual void readAsset(String const& asset) override;
    virtual Vec2U size() const override;

    virtual void forEachTile(TileCallback const& callback) const override;
    virtual void forEachTileAt(Vec2I pos, TileCallback const& callback) const override;

  private:
    IAssetsConstPtr m_assets;
    List<pair<String, ImageConstPtr>> m_images;
    ImageTilesetConstPtr m_tileset;
  };

  class ImageTileset {
  public:
    ImageTileset(Json const& tileset);

    Tile const* getTile(Vec4B color) const;

  private:
    unsigned colorAsInt(Vec4B color) const;

    Map<unsigned, Tile> m_tiles;
  };
}
}
