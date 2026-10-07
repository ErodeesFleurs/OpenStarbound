module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarRandom.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"
#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;
#include "StarCasting.hpp"
#include "StarImage.hpp"


import star.item_descriptor;
import star.world_geometry;
import star.dungeon_generator;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

export module star.dungeon_image_part;

export namespace Star::Dungeon {

STAR_CLASS(ImagePartReader);
STAR_CLASS(ImageTileset);

class ImagePartReader : public PartReader {
public:
  ImagePartReader(ImageTilesetConstPtr tileset) : m_tileset(tileset) {}

  virtual void readAsset(String const& asset) override;
  virtual Vec2U size() const override;

  virtual void forEachTile(TileCallback const& callback) const override;
  virtual void forEachTileAt(Vec2I pos, TileCallback const& callback) const override;

private:
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

namespace Star {

namespace Dungeon {

  void ImagePartReader::readAsset(String const& asset) {
    auto assets = Root::singleton().assets();
    m_images.push_back(make_pair(asset, assets->image(asset)));
  }

  Vec2U ImagePartReader::size() const {
    if (m_images.empty())
      return Vec2U{0, 0};
    return m_images[0].second->size();
  }

  void ImagePartReader::forEachTile(TileCallback const& callback) const {
    for (auto const& entry : m_images) {
      String const& file = entry.first;
      ImageConstPtr const& image = entry.second;
      for (size_t y = 0; y < image->height(); y++) {
        for (size_t x = 0; x < image->width(); x++) {

          Vec2I position(x, y);
          Vec4B tileColor = image->get(x, y);

          if (auto const& tile = m_tileset->getTile(tileColor)) {
            bool exitEarly = callback(position, *tile);
            if (exitEarly)
              return;
          } else {
            throw StarException::format("Dungeon image {} uses unknown tile color: #{:02x}{:02x}{:02x}{:02x}",
                file,
                tileColor[0],
                tileColor[1],
                tileColor[2],
                tileColor[3]);
          }
        }
      }
    }
  }

  void ImagePartReader::forEachTileAt(Vec2I pos, TileCallback const& callback) const {
    for (auto const& entry : m_images) {
      String const& file = entry.first;
      ImageConstPtr const& image = entry.second;
      Vec4B tileColor = image->get(pos.x(), pos.y());

      if (auto const& tile = m_tileset->getTile(tileColor)) {
        bool exitEarly = callback(pos, *tile);
        if (exitEarly)
          return;
      } else {
        throw StarException::format("Dungeon image {} uses unknown tile color: #{:02x}{:02x}{:02x}{:02x}",
            file,
            tileColor[0],
            tileColor[1],
            tileColor[2],
            tileColor[3]);
      }
    }
  }

  String connectorColorValue(Vec4B const& color) {
    return strf("{},{},{},{}", color[0], color[1], color[2], color[3]);
  }

  Tile variantMapToTile(JsonObject const& tile) {
    Tile result;
    if (tile.contains("brush"))
      result.brushes = Brush::readBrushes(tile.get("brush"));
    if (tile.contains("rules"))
      result.rules = Rule::readRules(tile.get("rules"));

    if (tile.contains("connector") && tile.get("connector").toBool()) {
      auto connector = TileConnector();

      connector.forwardOnly = tile.contains("connectForwardOnly") && tile.get("connectForwardOnly").toBool();

      Json connectorValue = tile.get("value");
      if (Maybe<Json> value = tile.maybe("connector-value"))
        connectorValue = value.take();

      if (connectorValue.isType(Json::Type::String))
        connector.value = connectorValue.toString();
      else
        connector.value = connectorColorValue(jsonToVec4B(connectorValue));

      if (tile.contains("direction"))
        connector.direction = DungeonDirectionNames.getLeft(tile.get("direction").toString());

      result.connector = connector;
    }

    return result;
  }

  ImageTileset::ImageTileset(Json const& tileset) {
    for (Json const& tileDef : tileset.iterateArray()) {
      Vec4B color = jsonToVec4B(tileDef.get("value"));
      m_tiles.insert(colorAsInt(color), variantMapToTile(tileDef.toObject()));
    }
  }

  Tile const* ImageTileset::getTile(Vec4B color) const {
    if (color[3] == 0)
      color = Vec4B(255, 255, 255, 0);
    if (color[3] != 255)
      return nullptr;
    return &m_tiles.get(colorAsInt(color));
  }

  unsigned ImageTileset::colorAsInt(Vec4B color) const {
    if ((color[3] != 0) && (color[3] != 255)) {
      starAssert(false);
      return 0;
    }
    if (color[3] == 0)
      color = Vec4B(255, 255, 255, 0);
    return (((unsigned)color[2]) << 16) | (((unsigned)color[1]) << 8) | ((unsigned)color[0]);
  }
}

}
