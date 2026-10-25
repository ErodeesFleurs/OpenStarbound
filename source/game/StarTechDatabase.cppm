module;

import std;
import star.string;
import star.format;
import star.json;
import star.bi_map;
import star.game_types;

namespace Star {

class Assets;

struct TechDatabaseExceptionTag {
  static constexpr std::string_view name() { return "TechDatabaseException"; }
};
using TechDatabaseException = StarError<TechDatabaseExceptionTag, StarException>;

enum class TechType {
  Head,
  Body,
  Legs
};
extern std::expected<EnumMap<TechType>, StarException> const TechTypeNames;

struct TechConfig {
  String name;
  String path;
  Json parameters;

  TechType type;

  StringList scripts;
  std::optional<String> animationConfig;

  String description;
  String shortDescription;
  Rarity rarity;
  String icon;
};

class TechDatabase {
public:
  [[nodiscard]] static std::expected<std::shared_ptr<TechDatabase>, StarException> create(std::shared_ptr<Assets const> assets);

  bool contains(String const& techName) const;
  [[nodiscard]] std::expected<TechConfig, StarException> tech(String const& techName) const;

private:
  TechDatabase() = default;
  [[nodiscard]] std::expected<TechConfig, StarException> parseTech(Json const& config, String const& path) const;

  StringMap<TechConfig> m_tech;
};

}

export module star.tech_database;

export namespace Star {
  using ::Star::TechDatabaseExceptionTag;
  using ::Star::TechDatabaseException;
  using ::Star::TechDatabase;
  using ::Star::TechType;
  using ::Star::TechTypeNames;
  using ::Star::TechConfig;
}
