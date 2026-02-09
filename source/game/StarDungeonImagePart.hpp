#pragma once

#include "StarConfig.hpp"
#include "StarDungeonGenerator.hpp"
#include "StarImage.hpp"

import std;

namespace Star::Dungeon {

class ImageTileset;

class ImagePartReader : public PartReader {
public:
  ImagePartReader(ConstPtr<ImageTileset> tileset) : m_tileset(std::move(tileset)) {}

  void readAsset(String const& asset) override;
  [[nodiscard]] auto size() const -> Vec2U override;

  void forEachTile(TileCallback const& callback) const override;
  void forEachTileAt(Vec2I pos, TileCallback const& callback) const override;

private:
  List<std::pair<String, ConstPtr<Image>>> m_images;
  ConstPtr<ImageTileset> m_tileset;
};

class ImageTileset {
public:
  ImageTileset(Json const& tileset);

  [[nodiscard]] auto getTile(Vec4B color) const -> Tile const*;

private:
  [[nodiscard]] auto colorAsInt(Vec4B color) const -> unsigned;

  Map<unsigned, Tile> m_tiles;
};
}// namespace Star::Dungeon
