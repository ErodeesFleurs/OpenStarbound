#include "StarRoot.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(RootTest, All) {
  auto root = Root::singletonPtr();
  EXPECT_TRUE(root);

  EXPECT_TRUE(static_cast<bool>(root->assets()));
  EXPECT_TRUE(static_cast<bool>(root->objectDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->plantDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->projectileDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->monsterDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->npcDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->playerFactory()));
  EXPECT_TRUE(static_cast<bool>(root->entityFactory()));
  EXPECT_TRUE(static_cast<bool>(root->nameGenerator()));
  EXPECT_TRUE(static_cast<bool>(root->itemDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->materialDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->terrainDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->biomeDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->liquidsDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->statusEffectDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->damageDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->particleDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->effectSourceDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->functionDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->treasureDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->dungeonDefinitions()));
  EXPECT_TRUE(static_cast<bool>(root->emoteProcessor()));
  EXPECT_TRUE(static_cast<bool>(root->speciesDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->imageMetadataDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->versioningDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->questTemplateDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->aiDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->techDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->codexDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->stagehandDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->behaviorDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->tenantDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->danceDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->spawnTypeDatabase()));

  root->reload();

  EXPECT_TRUE(static_cast<bool>(root->assets()));
  EXPECT_TRUE(static_cast<bool>(root->objectDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->plantDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->projectileDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->monsterDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->npcDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->playerFactory()));
  EXPECT_TRUE(static_cast<bool>(root->entityFactory()));
  EXPECT_TRUE(static_cast<bool>(root->nameGenerator()));
  EXPECT_TRUE(static_cast<bool>(root->itemDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->materialDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->terrainDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->biomeDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->liquidsDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->statusEffectDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->damageDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->particleDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->effectSourceDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->functionDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->treasureDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->dungeonDefinitions()));
  EXPECT_TRUE(static_cast<bool>(root->emoteProcessor()));
  EXPECT_TRUE(static_cast<bool>(root->speciesDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->imageMetadataDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->versioningDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->questTemplateDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->aiDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->techDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->codexDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->stagehandDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->behaviorDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->tenantDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->danceDatabase()));
  EXPECT_TRUE(static_cast<bool>(root->spawnTypeDatabase()));
}
