#pragma once

#include "StarDungeonGenerator.hpp"
#include "StarAssets.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

namespace Dungeon {

  class ImagePartReader;
  class ImageTileset;
  using ImageTilesetConstPtr = SharedPtr<ImageTileset const>;

  class ImagePartReader : public PartReader {
  public:
    ImagePartReader(AssetsConstPtr assets, ImageTilesetConstPtr tileset) : m_assets(requireServiceValueAs<DungeonException>(std::move(assets), "ImagePartReader", "assets")), m_tileset(requireServiceValueAs<DungeonException>(std::move(tileset), "ImagePartReader", "image tileset")) {}

    virtual void readAsset(String const& asset) override;
    virtual Vec2U size() const override;

    virtual void forEachTile(TileCallback const& callback) const override;
    virtual void forEachTileAt(Vec2I pos, TileCallback const& callback) const override;

  private:
    AssetsConstPtr m_assets;
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
