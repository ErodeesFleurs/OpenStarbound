#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarTileDamage.hpp"

import std;

namespace Star {

class Plant;

using PlantDatabaseException = ExceptionDerived<"PlantDatabaseException">;

// Configuration for a specific tree variant
struct TreeVariant {
  TreeVariant();
  TreeVariant(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  String stemName;
  String foliageName;

  String stemDirectory;
  Json stemSettings;
  float stemHueShift;

  String foliageDirectory;
  Json foliageSettings;
  float foliageHueShift;

  Json descriptions;
  bool ceiling;

  bool ephemeral;

  Json stemDropConfig;
  Json foliageDropConfig;

  TileDamageParameters tileDamageParameters;
};

// Configuration for a specific grass variant
struct GrassVariant {
  GrassVariant();
  GrassVariant(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  String name;

  String directory;
  StringList images;
  float hueShift;

  Json descriptions;
  bool ceiling;

  bool ephemeral;

  TileDamageParameters tileDamageParameters;
};

// Configuration for a specific bush variant
struct BushVariant {
  struct BushShape {
    String image;
    StringList mods;
  };

  BushVariant();
  BushVariant(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  String bushName;
  String modName;

  String directory;
  List<BushShape> shapes;

  float baseHueShift;
  float modHueShift;

  Json descriptions;
  bool ceiling;

  bool ephemeral;

  TileDamageParameters tileDamageParameters;
};

class PlantDatabase {
public:
  PlantDatabase();

  [[nodiscard]] auto treeStemNames(bool ceiling = false) const -> StringList;
  [[nodiscard]] auto treeFoliageNames() const -> StringList;
  // Each stem / foliage set has its own patterns of shapes that must match up
  [[nodiscard]] auto treeStemShape(String const& stemName) const -> String;
  [[nodiscard]] auto treeFoliageShape(String const& foliageName) const -> String;
  [[nodiscard]] auto treeStemDirectory(String const& stemName) const -> std::optional<String>;
  [[nodiscard]] auto treeFoliageDirectory(String const& foliageName) const -> std::optional<String>;
  // Throws an exception if stem shape and foliage shape do not match
  [[nodiscard]] auto buildTreeVariant(String const& stemName, float stemHueShift, String const& foliageName, float foliageHueShift) const -> TreeVariant;
  // Build a foliage-less tree
  [[nodiscard]] auto buildTreeVariant(String const& stemName, float stemHueShift) const -> TreeVariant;

  [[nodiscard]] auto grassNames(bool ceiling = false) const -> StringList;
  [[nodiscard]] auto buildGrassVariant(String const& grassName, float hueShift) const -> GrassVariant;

  [[nodiscard]] auto bushNames(bool ceiling = false) const -> StringList;
  [[nodiscard]] auto bushMods(String const& bushName) const -> StringList;
  [[nodiscard]] auto buildBushVariant(String const& bushName, float baseHueShift, String const& modName, float modHueShift) const -> BushVariant;

  [[nodiscard]] auto createPlant(TreeVariant const& treeVariant, std::uint64_t seed) const -> Ptr<Plant>;
  [[nodiscard]] auto createPlant(GrassVariant const& grassVariant, std::uint64_t seed) const -> Ptr<Plant>;
  [[nodiscard]] auto createPlant(BushVariant const& bushVariant, std::uint64_t seed) const -> Ptr<Plant>;

private:
  struct Config {
    String directory;
    Json settings;
  };

  StringMap<Config> m_treeStemConfigs;
  StringMap<Config> m_treeFoliageConfigs;

  StringMap<Config> m_grassConfigs;

  StringMap<Config> m_bushConfigs;
};

}// namespace Star
