#include "StarConfig.hpp"
#include "StarFile.hpp"
#include "StarLogging.hpp"
#include "StarRootLoader.hpp"

import std;

using namespace Star;

void removeCommonPrefix(StringList& a, StringList& b) {
  // Remove elements from a and b until there is one that differs.
  while (a.size() > 0 && b.size() > 0 && a[0] == b[0]) {
    a.eraseAt(0);
    b.eraseAt(0);
  }
}

auto createRelativePath(String fromFile, String toFile) -> String {
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

auto repairTileset(Json tileset, String const& mapPath, String const& tilesetPath) -> std::optional<Json> {
  if (tileset.contains("source"))
    return {};
  size_t firstGid = tileset.getUInt("firstgid");
  String tilesetName = tileset.getString("name");
  String tilesetFileName = File::relativeTo(tilesetPath, tilesetName + ".json");
  if (!File::exists(tilesetFileName))
    throw StarException::format("Tileset {} does not exist. Can't repair {}", tilesetFileName, mapPath);
  return {JsonObject{{"firstgid", firstGid}, {"source", createRelativePath(mapPath, tilesetFileName)}}};
}

auto repair(Json mapJson, String const& mapPath, String const& tilesetPath) -> std::optional<Json> {
  JsonArray tilesets = mapJson.getArray("tilesets");
  bool changed = false;
  for (auto& i : tilesets) {
    if (std::optional<Json> tileset = repairTileset(i, mapPath, tilesetPath)) {
      i = *tileset;
      changed = true;
    }
  }
  if (!changed)
    return {};
  return mapJson.set("tilesets", tilesets);
}

void forEachRecursiveFileMatch(String const& dirName, String const& filenameSuffix, std::function<void(String)> func) {
  for (std::pair<String, bool> entry : File::dirList(dirName)) {
    if (entry.second)
      forEachRecursiveFileMatch(File::relativeTo(dirName, entry.first), filenameSuffix, func);
    else if (entry.first.endsWith(filenameSuffix))
      func(File::relativeTo(dirName, entry.first));
  }
}

void fixEmbeddedTilesets(String const& searchRoot, String const& tilesetPath) {
  forEachRecursiveFileMatch(searchRoot, ".json", [tilesetPath](String const& path) -> void {
    Json json = Json::parseJson(File::readFileString(path));
    if (json.contains("tilesets")) {
      if (std::optional<Json> fixed = repair(json, path, tilesetPath)) {
        File::writeFile(fixed->repr(2, true), path);
        Logger::info("Repaired {}", path);
      }
    }
  });
}

auto main(int argc, char* argv[]) -> int {
  try {
    RootLoader rootLoader({.additionalAssetsSettings = {}, .additionalDefaultConfiguration = {}, .logFile = {}, .logLevel = LogLevel::Info, .quiet = false, .runtimeConfigFile = {}});
    rootLoader.setSummary("Replaces embedded tilesets in Tiled JSON files with references to external tilesets. Assumes tilesets are available in the packed assets.");
    rootLoader.addArgument("searchRoot", OptionParser::Required);
    rootLoader.addArgument("tilesetsPath", OptionParser::Required);

    UPtr<Root> root;
    OptionParser::Options options;
    tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    String searchRoot = options.arguments[0];
    String tilesetPath = options.arguments[1];

    fixEmbeddedTilesets(searchRoot, tilesetPath);

    return 0;
  } catch (std::exception const& e) {
    cerrf("exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
