import star.json;
import star.json_extra;
import star.ordered_map;
import star.either;
import star.game_types;
import star.color;
import star.bi_map;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.data_stream_extra;
import star.data_stream_devices;
import star.format;
import star.lexical_cast;
import star.file;
import star.logging;
import star.io_device;
import star.ref_ptr;
import star.parametric_function;
import star.multi_array_interpolator;
import star.multi_table;
import star.listener;
import std;
import star.optional;
import star.string;
import star.lua;
import star.version;

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.utility_lua_bindings;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.stored_functions;

import star.versioning_database;
import star.root_lua_bindings;
import star.npc_database;
import star.celestial_database;


import star.byte_array;
import star.algorithm;
namespace Star {

char const* const VersionedJson::Magic = "SBVJ01";
std::size_t const VersionedJson::MagicStringSize = 6;

VersionNumber const VersionedJson::SubVersioning = 1;

VersionedJson VersionedJson::readFile(String const& filename) {
  DataStreamIODevice ds(File::open(filename, IOMode::Read));

  if (ds.readBytes(MagicStringSize) != ByteArray(Magic, MagicStringSize))
    throw IOException(strf("Wrong magic bytes at start of versioned json file, expected '{}'", Magic));
  auto versionedJson = ds.read<VersionedJson>();
  readSubVersioning(ds, versionedJson);

  return versionedJson;
}

void VersionedJson::writeFile(VersionedJson const& versionedJson, String const& filename) {
  DataStreamBuffer ds;
  ds.writeData(Magic, MagicStringSize);
  ds.write(versionedJson);
  writeSubVersioning(ds, versionedJson);
  File::overwriteFileWithRename(ds.takeData(), filename);
}

void VersionedJson::writeSubVersioning(DataStream& ds, VersionedJson const& versionedJson) {
  ds.write(VersionedJson::SubVersioning);
  JsonObject subVersionsOut;
  for (auto const& p : versionedJson.subVersions)
    subVersionsOut.set(p.first, p.second);
  ds.write(JsonObject{
    {"subVersions", subVersionsOut}
  });
}
void VersionedJson::readSubVersioning(DataStream& ds, VersionedJson& versionedJson) {
  if (ds.atEnd())
    return;
  VersionNumber extraVersioning = 0;
  try {
    ds.read(extraVersioning);
  } catch (...) {
  }
  if (extraVersioning == 1) {
    JsonObject source = ds.read<JsonObject>();
    for (auto const& p : source.get("subVersions").toObject())
      versionedJson.subVersions[p.first] = p.second.toUInt();
  }
}

Json VersionedJson::toJson() const {
  JsonObject subVersionsOut;
  for (auto const& p : subVersions)
    subVersionsOut.set(p.first, p.second);
  return JsonObject{
    {"id", identifier},
    {"version", version},
    {"content", content},
    {"subVersions", subVersionsOut}
  };
}

VersionedJson VersionedJson::fromJson(Json const& source) {
  // Old versions of VersionedJson used '__' to distinguish between actual
  // content and versioned content, but this is no longer necessary or
  // relevant.
  auto id = source.optString("id");
  auto legacyId = source.optString("__id");
  if (!id)
    id = std::move(legacyId);
  auto version = source.optUInt("version");
  auto legacyVersion = source.optUInt("__version");
  if (!version)
    version = std::move(legacyVersion);
  auto content = source.opt("content");
  auto legacyContent = source.opt("__content");
  if (!content)
    content = std::move(legacyContent);
  StringMap<VersionNumber> subVersions;
  for (auto const& p : source.getObject("subVersions", JsonObject()))
    subVersions[p.first] = p.second.toUInt();

  return {id.value(), (VersionNumber)version.value(), content.value(), subVersions};
}

bool VersionedJson::empty() const {
  return content.isNull();
}

void VersionedJson::expectIdentifier(String const& expectedIdentifier) const {
  if (identifier != expectedIdentifier)
    throw VersionedJsonException::format("VersionedJson identifier mismatch, expected '{}' but got '{}'", expectedIdentifier, identifier);
}


DataStream& operator>>(DataStream& ds, VersionedJson& versionedJson) {
  ds.read(versionedJson.identifier);
  // This is a holdover from when the verison number was optional in
  // VersionedJson.  We should convert versioned json binary files and the
  // celestial chunk database and world storage to a new format eventually
  versionedJson.version = ds.read<std::optional<VersionNumber>>().value_or(VersionNumber{});
  ds.read(versionedJson.content);

  // this is a holdover from when sub versions were smuggled into content without realizing this caused issues, can potentially be removed later
  if (versionedJson.content.isType(Json::Type::Object) && versionedJson.content.contains("subVersions")) {
    for (auto const& p : versionedJson.content.getObject("subVersions", JsonObject()))
      versionedJson.subVersions[p.first] = p.second.toUInt();
    versionedJson.content = versionedJson.content.eraseKey("subVersions");
  }
  return ds;
}

DataStream& operator<<(DataStream& ds, VersionedJson const& versionedJson) {
  ds.write(versionedJson.identifier);
  ds.write(std::optional<VersionNumber>(versionedJson.version));
  ds.write(versionedJson.content);
  return ds;
}

VersioningDatabase::VersioningDatabase(Root& root) : m_root(root), m_luaRoot(root) {
  auto assets = m_root.assets();

  for (auto const& pair : assets->json("/versioning.config").iterateObject())
    m_currentVersions[pair.first] = pair.second.toUInt();

  for (auto const& pair : assets->json("/versioning/subVersioning.config").iterateObject())
    for (auto const& p : pair.second.iterateObject())
      m_currentSubVersions[pair.first][p.first] = p.second.toUInt();

  for (auto const& scriptFile : assets->scan("/versioning/", ".lua")) {
    try {
      auto scriptParts = File::baseName(scriptFile).splitAny("_.");
      if (scriptParts.size() == 4) {
        String identifier = scriptParts.at(0);
        VersionNumber fromVersion = lexicalCast<VersionNumber>(scriptParts.at(1));
        VersionNumber toVersion = lexicalCast<VersionNumber>(scriptParts.at(2));

        m_versionUpdateScripts[identifier.toLower()].append({scriptFile, fromVersion, toVersion});
      } else if (scriptParts.size() == 6) {
        String identifier = scriptParts.at(0);
        VersionNumber atVersion = lexicalCast<VersionNumber>(scriptParts.at(1));
        String subIdentifier = scriptParts.at(2);
        VersionNumber fromVersion = lexicalCast<VersionNumber>(scriptParts.at(3));
        VersionNumber toVersion = lexicalCast<VersionNumber>(scriptParts.at(4));

        m_subVersionUpdateScripts[identifier.toLower()][atVersion][subIdentifier.toLower()].append({scriptFile, fromVersion, toVersion});
      } else {
        throw VersioningDatabaseException::format("Script file '{}' filename not of the form <identifier>_<fromversion>_<toversion>.lua or <identifier>_<atVersion>_<subIdentifier>_<fromSubVersion>_<toSubVersion>.lua", scriptFile);
      }

    } catch (StarException const&) {
      throw VersioningDatabaseException::format("Error parsing version information from versioning script '{}'", scriptFile);
    }
  }

  // Sort each set of update scripts first by fromVersion, and then in
  // *reverse* order of toVersion.  This way, the first matching script for a
  // given fromVersion should take the json to the *furthest* toVersion.
  for (auto& pair : m_versionUpdateScripts) {
    pair.second.sort([](VersionUpdateScript const& lhs, VersionUpdateScript const& rhs) {
      if (lhs.fromVersion != rhs.fromVersion)
        return lhs.fromVersion < rhs.fromVersion;
      else
        return lhs.toVersion < rhs.toVersion;
    });
  }
  for (auto& p : m_subVersionUpdateScripts)
    for (auto& version : p.second)
      for (auto& pair : version.second) {
        pair.second.sort([](VersionUpdateScript const& lhs, VersionUpdateScript const& rhs) {
          if (lhs.fromVersion != rhs.fromVersion)
            return lhs.fromVersion < rhs.fromVersion;
          else
            return lhs.toVersion < rhs.toVersion;
        });
      }



}

VersionedJson VersioningDatabase::makeCurrentVersionedJson(String const& identifier, Json const& content) const {
  RecursiveMutexLocker locker(m_mutex);
  return VersionedJson{identifier, m_currentVersions.get(identifier), content, m_currentSubVersions.value(identifier)};
}

bool VersioningDatabase::versionedJsonCurrent(VersionedJson const& versionedJson) const {
  RecursiveMutexLocker locker(m_mutex);
  return (versionedJson.version == m_currentVersions.get(versionedJson.identifier))
  && (versionedJson.subVersions == m_currentSubVersions.value(versionedJson.identifier));
}

VersionedJson VersioningDatabase::updateVersionedJson(VersionedJson const& versionedJson) const {
  return updateVersionedJson(versionedJson, nullptr);
}

VersionedJson VersioningDatabase::updateVersionedJson(VersionedJson const& versionedJson, ItemDatabase const* itemCatalog) const {
  RecursiveMutexLocker locker(m_mutex);

  auto& root = m_root;
  CelestialMasterDatabase celestialDatabase(root);

  VersionedJson result = versionedJson;
  std::optional<VersionNumber> targetVersion = m_currentVersions.maybe(versionedJson.identifier);
  if (!targetVersion)
    throw VersioningDatabaseException::format("Versioned JSON has an unregistered identifier '{}'", versionedJson.identifier);

  LuaCallbacks celestialCallbacks;
  celestialCallbacks.registerCallback("parameters", [&celestialDatabase](Json const& coord) {
      return celestialDatabase.parameters(CelestialCoordinate(coord))->diskStore();
    });

  auto invokeUpdate = [&](String const& script, Json const& content) {
    auto scriptContext = m_luaRoot.createContext();
    auto previousRoot = scriptContext.get("root");
    auto previousVersioning = scriptContext.get("versioning");
    auto restoreCallbacks = finally([&] {
      scriptContext.set("root", previousRoot);
      scriptContext.set("versioning", previousVersioning);
    });
    scriptContext.remove("root");
    scriptContext.remove("versioning");
    scriptContext.setCallbacks("root", LuaBindings::makeRootCallbacks(root, itemCatalog));
    scriptContext.setCallbacks("sb", LuaBindings::makeUtilityCallbacks(root.random(), root.logger(), root.logMap()));
    scriptContext.setCallbacks("celestial", celestialCallbacks);
    scriptContext.setCallbacks("versioning", makeVersioningCallbacks(itemCatalog));
    scriptContext.load(*root.assets()->bytes(script), script);
    return scriptContext.invokePath<Json>("update", content);
  };

  try {
    for (auto const& updateScript : m_versionUpdateScripts.value(versionedJson.identifier.toLower())) {
      for (auto const& subVersionScripts : m_subVersionUpdateScripts.value(versionedJson.identifier.toLower()).value(result.version)) {
        auto targetSubVersion = m_currentSubVersions.value(result.identifier).value(subVersionScripts.first);
        for (auto const& subVersionUpdateScript : subVersionScripts.second) {
          if (result.subVersions.value(subVersionScripts.first) >= targetSubVersion)
            break;

          if (subVersionUpdateScript.fromVersion == result.subVersions.value(subVersionScripts.first)) {
            result.content = invokeUpdate(subVersionUpdateScript.script, result.content);
            if (!result.content) {
              throw VersioningDatabaseException::format(
                  "Could not bring versionedJson with identifier '{}' and version {} forward to current version of {}, conversion script of sub identifier '{}' from {} to {} returned null (un-upgradeable)",
                  versionedJson.identifier, result.version, outputOptional(targetVersion), subVersionScripts.first, subVersionUpdateScript.fromVersion, subVersionUpdateScript.toVersion);
            }
            root.logger().debug("Brought versionedJson '{}' sub identifier '{}' from version {} to {}",
                versionedJson.identifier, subVersionScripts.first, result.subVersions.value(subVersionScripts.first), subVersionUpdateScript.toVersion);
            result.subVersions[subVersionScripts.first] = subVersionUpdateScript.toVersion;
          }
        }
      }

      if (result.version >= *targetVersion)
        break;

      if (updateScript.fromVersion == result.version) {
        result.content = invokeUpdate(updateScript.script, result.content);
        if (!result.content) {
          throw VersioningDatabaseException::format(
              "Could not bring versionedJson with identifier '{}' and version {} forward to current version of {}, conversion script from {} to {} returned null (un-upgradeable)",
              versionedJson.identifier, result.version, outputOptional(targetVersion), updateScript.fromVersion, updateScript.toVersion);
        }
        root.logger().debug("Brought versionedJson '{}' from version {} to {}",
            versionedJson.identifier, result.version, updateScript.toVersion);
        result.version = updateScript.toVersion;
      }
    }
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    throw VersioningDatabaseException(strf("Could not bring versionedJson with identifier '{}' and version {} forward to current version of {}",
            versionedJson.identifier, result.version, outputOptional(targetVersion)), e);
  }

  if (result.version > *targetVersion) {
    throw VersioningDatabaseException::format(
        "VersionedJson with identifier '{}' and version {} is newer than current version of {}, cannot load",
        versionedJson.identifier, result.version, outputOptional(targetVersion));
  }

  if (result.version != *targetVersion) {
    throw VersioningDatabaseException::format(
        "Could not bring VersionedJson with identifier '{}' and version {} forward to current version of {}, best version was {}",
        versionedJson.identifier, result.version, outputOptional(targetVersion), result.version);
  }

  return result;
}

Json VersioningDatabase::loadVersionedJson(VersionedJson const& versionedJson, String const& expectedIdentifier) const {
  return loadVersionedJson(versionedJson, expectedIdentifier, nullptr);
}

Json VersioningDatabase::loadVersionedJson(VersionedJson const& versionedJson, String const& expectedIdentifier, ItemDatabase const* itemCatalog) const {
  versionedJson.expectIdentifier(expectedIdentifier);
  if (versionedJsonCurrent(versionedJson))
    return versionedJson.content;
  return updateVersionedJson(versionedJson, itemCatalog).content;
}

LuaCallbacks VersioningDatabase::makeVersioningCallbacks(ItemDatabase const* itemCatalog) const {
  LuaCallbacks versioningCallbacks;

  versioningCallbacks.registerCallback("loadVersionedJson", [this, itemCatalog](String const& storagePath) {
      try {
        auto& root = m_root;
        String filePath = File::fullPath(root.toStoragePath(storagePath));
        String basePath = File::fullPath(root.toStoragePath("."));
        if (!filePath.beginsWith(basePath))
          throw VersioningDatabaseException::format(
              "Cannot load external VersionedJson outside of the Root storage path");
        auto loadedJson = VersionedJson::readFile(filePath);
        return updateVersionedJson(loadedJson, itemCatalog).content;
      } catch (IOException const& e) {
        m_root.logger().debug(
            "Unable to load versioned JSON file {} in versioning script: {}", storagePath, outputException(e, false));
        return Json();
      }
    });

  return versioningCallbacks;
}

}
