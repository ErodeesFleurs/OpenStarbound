import star.json;
#include "StarJsonExtra.hpp"
import star.game_types;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
#include "StarConfig.hpp"
import star.string;
import star.format;
import star.version;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.tech_database;

namespace Star {

extern EnumMap<TechType> const TechTypeNames{
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
