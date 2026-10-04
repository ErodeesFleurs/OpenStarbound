module;

#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"

namespace Star {

struct TechDatabaseExceptionTag {
  static constexpr char const* name() { return "TechDatabaseException"; }
};
using TechDatabaseException = StarError<TechDatabaseExceptionTag, StarException>;

STAR_CLASS(TechDatabase);

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
