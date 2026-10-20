import star.json;
import star.native_runtime;
import star.maybe;
import star.file;
import star.logging;
import star.string;
import star.variant;
import star.ordered_map;
import star.ordered_set;
import std;
import star.format;
import star.version;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.listener;
import star.image;
import star.vector;
import star.game_types;
import star.material_types;
import star.random;
import star.set;
import star.lru_cache;

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;
import star.item_descriptor;
import star.world_geometry;
import star.dungeon_generator;

import star.tileset_database;
import star.dungeon_tmx_part;

using namespace Star;
using namespace Star::Dungeon;

typedef String TileName;
typedef std::pair<String, String> TileProperty;

typedef MVariant<TileName, TileProperty> MatchCriteria;

struct SearchParameters {
  MatchCriteria criteria;
};

typedef std::function<void(String, Vec2I)> MatchReporter;

String const MapFilenameSuffix = ".json";

Maybe<String> matchTile(SearchParameters const& search, Tiled::Tile const& tile) {
  Tiled::Properties const& properties = tile.properties;
  if (search.criteria.is<TileName>()) {
    if (auto tileName = properties.opt<String>("//name"))
      if (tileName->regexMatch(search.criteria.get<TileName>()))
        return tileName;
  } else {
    String propertyName;
    String matchValue;
    std::tie(propertyName, matchValue) = search.criteria.get<TileProperty>();
    if (auto propertyValue = properties.opt<String>(propertyName))
      if (propertyValue->regexMatch(matchValue))
        return properties.opt<String>("//name").value("?");
  }
  return {};
}

void grepTileLayer(SearchParameters const& search, std::shared_ptr<::Star::Dungeon::TMXMap> map, std::shared_ptr<::Star::Dungeon::TMXTileLayer> tileLayer, MatchReporter callback) {
  tileLayer->forEachTile(map.get(),
      [&](Vec2I pos, Tile const& tile) {
        if (auto tileName = matchTile(search, static_cast<Tiled::Tile const&>(tile)))
          callback(*tileName, pos);
        return false;
      });
}

void grepObjectGroup(SearchParameters const& search, std::shared_ptr<::Star::Dungeon::TMXObjectGroup> objectGroup, MatchReporter callback) {
  for (auto object : objectGroup->objects()) {
    if (auto tileName = matchTile(search, object->tile()))
      callback(*tileName, object->pos());
  }
}

void grepMap(Root& root, SearchParameters const& search, String file) {
  auto map = std::make_shared<TMXMap>(root, Json::parseJson(File::readFileString(file)));

  for (auto tileLayer : map->tileLayers())
    grepTileLayer(search, map, tileLayer, [&](String const& tileName, Vec2I const& pos) {
        coutf("{}: {}: {} @ {}\n", file, tileLayer->name(), tileName, pos);
      });

  for (auto objectGroup : map->objectGroups())
    grepObjectGroup(search, objectGroup, [&](String const& tileName, Vec2I const& pos) {
        coutf("{}: {}: {} @ {}\n", file, objectGroup->name(), tileName, pos);
      });
}

void grepDirectory(Root& root, SearchParameters const& search, String directory) {
  for (std::pair<String, bool> entry : File::dirList(directory)) {
    String entryPath = File::relativeTo(directory, entry.first);
    // Never follow a symlinked directory: it can point outside of the tree, or
    // at one of its own parents (which would recurse until the OS errors out).
    if (entry.second && File::isSymlink(entryPath)) {
      root.logger().warn("map_grep: Not descending into '{}' because it is a symlink", entryPath);
      continue;
    }

    if (entry.second)
      grepDirectory(root, search, entryPath);
    else if (entry.first.endsWith(MapFilenameSuffix))
      grepMap(root, search, entryPath);
  }
}

void grepPath(Root& root, SearchParameters const& search, String path) {
  if (File::isFile(path)) {
    grepMap(root, search, path);
  } else if (File::isDirectory(path)) {
    grepDirectory(root, search, path);
  }
}

MatchCriteria parseMatchCriteria(String const& criteriaStr) {
  if (criteriaStr.contains("=")) {
    StringList parts = criteriaStr.split('=', 1);
    return std::make_pair(parts[0], parts[1]);
  }
  return TileName(criteriaStr);
}

int main(int argc, char* argv[]) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Warn, false, {}});
    rootLoader.setSummary("Search Tiled map files for specific materials or objects.");
    rootLoader.addArgument("MaterialId|ObjectName|Property=Value", OptionParser::Required);
    rootLoader.addArgument("JsonMapFile", OptionParser::Multiple);

    std::unique_ptr<::Star::Root> root;
    OptionParser::Options options;
    std::tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    SearchParameters search = {parseMatchCriteria(options.arguments[0])};
    StringList files = options.arguments.slice(1);

    for (auto file : files)
      grepPath(*root, search, file);

    return 0;
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    cerrf("exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
