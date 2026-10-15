module;
import std;
import star.string;
import star.list;
import star.maybe;
import star.format;
import star.json;

import star.bi_map;
import star.tile_damage;

namespace Star {

class Plant;

struct PlantDatabaseExceptionTag {
  static constexpr char const* name() { return "PlantDatabaseException"; }
};
using PlantDatabaseException = StarError<PlantDatabaseExceptionTag, StarException>;

// Configuration for a specific tree variant
struct TreeVariant {
  TreeVariant();
  TreeVariant(Json const& json);

  Json toJson() const;

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

  Json toJson() const;

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

  Json toJson() const;

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

  StringList treeStemNames(bool ceiling = false) const;
  StringList treeFoliageNames() const;
  // Each stem / foliage set has its own patterns of shapes that must match up
  String treeStemShape(String const& stemName) const;
  String treeFoliageShape(String const& foliageName) const;
  Maybe<String> treeStemDirectory(String const& stemName) const;
  Maybe<String> treeFoliageDirectory(String const& foliageName) const;
  // Throws an exception if stem shape and foliage shape do not match
  TreeVariant buildTreeVariant(String const& stemName, float stemHueShift, String const& foliageName, float foliageHueShift) const;
  // Build a foliage-less tree
  TreeVariant buildTreeVariant(String const& stemName, float stemHueShift) const;

  StringList grassNames(bool ceiling = false) const;
  GrassVariant buildGrassVariant(String const& grassName, float hueShift) const;

  StringList bushNames(bool ceiling = false) const;
  StringList bushMods(String const& bushName) const;
  BushVariant buildBushVariant(String const& bushName, float baseHueShift, String const& modName, float modHueShift) const;

  std::shared_ptr<Plant> createPlant(TreeVariant const& treeVariant, std::uint64_t seed) const;
  std::shared_ptr<Plant> createPlant(GrassVariant const& grassVariant, std::uint64_t seed) const;
  std::shared_ptr<Plant> createPlant(BushVariant const& bushVariant, std::uint64_t seed) const;

  Json treeFoliageConfig(String const& foliageName) const;
  Json treeStemConfig(String const& stemName) const;

private:
  struct Config {
    String directory;
    Json settings;
    Json toJson() const;
  };

  StringMap<Config> m_treeStemConfigs;
  StringMap<Config> m_treeFoliageConfigs;

  StringMap<Config> m_grassConfigs;

  StringMap<Config> m_bushConfigs;
};

}

export module star.plant_database;

export namespace Star {
  using ::Star::PlantDatabaseExceptionTag;
  using ::Star::PlantDatabaseException;
  using ::Star::TreeVariant;
  using ::Star::GrassVariant;
  using ::Star::BushVariant;
  using ::Star::Plant;
  using ::Star::PlantDatabase;
}
