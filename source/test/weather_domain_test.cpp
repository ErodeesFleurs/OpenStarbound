import star.json;
import star.string;
import star.ordered_map;
import star.thread;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;
import star.color;
import star.bi_map;
import star.either;
import star.variant;
import star.game_types;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.lru_cache;
import star.perlin;
import star.id_map;
import star.set;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.sky_types;
import star.celestial_coordinate;
import star.sky_parameters;
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

static WorldTemplate makeGardenWorld(std::shared_ptr<::Star::TerrestrialWorldParameters> parameters, std::uint64_t seed) {
  return WorldTemplate(parameters, SkyParameters(), seed);
}

static WorldTemplate::WeatherLayer const* layerAt(WorldTemplate const& world, int height) {
  return world.weatherLayerAt({0, height});
}

}

TEST(WeatherDomainTest, DefaultTerrestrialOwnership) {
  std::uint64_t seed = 1234;
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
  std::uint64_t seed = 5678;
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
