#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarEither.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

#include "StarTestUniverse.hpp"
#include "gtest/gtest.h"


import star.celestial_database;

using namespace Star;

void validateWorld(TestUniverse& testUniverse) {
  testUniverse.update(100);

  // Just make sure the test world draws something for now, this will grow to
  // include more than this.
  EXPECT_GE(testUniverse.currentClientDrawables().size(), 1u) << strf("world: {}", testUniverse.currentPlayerWorld());

  auto assets = Root::singleton().assets();
  for (auto const& drawable : testUniverse.currentClientDrawables()) {
    if (drawable.isImage())
      assets->image(drawable.imagePart().image);
  }
}

TEST(SpawnTest, RandomCelestialWorld) {
  CelestialMasterDatabase celestialDatabase;
  Maybe<CelestialCoordinate> celestialWorld = celestialDatabase.findRandomWorld(10, 50, [&](CelestialCoordinate const& coord) {
      return celestialDatabase.parameters(coord)->isVisitable();
    });
  ASSERT_TRUE((bool)celestialWorld);

  TestUniverse testUniverse(Vec2U(100, 100));
  WorldId worldId = CelestialWorldId(*celestialWorld);
  testUniverse.warpPlayer(worldId);
  EXPECT_EQ(testUniverse.currentPlayerWorld(), worldId);
  validateWorld(testUniverse);
}

TEST(SpawnTest, RandomInstanceWorld) {
  auto& root = Root::singleton();
  StringList instanceWorlds = root.assets()->json("/instance_worlds.config").toObject().keys();
  ASSERT_GT(instanceWorlds.size(), 0u);
  WorldId instanceWorld = InstanceWorldId(Random::randFrom(instanceWorlds));

  TestUniverse testUniverse(Vec2U(100, 100));
  testUniverse.warpPlayer(instanceWorld);
  EXPECT_EQ(testUniverse.currentPlayerWorld(), instanceWorld);
  validateWorld(testUniverse);
}
