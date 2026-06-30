#include "StarUniverseServer.hpp"
#include "StarRoot.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(ServerTest, Run) {
  auto& root = Root::singleton();
  UniverseServer server(root.toStoragePath("universe"), root.assets(), root.configuration(), root.materialDatabase(), root.imageMetadataDatabase(), root.itemDatabase(), root.objectDatabase(), root.projectileDatabase(), root.plantDatabase(), root.treasureDatabase(), root.npcDatabase(), root.monsterDatabase(), root.spawnTypeDatabase(), root.stagehandDatabase(), root.vehicleDatabase(), root.speciesDatabase(), root.entityFactory(), root.liquidsDatabase(), root.biomeDatabase(), root.nameGenerator(), root.versioningDatabase(), root.functionDatabase(), root.effectSourceDatabase(), root.particleDatabase(), root.techDatabase(), root.statusEffectDatabase());
  server.start();
  server.stop();
  server.join();
}
