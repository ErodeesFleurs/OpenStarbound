#include "StarTestUniverse.hpp"
#include "StarTestRoot.hpp"
#include "StarAssets.hpp"
#include "StarFile.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerFactory.hpp"
#include "StarPlayerStorage.hpp"
#include "StarQuests.hpp"
#include "StarStatistics.hpp"
#include "StarStatisticsService.hpp"
#include "StarWorldClient.hpp"

#include "gtest/gtest.h"

namespace Star {

TestUniverse::TestUniverse(Vec2U clientWindowSize) {
  auto& root = testRoot();

  m_clientWindowSize = clientWindowSize;

  m_storagePath = File::temporaryDirectory();
  auto playerStorage = make_shared<PlayerStorage>(File::relativeTo(m_storagePath, "player"), root.configuration(), root.entityFactory());
  auto luaRootServices = root.luaRootServices();
  auto statistics = make_shared<Statistics>(
      File::relativeTo(m_storagePath, "statistics"),
      root.versioningDatabase(),
      root.statisticsDatabase(),
      luaRootServices);
  m_server = make_shared<UniverseServer>(File::relativeTo(m_storagePath, "universe"), root.assets(), root.configuration(), root.materialDatabase(), root.imageMetadataDatabase(), root.itemDatabase(), root.objectDatabase(), root.projectileDatabase(), root.plantDatabase(), root.treasureDatabase(), root.npcDatabase(), root.monsterDatabase(), root.spawnTypeDatabase(), root.stagehandDatabase(), root.vehicleDatabase(), root.speciesDatabase(), root.entityFactory(), root.liquidsDatabase(), root.terrainDatabase(), root.biomeDatabase(), root.nameGenerator(), root.versioningDatabase(), root.functionDatabase(), root.effectSourceDatabase(), root.particleDatabase(), root.techDatabase(), root.statusEffectDatabase(), root.dungeonDefinitions(), root.behaviorDatabase(), luaRootServices, [&root]() {
    root.reload();
    root.fullyLoad();
  });
  m_client = make_shared<UniverseClient>(playerStorage, statistics, root.assets(), root.configuration(), root.materialDatabase(), root.itemDatabase(), root.objectDatabase(), root.speciesDatabase(), root.entityFactory(), root.liquidsDatabase(), root.terrainDatabase(), root.biomeDatabase(), root.nameGenerator(), root.functionDatabase(), root.behaviorDatabase(), root.particleDatabase(), root.damageDatabase(), root.projectileDatabase(), root.effectSourceDatabase(), root.techDatabase(), root.statusEffectDatabase(), root.plantDatabase(), root.treasureDatabase(), root.imageMetadataDatabase(), root.dungeonDefinitions(), luaRootServices);

  m_server->start();

  m_mainPlayer = root.playerFactory()->create();
  m_mainPlayer->setSpecies("human");
  m_mainPlayer->setShipSpecies("human");
  m_mainPlayer->finalizeCreation();
  m_mainPlayer->setAdmin(true);
  m_mainPlayer->setModeType(PlayerMode::Survival);
  m_client->setMainPlayer(m_mainPlayer);
  m_client->connect(m_server->addLocalClient(), "test", "");
}

TestUniverse::~TestUniverse() {
  m_client = {};
  m_server = {};
  m_mainPlayer = {};
  File::removeDirectoryRecursive(m_storagePath);
}

bool TestUniverse::warpPlayer(WorldId worldId) {
  m_client->warpPlayer(WarpToWorld(worldId), true);
  for (unsigned i = 0; i < 1200; ++i) {
    m_client->update(0.016f);
    Thread::sleep(16);

    bool teleporting = m_mainPlayer->isTeleporting();
    bool hasWorldClient = static_cast<bool>(m_client->worldClient());
    if (!hasWorldClient)
      continue;

    bool worldEmpty = m_client->playerWorld().empty();
    if (!teleporting && !worldEmpty)
      break;
  }

  if (m_mainPlayer->isTeleporting() || m_client->playerWorld().empty() || !m_client->worldClient()) {
    ADD_FAILURE() << "Timed out while warping test player";
    return false;
  }

  return true;
}

WorldId TestUniverse::currentPlayerWorld() const {
  return m_client->clientContext()->playerWorldId();
}

void TestUniverse::update(unsigned times) {
  for (unsigned i = 0; i < times; ++i) {
    m_client->update(0.016f);
    Thread::sleep(16);
  }
}

List<Drawable> TestUniverse::currentClientDrawables() {
  WorldRenderData renderData;
  auto worldClient = m_client->worldClient();
  if (!worldClient) {
    ADD_FAILURE() << "World client is not available";
    return {};
  }

  worldClient->centerClientWindowOnPlayer(m_clientWindowSize);
  worldClient->render(renderData, 0);

  List<Drawable> drawables;
  for (auto& ed : renderData.entityDrawables) {
    for (auto& p : ed.layers)
      drawables.appendAll(std::move(p.second));
  }

  return drawables;
}

}// namespace Star
