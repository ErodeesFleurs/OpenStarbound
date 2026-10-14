module;
import std;
import star.string;
import star.list;
import star.maybe;
import star.format;
import star.json;
import star.rect;
import star.image;
import star.data_stream;
import star.bi_map;
import star.vector;
import star.game_types;
import star.material_types;
import star.random;
import star.set;
import star.thread;
import star.ordered_map;
import star.lru_cache;


import star.item_descriptor;
import star.world_geometry;
import star.dungeon_generator;

export module star.dungeon_tmx_part;
import star.tileset_database;

export namespace Star {

namespace Dungeon {
  class TMXTilesets;
  using TMXTilesetsPtr = std::shared_ptr<TMXTilesets>;
  using TMXTilesetsConstPtr = std::shared_ptr<const TMXTilesets>;
  using TMXTilesetsWeakPtr = std::weak_ptr<TMXTilesets>;
  using TMXTilesetsConstWeakPtr = std::weak_ptr<const TMXTilesets>;
  using TMXTilesetsUPtr = std::unique_ptr<TMXTilesets>;
  using TMXTilesetsConstUPtr = std::unique_ptr<const TMXTilesets>;
  class TMXTileLayer;
  using TMXTileLayerPtr = std::shared_ptr<TMXTileLayer>;
  using TMXTileLayerConstPtr = std::shared_ptr<const TMXTileLayer>;
  using TMXTileLayerWeakPtr = std::weak_ptr<TMXTileLayer>;
  using TMXTileLayerConstWeakPtr = std::weak_ptr<const TMXTileLayer>;
  using TMXTileLayerUPtr = std::unique_ptr<TMXTileLayer>;
  using TMXTileLayerConstUPtr = std::unique_ptr<const TMXTileLayer>;
  class TMXObject;
  using TMXObjectPtr = std::shared_ptr<TMXObject>;
  using TMXObjectConstPtr = std::shared_ptr<const TMXObject>;
  using TMXObjectWeakPtr = std::weak_ptr<TMXObject>;
  using TMXObjectConstWeakPtr = std::weak_ptr<const TMXObject>;
  using TMXObjectUPtr = std::unique_ptr<TMXObject>;
  using TMXObjectConstUPtr = std::unique_ptr<const TMXObject>;
  class TMXObjectGroup;
  using TMXObjectGroupPtr = std::shared_ptr<TMXObjectGroup>;
  using TMXObjectGroupConstPtr = std::shared_ptr<const TMXObjectGroup>;
  using TMXObjectGroupWeakPtr = std::weak_ptr<TMXObjectGroup>;
  using TMXObjectGroupConstWeakPtr = std::weak_ptr<const TMXObjectGroup>;
  using TMXObjectGroupUPtr = std::unique_ptr<TMXObjectGroup>;
  using TMXObjectGroupConstUPtr = std::unique_ptr<const TMXObjectGroup>;
  class TMXMap;
  using TMXMapPtr = std::shared_ptr<TMXMap>;
  using TMXMapConstPtr = std::shared_ptr<const TMXMap>;
  using TMXMapWeakPtr = std::weak_ptr<TMXMap>;
  using TMXMapConstWeakPtr = std::weak_ptr<const TMXMap>;
  using TMXMapUPtr = std::unique_ptr<TMXMap>;
  using TMXMapConstUPtr = std::unique_ptr<const TMXMap>;

  class TMXTilesets {
  public:
    TMXTilesets(Json const& tmx);

    Tiled::Tile const& getTile(unsigned gid, TileLayer layer) const;

    Tiled::Tile const& nullTile() const {
      return *m_nullTile;
    }

  private:
    struct TilesetInfo {
      Tiled::Tileset const* tileset;
      std::size_t firstGid;
      std::size_t lastGid;
    };

    static bool tilesetComparator(TilesetInfo const& a, TilesetInfo const& b);

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
    List<Tiled::Tile const*> m_foregroundTilesByGid;
    List<Tiled::Tile const*> m_backgroundTilesByGid;
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

    bool forEachTile(TMXMap const* map, TileCallback const& callback) const;
    bool forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const;

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

    bool forEachTile(TMXMap const* map, TileCallback const& callback) const;
    bool forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const;

  private:
    // "Tile Objects" in Tiled are objects that contain an image from a tileset,
    // and have a bunch of their own Tile Object-specific properties.
    struct TileObjectInfo {
      Tiled::Properties tileProperties;
      unsigned flipBits;
    };

    static Vec2I getSize(Json const& tmx);
    static Vec2I getImagePosition(Tiled::Properties const& properties);
    static ObjectKind getObjectKind(Json const& tmx, Maybe<Json >const& objectProperties);
    static Maybe<TileObjectInfo> getTileObjectInfo(Json const& tmx, TMXTilesetsPtr tilesets, TileLayer layer);
    static TileLayer getLayer(Maybe<Json> const& groupProperties, Maybe<Json> const& objectProperties);

    static Vec2I getPos(Json const& tmx);
    static StarException tmxObjectError(Json const& tmx, String const& msg);

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

    String name() const;

    bool forEachTile(TMXMap const* map, TileCallback const& callback) const;
    bool forEachTileAt(Vec2I pos, TMXMap const* map, TileCallback const& callback) const;

  private:
    String m_name;
    List<TMXObjectPtr> m_objects;
  };

  class TMXMap {
  public:
    TMXMap(Json const& tmx);

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

    bool forEachTile(TileCallback const& callback) const;
    bool forEachTileAt(Vec2I pos, TileCallback const& callback) const;

  private:
    List<TMXTileLayerPtr> m_tileLayers;
    List<TMXObjectGroupPtr> m_objectGroups;

    TMXTilesetsPtr m_tilesets;
    unsigned m_width, m_height;
  };

  class TMXPartReader : public PartReader {
  public:
    virtual void readAsset(String const& asset) override;

    virtual Vec2U size() const override;

    virtual void forEachTile(TileCallback const& callback) const override;
    virtual void forEachTileAt(Vec2I pos, TileCallback const& callback) const override;

  private:
    // Return true in the callback to exit early without processing later maps
    void forEachMap(std::function<bool(TMXMapConstPtr const&)> func) const;

    List<std::pair<String, TMXMapConstPtr>> m_maps;
  };
}

}

