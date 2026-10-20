import star.json;
import star.json_extra;
import star.id_map;
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
import star.variant;
import star.casting;
import star.vector;
import star.game_types;
import star.liquid_types;
import star.strong_typedef;
import star.rpc_promise;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;

import star.lua_root;
import star.animation;
import star.particle;

import star.entity_splash;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.asset_source;
import star.assets;
import star.configuration;
import star.root;

import star.liquids_database;

namespace Star {

EntitySplashConfig::EntitySplashConfig() {}

EntitySplashConfig::EntitySplashConfig(Root& root, Json const& config) {
  splashSpeedMin = config.get("splashSpeedMin").toFloat();
  splashMinWaterLevel = config.get("splashMinWaterLevel").toFloat();
  splashBottomSensor = jsonToVec2F(config.get("splashBottomSensor"));
  splashTopSensor = jsonToVec2F(config.get("splashTopSensor"));
  numSplashParticles = config.get("numSplashParticles").toInt();
  splashYVelocityFactor = config.get("splashYVelocityFactor").toFloat();
  splashParticle = Particle(root, config.get("splashParticle").toObject());
  splashParticleVariance = Particle(root, config.get("splashParticleVariance").toObject());
}

List<Particle> EntitySplashConfig::doSplash(Vec2F position, Vec2F velocity, World* world) const {
  List<Particle> particles;
  if (std::fabs(velocity[1]) >= splashSpeedMin) {
    auto liquidDb = world->root().liquidsDatabase();
    Vec2I bottom = Vec2I::floor(position + splashBottomSensor);
    Vec2I top = Vec2I::floor(position + splashTopSensor);
    if (world->liquidLevel(bottom).level - world->liquidLevel(top).level >= splashMinWaterLevel) {
      LiquidId liquidType;
      auto bottomLiquid = world->liquidLevel(bottom);
      auto topLiquid = world->liquidLevel(top);
      if (bottomLiquid.level > 0 && (int)bottomLiquid.liquid)
        liquidType = bottomLiquid.liquid;
      else
        liquidType = topLiquid.liquid;
      Color particleColor = Color::rgba(liquidDb->liquidSettings(liquidType)->liquidColor);
      for (int i = 0; i < numSplashParticles; ++i) {
        Particle newSplashParticle = splashParticle;
        newSplashParticle.position = position;
        newSplashParticle.velocity[1] = std::fabs(velocity[1]) * splashYVelocityFactor;
        newSplashParticle.color = particleColor;
        newSplashParticle.applyVariance(world->root(), splashParticleVariance);
        particles.append(newSplashParticle);
      }
    }
  }
  return particles;
}

}
