#include "StarTechDatabase.hpp"

#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

EnumMap<TechType> const TechTypeNames{
  {TechType::Head, "Head"},
  {TechType::Body, "Body"},
  {TechType::Legs, "Legs"}};

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

auto TechDatabase::contains(String const& techName) const -> bool {
  return m_tech.contains(techName);
}

auto TechDatabase::tech(String const& techName) const -> TechConfig {
  if (auto p = m_tech.ptr(techName))
    return *p;
  throw TechDatabaseException::format("No such tech '{}'", techName);
}

auto TechDatabase::parseTech(Json const& config, String const& path) const -> TechConfig {
  try {
    auto assets = Root::singleton().assets();

    TechConfig tech;
    tech.name = config.getString("name");
    tech.path = path;
    tech.parameters = config;

    tech.type = TechTypeNames.getLeft(config.getString("type"));

    tech.scripts = jsonToStringList(config.get("scripts")).transformed([&path](String const& s) -> String { return AssetPath::relativeTo(path, s); });
    tech.animationConfig = config.optString("animator").transform([&path](String const& s) -> String { return AssetPath::relativeTo(path, s); });

    tech.description = config.getString("description");
    tech.shortDescription = config.getString("shortDescription");
    tech.rarity = RarityNames.getLeft(config.getString("rarity"));
    tech.icon = AssetPath::relativeTo(path, config.getString("icon"));

    return tech;
  } catch (std::exception const& e) {
    throw TechDatabaseException(strf("Error reading tech config {}", path), e);
  }
}

}// namespace Star
