module;
import std;
import star.string;
import star.list;
import star.format;
import star.json;

import star.bi_map;
import star.tile_damage;

namespace Star {

class Assets;
class Root;
class Plant;

struct PlantDatabaseExceptionTag {
  static constexpr std::string_view name() { return "PlantDatabaseException"; }
};
using PlantDatabaseException = StarError<PlantDatabaseExceptionTag, StarException>;

// Configuration for a specific tree variant
struct TreeVariant {
  TreeVariant();
  [[nodiscard]] static std::expected<TreeVariant, StarException> create(Assets const& assets, Json const& json);

  [[nodiscard]] std::expected<Json, StarException> toJson() const;

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
  [[nodiscard]] static std::expected<GrassVariant, StarException> create(Assets const& assets, Json const& json);

  [[nodiscard]] std::expected<Json, StarException> toJson() const;

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
  [[nodiscard]] static std::expected<BushVariant, StarException> create(Assets const& assets, Json const& json);

  [[nodiscard]] std::expected<Json, StarException> toJson() const;

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
  [[nodiscard]] static std::expected<std::shared_ptr<PlantDatabase>, StarException> create(Root& root);

  [[nodiscard]] std::expected<StringList, StarException> treeStemNames(bool ceiling = false) const;
  StringList treeFoliageNames() const;
  // Each stem / foliage set has its own patterns of shapes that must match up
  [[nodiscard]] std::expected<String, StarException> treeStemShape(String const& stemName) const;
  [[nodiscard]] std::expected<String, StarException> treeFoliageShape(String const& foliageName) const;
  std::optional<String> treeStemDirectory(String const& stemName) const;
  std::optional<String> treeFoliageDirectory(String const& foliageName) const;
  // Returns an error if stem shape and foliage shape do not match
  [[nodiscard]] std::expected<TreeVariant, StarException> buildTreeVariant(String const& stemName, float stemHueShift, String const& foliageName, float foliageHueShift) const;
  // Build a foliage-less tree
  [[nodiscard]] std::expected<TreeVariant, StarException> buildTreeVariant(String const& stemName, float stemHueShift) const;

  [[nodiscard]] std::expected<StringList, StarException> grassNames(bool ceiling = false) const;
  [[nodiscard]] std::expected<GrassVariant, StarException> buildGrassVariant(String const& grassName, float hueShift) const;

  [[nodiscard]] std::expected<StringList, StarException> bushNames(bool ceiling = false) const;
  [[nodiscard]] std::expected<StringList, StarException> bushMods(String const& bushName) const;
  [[nodiscard]] std::expected<BushVariant, StarException> buildBushVariant(String const& bushName, float baseHueShift, String const& modName, float modHueShift) const;

  [[nodiscard]] std::expected<std::shared_ptr<Plant>, StarException> createPlant(TreeVariant const& treeVariant, std::uint64_t seed) const;
  [[nodiscard]] std::expected<std::shared_ptr<Plant>, StarException> createPlant(GrassVariant const& grassVariant, std::uint64_t seed) const;
  [[nodiscard]] std::expected<std::shared_ptr<Plant>, StarException> createPlant(BushVariant const& bushVariant, std::uint64_t seed) const;

  [[nodiscard]] std::expected<Json, StarException> treeFoliageConfig(String const& foliageName) const;
  [[nodiscard]] std::expected<Json, StarException> treeStemConfig(String const& stemName) const;

private:
  explicit PlantDatabase(Root& root) : m_root(root) {}
  struct Config {
    String directory;
    Json settings;
    Json toJson() const;
  };

  Root& m_root;

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
