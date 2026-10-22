import std;
import star.json;
import star.string;
import star.strong_typedef;
import star.data_stream;
import star.id_map;
import star.status_types;
import star.either;

#include "gtest/gtest.h"
import star.stat_collection;
import star.stat_set;

import star.math_common;
using namespace Star;

bool withinAmount(float value, float target, float amount) {
  return std::fabs(value - target) <= amount;
}

TEST(StatTest, Set) {
  StatSet stats;

  ASSERT_TRUE(stats.addStat("MaxHealth", 100.0f));
  ASSERT_TRUE(stats.addStat("HealthRegen", 0.0f));
  ASSERT_TRUE(stats.addStat("MaxEnergy", 100.0f));
  ASSERT_TRUE(stats.addStat("EnergyRegen", 0.0f));

  EXPECT_EQ(stats.statBaseValue("MaxHealth"), 100.0f);
  EXPECT_EQ(stats.statEffectiveValue("MaxHealth"), 100.0f);
  EXPECT_EQ(stats.statBaseValue("MaxEnergy"), 100.0f);
  EXPECT_EQ(stats.statEffectiveValue("MaxEnergy"), 100.0f);

  ASSERT_TRUE(stats.addResource("Health", String("MaxHealth"), String("HealthRegen")));
  ASSERT_TRUE(stats.setResourceValue("Health", 110.0f));
  ASSERT_TRUE(stats.addResource("Energy", String("MaxEnergy"), 0.0f));
  ASSERT_TRUE(stats.setResourceValue("Energy", 112.0f));
  ASSERT_TRUE(stats.addResource("Experience"));

  EXPECT_EQ(stats.resourceValue("Health"), 100.0f);
  EXPECT_EQ(stats.resourceValue("Energy"), 100.0f);
  EXPECT_EQ(stats.resourceValue("Experience"), 0.0f);

  ASSERT_TRUE(stats.setStatBaseValue("MaxHealth", 200.0f));
  EXPECT_TRUE(withinAmount(stats.resourceValue("Health"), 200.0f, 0.0001f));

  ASSERT_TRUE(stats.modifyResourceValue("Health", -100.0f));
  EXPECT_TRUE(withinAmount(stats.resourceValue("Health"), 100.0f, 0.0001f));

  ASSERT_TRUE(stats.modifyResourceValue("Experience", -100.0f));
  EXPECT_EQ(stats.resourceValue("Experience"), 0.0f);
  ASSERT_TRUE(stats.modifyResourceValue("Experience", 1000.0f));
  EXPECT_TRUE(withinAmount(stats.resourceValue("Experience"), 1000.0f, 0.0001f));

  ASSERT_TRUE(stats.setStatBaseValue("HealthRegen", 100.0f));
  stats.update(0.01f);
  EXPECT_TRUE(withinAmount(stats.resourceValue("Health"), 101.0f, 0.0001f));
  stats.update(2.0f);
  EXPECT_TRUE(withinAmount(stats.resourceValue("Health"), 200.0f, 0.0001f));
  ASSERT_TRUE(stats.setStatBaseValue("HealthRegen", 0.0f));

  auto idResult = stats.addStatModifierGroup({StatValueModifier{"HealthRegen", -50.0f}});
  ASSERT_TRUE(idResult);
  auto id = *idResult;
  stats.update(1.0f);
  EXPECT_TRUE(withinAmount(stats.resourceValue("Health"), 150.0f, 0.0001f));
  stats.removeStatModifierGroup(id);
  stats.update(1.0f);
  EXPECT_TRUE(withinAmount(stats.resourceValue("Health"), 150.0f, 0.0001f));

  idResult = stats.addStatModifierGroup({StatBaseMultiplier{"MaxHealth", 1.1f},
      StatEffectiveMultiplier{"MaxHealth", 1.1f},
      StatEffectiveMultiplier{"MaxHealth", 1.2f},
      StatValueModifier{"MaxHealth", 50.0}});
  ASSERT_TRUE(idResult);
  id = *idResult;
  // 200 (base) + 20 (base perc mod) + 50 (value mod) = 270 ...
  // * 1.1 (eff perc mod) * 1.2 (eff perc mod) = 356.4
  EXPECT_TRUE(withinAmount(stats.statEffectiveValue("MaxHealth"), 356.4f, 0.0001f));
  stats.removeStatModifierGroup(id);

  idResult = stats.addStatModifierGroup({StatBaseMultiplier{"MaxHealth", 1.5f}, StatBaseMultiplier{"MaxHealth", 1.5f}});
  ASSERT_TRUE(idResult);
  id = *idResult;
  // 200 (base) + 100 (base perc mod) + 100 (base perc mod) -- make sure base
  // perc mods do NOT stack
  // with each other
  EXPECT_TRUE(withinAmount(stats.statEffectiveValue("MaxHealth"), 400.0f, 0.0001f));
  stats.removeStatModifierGroup(id);

  EXPECT_FALSE(stats.isEffectiveStat("TempStat"));
  EXPECT_TRUE(withinAmount(stats.statEffectiveValue("TempStat"), 0.0f, 0.0001f));
  auto nextId = stats.addStatModifierGroup({StatValueModifier{"TempStat", 20}});
  ASSERT_TRUE(nextId);
  EXPECT_TRUE(withinAmount(stats.statEffectiveValue("TempStat"), 20.0f, 0.0001f));
  EXPECT_TRUE(stats.isEffectiveStat("TempStat"));
  stats.removeStatModifierGroup(*nextId);
  EXPECT_TRUE(withinAmount(stats.statEffectiveValue("TempStat"), 0.0f, 0.0001f));
  EXPECT_FALSE(stats.isEffectiveStat("TempStat"));
}
