#include "StarTestRoot.hpp"
#include "StarUniverseServer.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(ServerTest, Run) {
  auto& root = testRoot();
  auto luaRootServices = root.luaRootServices();
  UniverseServer server(root.toStoragePath("universe"), root.assets(), root.configuration(), root.materialDatabase(), root.imageMetadataDatabase(), root.itemDatabase(), root.objectDatabase(), root.projectileDatabase(), root.plantDatabase(), root.treasureDatabase(), root.npcDatabase(), root.monsterDatabase(), root.spawnTypeDatabase(), root.stagehandDatabase(), root.vehicleDatabase(), root.speciesDatabase(), root.entityFactory(), root.liquidsDatabase(), root.terrainDatabase(), root.biomeDatabase(), root.nameGenerator(), root.versioningDatabase(), root.functionDatabase(), root.effectSourceDatabase(), root.particleDatabase(), root.techDatabase(), root.statusEffectDatabase(), root.dungeonDefinitions(), root.behaviorDatabase(), luaRootServices, [&root]() {
    root.reload();
    root.fullyLoad();
  });
  server.start();
  server.stop();
  server.join();
}
