module;
import std;
import star.json;
import star.string;
import star.list;
import star.map;
import star.set;
import star.image;

import star.tileset_database;

namespace Star {

struct Tile {
  String source, database, tileset, name;
  std::shared_ptr<Image const> image;
  Tiled::Properties properties;
};

class TileDatabase {
public:
  TileDatabase(String const& name);

  void defineTile(std::shared_ptr<Tile> const& tile);
  std::shared_ptr<Tile> getTile(String const& tileName) const;
  String name() const;
  StringSet tileNames() const;

private:
  Map<String, std::shared_ptr<Tile>> m_tiles;
  String m_name;
};

class Tileset {
public:
  Tileset(String const& source, String const& name, std::shared_ptr<TileDatabase> const& database);

  void defineTile(std::shared_ptr<Tile> const& tile);
  void exportTileset() const;

  String name() const;
  std::shared_ptr<TileDatabase> database() const;

private:
  String imageDirName(String const& baseExportDir) const;
  String relativePathBase() const;
  Json imageFileReference(String const& fileName) const;
  Json tileImageReference(String const& tileName, String const& database) const;

  // Exports an image for each tile into its own file. Tiles can represent
  // objects with all different sizes, so we use Tiled's "collection of images"
  // tileset feature, which puts each image in its own file.
  void exportTilesetImages(String const& exportDir) const;

  // Read the tileset from the given path, or create a new tileset root
  // structure if it doesn't already exist.
  Json getTilesetJson(String const& tilesetPath) const;

  // Determine which tiles already exist in the tileset, returning a map
  // which contains the id of each named tile, and the next available Id after
  // the highest Id seen in the tileset.
  std::pair<StringMap<std::size_t>, std::size_t> indexExistingTiles(Json tileset) const;

  // Update existing and insert new tile definitions in the tileProperties and
  // tileImages objects.
  StringSet updateTiles(JsonObject& tileProperties,
      JsonObject& tileImages,
      StringMap<std::size_t> const& existingTiles,
      std::size_t& nextId,
      String const& tilesetPath) const;

  // Mark the given tiles as 'invalid' so they can't be used. (Actually removing
  // them from the tileset would cause the tile indices to change and break
  // existing maps.)
  void invalidateTiles(StringSet const& invalidTiles,
      StringMap<std::size_t> const& existingTiles,
      JsonObject& tileProperties,
      JsonObject& tileImages,
      String const& tilesetPath) const;

  String m_source, m_name;
  List<std::shared_ptr<Tile>> m_tiles;
  std::shared_ptr<TileDatabase> m_database;
};

class TilesetUpdater {
public:
  void defineAssetSource(String const& source);
  void defineTile(std::shared_ptr<Tile> const& tile);
  void exportTilesets();

private:
  std::shared_ptr<TileDatabase> const& getDatabase(std::shared_ptr<Tile> const& tile);
  std::shared_ptr<Tileset> const& getTileset(std::shared_ptr<Tile> const& tile);

  // Asset Source -> Tileset Name -> Tileset
  StringMap<StringMap<std::shared_ptr<Tileset>>> m_tilesets;
  // Asset Source -> Database Name -> Database
  StringMap<StringMap<std::shared_ptr<TileDatabase>>> m_databases;

  // Images that existed before running update_tilesets:
  // Asset Source -> Database Name -> Tile Name
  StringMap<StringMap<StringSet>> m_preexistingImages;
};

}


export module star.tileset_updater;

export namespace Star {
  using ::Star::Tile;
  using ::Star::TileDatabase;
  using ::Star::Tileset;
  using ::Star::TilesetUpdater;
}
