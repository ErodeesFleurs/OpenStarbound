import star.json;
#include "StarJsonExtra.hpp"
import star.id_map;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.lua;
import star.ordered_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
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

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
#include "StarLuaComponents.hpp"
import star.scripted_entity;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.stagehand;

import star.stagehand_database;

namespace Star {

StagehandDatabase::StagehandDatabase() {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("stagehand");
  assets->queueJsons(files);
  for (auto& file : files) {
    try {
      auto config = assets->json(file);

      String typeName = config.getString("type");

      if (m_stagehandTypes.contains(typeName))
        throw StagehandDatabaseException(strf("Repeat stagehand type name '{}'", typeName));

      m_stagehandTypes[typeName] = config;

    } catch (StarException const& e) {
      throw StagehandDatabaseException(strf("Error loading stagehand type '{}'", file), e);
    }
  }
}

StagehandPtr StagehandDatabase::createStagehand(String const& stagehandType, Json const& extraConfig) const {
  auto finalConfig = jsonMerge(m_stagehandTypes.get(stagehandType), extraConfig);
  return make_shared<Stagehand>(finalConfig);
}

}
