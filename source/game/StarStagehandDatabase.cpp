#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarLua.hpp"
#include "StarNetElementSystem.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
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
