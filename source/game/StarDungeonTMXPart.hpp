#pragma once

#include "StarConfig.hpp"
#include "StarDungeonGenerator.hpp"
#include "StarRect.hpp"
#include "StarTilesetDatabase.hpp"

import std;

namespace Star::Dungeon {

class TMXMap;

class TMXTilesets {
public:
  TMXTilesets(Json const& tmx);

  [[nodiscard]] auto getTile(unsigned gid, TileLayer layer) const -> Tiled::Tile const&;

  [[nodiscard]] auto nullTile() const -> Tiled::Tile const& {
    return *m_nullTile;
  }

private:
  struct TilesetInfo {
    Tiled::Tileset const* tileset;
    std::size_t firstGid;
    std::size_t lastGid;
  };

  static auto tilesetComparator(TilesetInfo const& a, TilesetInfo const& b) -> bool;

  // The default empty background tile has clear=true.  (If you use the pink
  // tile in the background, clear will be false instead.) Analogous to
  // EmptyMaterialId.
  ConstPtr<Tiled::Tile> m_emptyBackTile;
  // The default foreground tile doesn't have a 'clear' property.  Also
  // returned by tile layers when given coordinates outside the bounds of the
  // layer.  Analogous to the NullMaterialId that mission maps are initially
  // filled with.
  ConstPtr<Tiled::Tile> m_nullTile;

  List<ConstPtr<Tiled::Tileset>> m_tilesets;
  List<Tiled::Tile const*> m_foregroundTilesByGid;
  List<Tiled::Tile const*> m_backgroundTilesByGid;
};

class TMXTileLayer {
public:
  TMXTileLayer(Json const& tmx);

  [[nodiscard]] auto getTile(Ptr<TMXTilesets> const& tilesets, Vec2I pos) const -> Tiled::Tile const&;

  [[nodiscard]] auto width() const -> unsigned {
    return m_rect.xMax() - m_rect.xMin() + 1;
  }
  [[nodiscard]] auto height() const -> unsigned {
    return m_rect.yMax() - m_rect.yMin() + 1;
  }

  [[nodiscard]] auto rect() const -> RectI const& {
    return m_rect;
  }
  [[nodiscard]] auto name() const -> String const& {
    return m_name;
  }

  [[nodiscard]] auto layer() const -> TileLayer {
    return m_layer;
  }

  auto forEachTile(TMXMap const* map, TileCallback const& callback) const -> bool;
  auto forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const -> bool;

private:
  RectI m_rect;
  String m_name;
  TileLayer m_layer;
  List<unsigned> m_tileData;
};

enum class ObjectKind {
  Tile,
  Rectangle,
  Ellipse,
  Polygon,
  Polyline,
  Stagehand
};

enum TileFlip {
  Horizontal = 0x80000000u,
  Vertical = 0x40000000u,
  Diagonal = 0x20000000u,
  AllBits = 0xe0000000u
};

class TMXObject {
public:
  TMXObject(std::optional<Json> const& groupProperties, Json const& tmx, Ptr<TMXTilesets> tilesets);

  [[nodiscard]] auto pos() const -> Vec2I const& {
    return m_rect.min();
  }
  [[nodiscard]] auto rect() const -> RectI const& {
    return m_rect;
  }
  [[nodiscard]] auto tile() const -> Tiled::Tile const& {
    return *m_tile;
  }
  [[nodiscard]] auto kind() const -> ObjectKind {
    return m_kind;
  }

  auto forEachTile(TMXMap const* map, TileCallback const& callback) const -> bool;
  auto forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const -> bool;

private:
  // "Tile Objects" in Tiled are objects that contain an image from a tileset,
  // and have a bunch of their own Tile Object-specific properties.
  struct TileObjectInfo {
    Tiled::Properties tileProperties;
    unsigned flipBits;
  };

  static auto getSize(Json const& tmx) -> Vec2I;
  static auto getImagePosition(Tiled::Properties const& properties) -> Vec2I;
  static auto getObjectKind(Json const& tmx, std::optional<Json> const& objectProperties) -> ObjectKind;
  static auto getTileObjectInfo(Json const& tmx, Ptr<TMXTilesets> tilesets, TileLayer layer) -> std::optional<TileObjectInfo>;
  static auto getLayer(std::optional<Json> const& groupProperties, std::optional<Json> const& objectProperties) -> TileLayer;

  static auto getPos(Json const& tmx) -> Vec2I;
  static auto tmxObjectError(Json const& tmx, String const& msg) -> StarException;

  RectI m_rect;
  ConstPtr<Tiled::Tile> m_tile;
  TileLayer m_layer;
  ObjectKind m_kind;
  unsigned m_objectId;
  List<Vec2I> m_polyline;
};

class TMXObjectGroup {
public:
  TMXObjectGroup(Json const& tmx, Ptr<TMXTilesets> tilesets);

  [[nodiscard]] auto objects() const -> List<Ptr<TMXObject>> const& {
    return m_objects;
  }

  [[nodiscard]] auto name() const -> String;

  auto forEachTile(TMXMap const* map, TileCallback const& callback) const -> bool;
  auto forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const -> bool;

private:
  String m_name;
  List<Ptr<TMXObject>> m_objects;
};

class TMXMap {
public:
  TMXMap(Json const& tmx);

  [[nodiscard]] auto tileLayers() const -> List<Ptr<TMXTileLayer>> const& {
    return m_tileLayers;
  }
  [[nodiscard]] auto objectGroups() const -> List<Ptr<TMXObjectGroup>> const& {
    return m_objectGroups;
  }
  [[nodiscard]] auto tilesets() const -> Ptr<TMXTilesets> const& {
    return m_tilesets;
  }
  [[nodiscard]] auto width() const -> unsigned {
    return m_width;
  }
  [[nodiscard]] auto height() const -> unsigned {
    return m_height;
  }

  [[nodiscard]] auto forEachTile(TileCallback const& callback) const -> bool;
  [[nodiscard]] auto forEachTileAt(Vec2I pos, TileCallback const& callback) const -> bool;

private:
  List<Ptr<TMXTileLayer>> m_tileLayers;
  List<Ptr<TMXObjectGroup>> m_objectGroups;

  Ptr<TMXTilesets> m_tilesets;
  unsigned m_width, m_height;
};

class TMXPartReader : public PartReader {
public:
  void readAsset(String const& asset) override;

  [[nodiscard]] auto size() const -> Vec2U override;

  void forEachTile(TileCallback const& callback) const override;
  void forEachTileAt(Vec2I pos, TileCallback const& callback) const override;

private:
  // Return true in the callback to exit early without processing later maps
  void forEachMap(std::function<bool(ConstPtr<TMXMap> const&)> func) const;

  List<std::pair<String, ConstPtr<TMXMap>>> m_maps;
};
}// namespace Star::Dungeon
