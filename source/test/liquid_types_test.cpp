import star.liquid_types;
#include "gtest/gtest.h"

using namespace Star;

TEST(LiquidStoreTest, OverfullLevelSaturatesNetworkUpdates) {
  auto store = LiquidStore::filled(1, 0.5f);
  auto update = store.update(1, 1.1f, 3.0f);
  ASSERT_TRUE(update);
  EXPECT_EQ(update->liquid, 1);
  EXPECT_EQ(update->level, 255);
  EXPECT_EQ(store.netUpdate().level, 255);
  EXPECT_FLOAT_EQ(store.level, 1.1f);
  EXPECT_FLOAT_EQ(store.pressure, 3.0f);

  // Both overfull levels have the same wire value; only physics changes.
  EXPECT_FALSE(store.update(1, 1.3f, 4.0f));
  EXPECT_FLOAT_EQ(store.level, 1.3f);
  EXPECT_FLOAT_EQ(store.pressure, 4.0f);
}
