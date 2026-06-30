#pragma once

#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarJson.hpp"
#include "StarThread.hpp"
#include "StarTileDamage.hpp"

namespace Star {

class Plant;
using PlantPtr = SharedPtr<Plant>;
class PlantDatabase;
using PlantDatabasePtr = SharedPtr<PlantDatabase>;
using PlantDatabaseConstPtr = SharedPtr<PlantDatabase const>;

struct PlantDatabaseExceptionTag { static constexpr char const* typeName = "PlantDatabaseException"; };
using PlantDatabaseException = TypedException<StarException, PlantDatabaseExceptionTag>;

// Configuration for a specific tree variant
struct TreeVariant {
  TreeVariant() = default;
  TreeVariant(Json const& json);

  [[nodiscard]] Json toJson() const;

  String stemName;
  String foliageName;

  String stemDirectory;
  Json stemSettings;
  float stemHueShift = 0.0f;

  String foliageDirectory;
  Json foliageSettings;
  float foliageHueShift = 0.0f;

  Json descriptions;
  bool ceiling = false;

  bool ephemeral = false;

  Json stemDropConfig;
  Json foliageDropConfig;

  TileDamageParameters tileDamageParameters;
};

// Configuration for a specific grass variant
struct GrassVariant {
  GrassVariant() = default;
  GrassVariant(Json const& json);

  [[nodiscard]] Json toJson() const;

  String name;

  String directory;
  StringList images;
  float hueShift = 0.0f;

  Json descriptions;
  bool ceiling = false;

  bool ephemeral = false;

  TileDamageParameters tileDamageParameters;
};

// Configuration for a specific bush variant
struct BushVariant {
  struct BushShape {
    String image;
    StringList mods;
  };

  BushVariant() = default;
  BushVariant(Json const& json);

  [[nodiscard]] Json toJson() const;

  String bushName;
  String modName;

  String directory;
  List<BushShape> shapes;

  float baseHueShift = 0.0f;
  float modHueShift = 0.0f;

  Json descriptions;
  bool ceiling = false;

  bool ephemeral = false;

  TileDamageParameters tileDamageParameters;
};

class PlantDatabase {
public:
  PlantDatabase(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] StringList treeStemNames(bool ceiling = false) const;
  [[nodiscard]] StringList treeFoliageNames() const;
  // Each stem / foliage set has its own patterns of shapes that must match up
  [[nodiscard]] String treeStemShape(String const& stemName) const;
  [[nodiscard]] String treeFoliageShape(String const& foliageName) const;
  [[nodiscard]] Maybe<String> treeStemDirectory(String const& stemName) const;
  [[nodiscard]] Maybe<String> treeFoliageDirectory(String const& foliageName) const;
  // Throws an exception if stem shape and foliage shape do not match
  [[nodiscard]] TreeVariant buildTreeVariant(String const& stemName, float stemHueShift, String const& foliageName, float foliageHueShift) const;
  // Build a foliage-less tree
  [[nodiscard]] TreeVariant buildTreeVariant(String const& stemName, float stemHueShift) const;

  [[nodiscard]] StringList grassNames(bool ceiling = false) const;
  [[nodiscard]] GrassVariant buildGrassVariant(String const& grassName, float hueShift) const;

  [[nodiscard]] StringList bushNames(bool ceiling = false) const;
  [[nodiscard]] StringList bushMods(String const& bushName) const;
  [[nodiscard]] BushVariant buildBushVariant(String const& bushName, float baseHueShift, String const& modName, float modHueShift) const;

  [[nodiscard]] PlantPtr createPlant(TreeVariant const& treeVariant, uint64_t seed) const;
  [[nodiscard]] PlantPtr createPlant(GrassVariant const& grassVariant, uint64_t seed) const;
  [[nodiscard]] PlantPtr createPlant(BushVariant const& bushVariant, uint64_t seed) const;

  [[nodiscard]] Json treeFoliageConfig(String const& foliageName) const;
  [[nodiscard]] Json treeStemConfig(String const& stemName) const;

private:
  struct Config {
    String directory;
    Json settings;
    [[nodiscard]] Json toJson() const;
  };

  StringMap<Config> m_treeStemConfigs;
  StringMap<Config> m_treeFoliageConfigs;

  StringMap<Config> m_grassConfigs;

  StringMap<Config> m_bushConfigs;
  AssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
};

}
