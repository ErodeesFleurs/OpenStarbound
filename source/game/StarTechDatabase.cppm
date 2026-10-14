module;

import std;
import star.string;
import star.maybe;
import star.format;
import star.json;
import star.bi_map;
import star.game_types;

namespace Star {

struct TechDatabaseExceptionTag {
  static constexpr char const* name() { return "TechDatabaseException"; }
};
using TechDatabaseException = StarError<TechDatabaseExceptionTag, StarException>;

class TechDatabase;
using TechDatabasePtr = std::shared_ptr<TechDatabase>;
using TechDatabaseConstPtr = std::shared_ptr<const TechDatabase>;
using TechDatabaseWeakPtr = std::weak_ptr<TechDatabase>;
using TechDatabaseConstWeakPtr = std::weak_ptr<const TechDatabase>;
using TechDatabaseUPtr = std::unique_ptr<TechDatabase>;
using TechDatabaseConstUPtr = std::unique_ptr<const TechDatabase>;

enum class TechType {
  Head,
  Body,
  Legs
};
extern EnumMap<TechType> const TechTypeNames;

struct TechConfig {
  String name;
  String path;
  Json parameters;

  TechType type;

  StringList scripts;
  Maybe<String> animationConfig;

  String description;
  String shortDescription;
  Rarity rarity;
  String icon;
};

class TechDatabase {
public:
  TechDatabase();

  bool contains(String const& techName) const;
  TechConfig tech(String const& techName) const;

private:
  TechConfig parseTech(Json const& config, String const& path) const;

  StringMap<TechConfig> m_tech;
};

}

export module star.tech_database;

export namespace Star {
  using ::Star::TechDatabaseExceptionTag;
  using ::Star::TechDatabaseException;
  using ::Star::TechDatabase;
  using ::Star::TechDatabasePtr;
  using ::Star::TechDatabaseConstPtr;
  using ::Star::TechType;
  using ::Star::TechTypeNames;
  using ::Star::TechConfig;
}
