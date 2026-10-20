import std;
import star.json;
import star.random;
import star.weighted_pool;
import star.color;
import star.bi_map;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.animation;
import star.particle;
import star.weather_types;
import star.world_geometry;
import star.vector;
import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.game_test;
import star.root;

#include "gtest/gtest.h"

import star.weather;

using namespace Star;

namespace {

static Json weatherConfig() {
  return JsonObject{
    {"name", "testWeather"},
    {"particles", JsonArray()},
    {"projectiles", JsonArray()},
    {"duration", JsonArray{40, 100}}};
}

}// namespace

const GameTestRegistration weatherNoParallax("WeatherTest", "NoParallax", [](GameTest& fixture) {
  WeatherType weather(fixture.root(), weatherConfig());

  EXPECT_FALSE(weather.parallax);
  EXPECT_TRUE(weather.toJson().get("parallax").isNull());
  EXPECT_FALSE(WeatherType(fixture.root(), weather.toJson()).parallax);
});

const GameTestRegistration weatherEmptyParallax("WeatherTest", "EmptyParallax", [](GameTest& fixture) {
  WeatherType weather(fixture.root(), weatherConfig().set("parallax", ""));

  EXPECT_FALSE(weather.parallax);
});

const GameTestRegistration weatherShorthandParallax("WeatherTest", "ShorthandParallax", [](GameTest& fixture) {
  WeatherType weather(fixture.root(), weatherConfig().set("parallax", "rain"));

  ASSERT_TRUE(weather.parallax);
  EXPECT_EQ(*weather.parallax, "/parallax/weather/rain.parallax");

  WeatherType roundTrip(fixture.root(), weather.toJson());
  ASSERT_TRUE(roundTrip.parallax);
  EXPECT_EQ(*roundTrip.parallax, "/parallax/weather/rain.parallax");
});

const GameTestRegistration weatherAbsoluteParallax("WeatherTest", "AbsoluteParallax", [](GameTest& fixture) {
  WeatherType weather(fixture.root(), weatherConfig().set("parallax", "/custom/weather/storm.parallax"));

  ASSERT_TRUE(weather.parallax);
  EXPECT_EQ(*weather.parallax, "/custom/weather/storm.parallax");
});

const GameTestRegistration weatherCompleteStateRestoresAfterLocalClear("WeatherTest", "CompleteStateRestoresAfterLocalClear", [](GameTest& fixture) {
  WeatherPool pool(WeatherPool::ItemsList{{1.0, "rain"}});
  WorldGeometry geometry(Vec2U(1000, 1000));

  ServerWeather server(fixture.root());
  server.setup(pool, 0, geometry, {});
  server.setWeather("rain", true);
  auto update = server.writeUpdate(0).first;

  ClientWeather client(fixture.root());
  client.setup(geometry, {});
  client.setVisibleRegion({0, 100, 100, 200});
  client.readUpdate(update, {});
  client.update(0);
  EXPECT_FALSE(client.statusEffects().empty());

  client.clear();
  EXPECT_TRUE(client.statusEffects().empty());

  client.readUpdate(update, {});
  client.setVisibleRegion({0, 100, 100, 200});
  client.update(0);
  EXPECT_FALSE(client.statusEffects().empty());
});

const GameTestRegistration parallaxWindSettingsDefaultOff("ParallaxTest", "WindSettingsDefaultOff", [](GameTest&) {
  ParallaxLayer layer;
  ParallaxLayer restored(layer.store());

  EXPECT_FALSE(restored.followsWind);
  EXPECT_FALSE(restored.windSpeedMultiplier);
});

const GameTestRegistration parallaxWindSettingsRoundTrip("ParallaxTest", "WindSettingsRoundTrip", [](GameTest&) {
  ParallaxLayer layer;
  layer.followsWind = true;
  layer.windSpeedMultiplier = 0.25f;
  ParallaxLayer restored(layer.store());

  EXPECT_TRUE(restored.followsWind);
  ASSERT_TRUE(restored.windSpeedMultiplier);
  EXPECT_FLOAT_EQ(*restored.windSpeedMultiplier, 0.25f);
});
