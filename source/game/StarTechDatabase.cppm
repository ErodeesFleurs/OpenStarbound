#ifdef STAR_MODULE_IMPLEMENTATION
#include "StarGameTypes.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"

import star.tech_database;

namespace Star {

EnumMap<TechType> const TechTypeNames{
  {TechType::Head, "Head"},
  {TechType::Body, "Body"},
  {TechType::Legs, "Legs"}
};

TechDatabase::TechDatabase() {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("tech");
  assets->queueJsons(files);
  for (auto& file : files) {
    auto tech = parseTech(assets->json(file), file);

    if (m_tech.contains(tech.name))
      throw TechDatabaseException::format("Duplicate tech named '{}', config file '{}'", tech.name, file);
    m_tech[tech.name] = tech;
  }
}

bool TechDatabase::contains(String const& techName) const {
  return m_tech.contains(techName);
}

TechConfig TechDatabase::tech(String const& techName) const {
  if (auto p = m_tech.ptr(techName))
    return *p;
  throw TechDatabaseException::format("No such tech '{}'", techName);
}

TechConfig TechDatabase::parseTech(Json const& config, String const& path) const {
  try {
    auto assets = Root::singleton().assets();

    TechConfig tech;
    tech.name = config.getString("name");
    tech.path = path;
    tech.parameters = config;

    tech.type = TechTypeNames.getLeft(config.getString("type"));

    tech.scripts = jsonToStringList(config.get("scripts")).transformed(bind(AssetPath::relativeTo, path, _1));
    tech.animationConfig = config.optString("animator").apply(bind(&AssetPath::relativeTo, path, _1));

    tech.description = config.getString("description");
    tech.shortDescription = config.getString("shortDescription");
    tech.rarity = RarityNames.getLeft(config.getString("rarity"));
    tech.icon = AssetPath::relativeTo(path, config.getString("icon"));

    return tech;
  } catch (std::exception const& e) {
    throw TechDatabaseException(strf("Error reading tech config {}", path), e);
  }
}

}
#else
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
#endif
