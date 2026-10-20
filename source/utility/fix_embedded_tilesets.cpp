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

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;

import star.tileset_database;

using namespace Star;

void removeCommonPrefix(StringList& a, StringList& b) {
  // Remove elements from a and b until there is one that differs.
  while (a.size() > 0 && b.size() > 0 && a[0] == b[0]) {
    a.eraseAt(0);
    b.eraseAt(0);
  }
}

String createRelativePath(String fromFile, String toFile) {
  if (!File::isDirectory(fromFile))
    fromFile = File::dirName(fromFile);
  fromFile = File::fullPath(fromFile);
  toFile = File::fullPath(toFile);

  StringList fromParts = fromFile.splitAny("/\\");
  StringList toParts = toFile.splitAny("/\\");
  removeCommonPrefix(fromParts, toParts);

  StringList relativeParts;
  for (String part : fromParts)
    relativeParts.append("..");
  relativeParts.appendAll(toParts);

  return relativeParts.join("/");
}

Maybe<Json> repairTileset(Json tileset, String const& mapPath, String const& tilesetPath) {
  if (tileset.contains("source"))
    return {};
  std::size_t firstGid = tileset.getUInt("firstgid");
  String tilesetName = tileset.getString("name");
  String tilesetFileName = File::relativeTo(tilesetPath, tilesetName + ".json");
  if (!File::exists(tilesetFileName))
    throw StarException::format("Tileset {} does not exist. Can't repair {}", tilesetFileName, mapPath);
  return {JsonObject{{"firstgid", firstGid}, {"source", createRelativePath(mapPath, tilesetFileName)}}};
}

Maybe<Json> repair(Json mapJson, String const& mapPath, String const& tilesetPath) {
  JsonArray tilesets = mapJson.getArray("tilesets");
  bool changed = false;
  for (std::size_t i = 0; i < tilesets.size(); ++i) {
    if (Maybe<Json> tileset = repairTileset(tilesets[i], mapPath, tilesetPath)) {
      tilesets[i] = *tileset;
      changed = true;
    }
  }
  if (!changed)
    return {};
  return mapJson.set("tilesets", tilesets);
}

void forEachRecursiveFileMatch(Logger& logger, String const& dirName, String const& filenameSuffix, std::function<void(String)> func) {
  for (std::pair<String, bool> entry : File::dirList(dirName)) {
    String entryPath = File::relativeTo(dirName, entry.first);
    // Never follow a symlinked directory: it can point outside of the tree (and
    // this walk writes to the files it finds), or at one of its own parents.
    if (entry.second && File::isSymlink(entryPath)) {
      logger.warn("fix_embedded_tilesets: Not descending into '{}' because it is a symlink", entryPath);
      continue;
    }

    if (entry.second)
      forEachRecursiveFileMatch(logger, entryPath, filenameSuffix, func);
    else if (entry.first.endsWith(filenameSuffix))
      func(entryPath);
  }
}

void fixEmbeddedTilesets(Logger& logger, String const& searchRoot, String const& tilesetPath) {
  forEachRecursiveFileMatch(logger, searchRoot, ".json", [&logger, tilesetPath](String const& path) {
      Json json = Json::parseJson(File::readFileString(path));
      if (json.contains("tilesets")) {
        if (Maybe<Json> fixed = repair(json, path, tilesetPath)) {
          File::writeFile(fixed->repr(2, true), path);
          logger.info("Repaired {}", path);
        }
      }
    });
}

int main(int argc, char* argv[]) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Info, false, {}});
    rootLoader.setSummary("Replaces embedded tilesets in Tiled JSON files with references to external tilesets. Assumes tilesets are available in the packed assets.");
    rootLoader.addArgument("searchRoot", OptionParser::Required);
    rootLoader.addArgument("tilesetsPath", OptionParser::Required);

    std::unique_ptr<::Star::Root> root;
    OptionParser::Options options;
    std::tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    String searchRoot = options.arguments[0];
    String tilesetPath = options.arguments[1];

    fixEmbeddedTilesets(root->logger(), searchRoot, tilesetPath);

    return 0;
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    cerrf("exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
