import star.json;
import star.string;
import star.ordered_map;
import star.either;
import star.vector;
import star.variant;
import star.game_types;
import star.color;
import star.bi_map;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
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
