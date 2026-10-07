#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;
#include "StarColor.hpp"
#include "StarBiMap.hpp"
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
import star.directives;
import star.asset_path;
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarIdMap.hpp"
#include "StarSet.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarStrongTypedef.hpp"
#include "StarList.hpp"

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.sky_types;
import star.celestial_coordinate;
import star.sky_parameters;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_damage;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.celestial_types;
import star.world_template;

#include "gtest/gtest.h"


import star.biome_database;

using namespace Star;

namespace {

static WorldTemplate makeGardenWorld(TerrestrialWorldParametersPtr parameters, uint64_t seed) {
  return WorldTemplate(parameters, SkyParameters(), seed);
}

static WorldTemplate::WeatherLayer const* layerAt(WorldTemplate const& world, int height) {
  return world.weatherLayerAt({0, height});
}

}

TEST(WeatherDomainTest, DefaultTerrestrialOwnership) {
  uint64_t seed = 1234;
  auto parameters = generateTerrestrialWorldParameters("garden", "small", seed);
  auto world = makeGardenWorld(parameters, seed);

  auto surface = layerAt(world, parameters->surfaceLayer.layerMinHeight);
  auto atmosphere = layerAt(world, parameters->atmosphereLayer.layerMinHeight);
  auto subsurface = layerAt(world, parameters->subsurfaceLayer.layerMinHeight);
  auto space = layerAt(world, parameters->spaceLayer.layerMinHeight);
  auto core = layerAt(world, parameters->coreLayer.layerMinHeight);

  ASSERT_NE(surface, nullptr);
  ASSERT_NE(atmosphere, nullptr);
  ASSERT_NE(subsurface, nullptr);
  ASSERT_NE(space, nullptr);
  ASSERT_NE(core, nullptr);
  EXPECT_EQ(surface->domain, Maybe<String>(String("surface")));
  EXPECT_EQ(atmosphere->domain, surface->domain);
  EXPECT_EQ(subsurface->domain, surface->domain);
  EXPECT_FALSE(space->domain);
  EXPECT_FALSE(core->domain);

  for (auto const& underground : parameters->undergroundLayers) {
    auto layer = layerAt(world, underground.layerMinHeight);
    ASSERT_NE(layer, nullptr);
    EXPECT_FALSE(layer->domain);
  }

  auto surfaceDomain = world.weatherDomain("surface");
  ASSERT_NE(surfaceDomain, nullptr);
  EXPECT_EQ(surfaceDomain->effectsMinHeight, parameters->subsurfaceLayer.layerMinHeight);
}

TEST(WeatherDomainTest, ExplicitLayerWeatherCreatesIndependentDomain) {
  uint64_t seed = 5678;
  auto parameters = generateTerrestrialWorldParameters("garden", "small", seed);
  parameters->atmosphereLayer.primaryRegion.biome = "garden";
  auto world = makeGardenWorld(parameters, seed);

  auto atmosphere = layerAt(world, parameters->atmosphereLayer.layerMinHeight);
  ASSERT_NE(atmosphere, nullptr);
  EXPECT_EQ(atmosphere->domain, Maybe<String>(String("atmosphere")));

  auto atmosphereDomain = world.weatherDomain("atmosphere");
  ASSERT_NE(atmosphereDomain, nullptr);
  EXPECT_FALSE(atmosphereDomain->pool.empty());
  EXPECT_EQ(atmosphereDomain->effectsMinHeight, parameters->atmosphereLayer.layerMinHeight);
}

TEST(WeatherDomainTest, WeatherPropertyPresenceIsDistinctFromOmission) {
  auto biomes = Root::singleton().biomeDatabase();
  EXPECT_TRUE(biomes->biomeHasWeather("garden"));
  EXPECT_FALSE(biomes->biomeHasWeather("atmosphere"));
}
