#pragma once

#include "StarObserverPtr.hpp"
#include "StarRect.hpp"
#include "StarDungeonGenerator.hpp"
#include "StarTilesetDatabase.hpp"
#include "StarLexicalCast.hpp"
#include "StarAssets.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

namespace Dungeon {
  class TMXTilesets;
  using TMXTilesetsPtr = SharedPtr<TMXTilesets>;
  class TMXTileLayer;
  using TMXTileLayerPtr = SharedPtr<TMXTileLayer>;
  class TMXObject;
  using TMXObjectPtr = SharedPtr<TMXObject>;
  class TMXObjectGroup;
  using TMXObjectGroupPtr = SharedPtr<TMXObjectGroup>;
  class TMXMap;
  using TMXMapConstPtr = SharedPtr<TMXMap const>;

  class TMXTilesets {
  public:
    TMXTilesets(Json const& tmx, TilesetDatabaseConstPtr tilesetDatabase);

    Tiled::Tile const& getTile(unsigned gid, TileLayer layer) const;

    Tiled::Tile const& nullTile() const {
      return *m_nullTile;
    }

  private:
    struct TilesetInfo {
      Tiled::Tileset const* tileset;
      size_t firstGid;
      size_t lastGid;
    };

    [[nodiscard]] static bool tilesetComparator(TilesetInfo const& a, TilesetInfo const& b);

    // The default empty background tile has clear=true.  (If you use the pink
    // tile in the background, clear will be false instead.) Analogous to
    // EmptyMaterialId.
    Tiled::TileConstPtr m_emptyBackTile;
    // The default foreground tile doesn't have a 'clear' property.  Also
    // returned by tile layers when given coordinates outside the bounds of the
    // layer.  Analogous to the NullMaterialId that mission maps are initially
    // filled with.
    Tiled::TileConstPtr m_nullTile;

    List<Tiled::TilesetConstPtr> m_tilesets;
    List<observer_ptr<Tiled::Tile const>> m_foregroundTilesByGid;
    List<observer_ptr<Tiled::Tile const>> m_backgroundTilesByGid;
  };

  class TMXTileLayer {
  public:
    TMXTileLayer(Json const& tmx);

    Tiled::Tile const& getTile(TMXTilesetsPtr const& tilesets, Vec2I pos) const;

    unsigned width() const {
      return m_rect.xMax() - m_rect.xMin() + 1;
    }
    unsigned height() const {
      return m_rect.yMax() - m_rect.yMin() + 1;
    }

    RectI const& rect() const {
      return m_rect;
    }
    String const& name() const {
      return m_name;
    }

    TileLayer layer() const {
      return m_layer;
    }

    [[nodiscard]] bool forEachTile(TMXMap const* map, TileCallback const& callback) const;
    [[nodiscard]] bool forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const;

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
    TMXObject(Maybe<Json> const& groupProperties, Json const& tmx, TMXTilesetsPtr tilesets);

    Vec2I const& pos() const {
      return m_rect.min();
    }
    RectI const& rect() const {
      return m_rect;
    }
    Tiled::Tile const& tile() const {
      return *m_tile;
    }
    ObjectKind kind() const {
      return m_kind;
    }

    [[nodiscard]] bool forEachTile(TMXMap const* map, TileCallback const& callback) const;
    [[nodiscard]] bool forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const;

  private:
    // "Tile Objects" in Tiled are objects that contain an image from a tileset,
    // and have a bunch of their own Tile Object-specific properties.
    struct TileObjectInfo {
      Tiled::Properties tileProperties;
      unsigned flipBits;
    };

    [[nodiscard]] static Vec2I getSize(Json const& tmx);
    [[nodiscard]] static Vec2I getImagePosition(Tiled::Properties const& properties);
    [[nodiscard]] static ObjectKind getObjectKind(Json const& tmx, Maybe<Json >const& objectProperties);
    [[nodiscard]] static Maybe<TileObjectInfo> getTileObjectInfo(Json const& tmx, TMXTilesetsPtr tilesets, TileLayer layer);
    [[nodiscard]] static TileLayer getLayer(Maybe<Json> const& groupProperties, Maybe<Json> const& objectProperties);

    [[nodiscard]] static Vec2I getPos(Json const& tmx);
    [[nodiscard]] static StarException tmxObjectError(Json const& tmx, String const& msg);

    RectI m_rect;
    Tiled::TileConstPtr m_tile;
    TileLayer m_layer;
    ObjectKind m_kind;
    unsigned m_objectId;
    List<Vec2I> m_polyline;
  };

  class TMXObjectGroup {
  public:
    TMXObjectGroup(Json const& tmx, TMXTilesetsPtr tilesets);

    List<TMXObjectPtr> const& objects() const {
      return m_objects;
    }

    [[nodiscard]] String name() const;

    [[nodiscard]] bool forEachTile(TMXMap const* map, TileCallback const& callback) const;
    [[nodiscard]] bool forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const;

  private:
    String m_name;
    List<TMXObjectPtr> m_objects;
  };

  class TMXMap {
  public:
    TMXMap(Json const& tmx, TilesetDatabaseConstPtr tilesetDatabase);

    List<TMXTileLayerPtr> const& tileLayers() const {
      return m_tileLayers;
    }
    List<TMXObjectGroupPtr> const& objectGroups() const {
      return m_objectGroups;
    }
    TMXTilesetsPtr const& tilesets() const {
      return m_tilesets;
    }
    unsigned width() const {
      return m_width;
    }
    unsigned height() const {
      return m_height;
    }

    [[nodiscard]] bool forEachTile(TileCallback const& callback) const;
    [[nodiscard]] bool forEachTileAt(Vec2I pos, TileCallback const& callback) const;

  private:
    List<TMXTileLayerPtr> m_tileLayers;
    List<TMXObjectGroupPtr> m_objectGroups;

    TMXTilesetsPtr m_tilesets;
    unsigned m_width, m_height;
  };

  class TMXPartReader : public PartReader {
  public:
    explicit TMXPartReader(AssetsConstPtr assets, TilesetDatabaseConstPtr tilesetDatabase) : m_assets(requireServiceValueAs<DungeonException>(std::move(assets), "TMXPartReader", "assets")), m_tilesetDatabase(requireServiceValueAs<DungeonException>(std::move(tilesetDatabase), "TMXPartReader", "tileset database")) {}

    void readAsset(String const& asset) override;

    [[nodiscard]] Vec2U size() const override;

    void forEachTile(TileCallback const& callback) const override;
    void forEachTileAt(Vec2I pos, TileCallback const& callback) const override;

  private:
    // Return true in the callback to exit early without processing later maps
    void forEachMap(function<bool(TMXMapConstPtr const&)> func) const;

    AssetsConstPtr m_assets;
    TilesetDatabaseConstPtr m_tilesetDatabase;
    List<pair<String, TMXMapConstPtr>> m_maps;
  };
}

}
