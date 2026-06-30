#include "StarPlantDatabase.hpp"
#include "StarTestRoot.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(PlantTest, SkippedBranchesUseBranchAttachmentHeight) {
  auto plantDatabase = testRoot().plantDatabase();
  auto variant = plantDatabase->buildTreeVariant("roottree", 0.0f, "bubbles", 0.0f);

  for (uint64_t seed = 0; seed != 256; ++seed)
    EXPECT_NO_THROW((void)plantDatabase->createPlant(variant, seed)) << "seed: " << seed;
}
