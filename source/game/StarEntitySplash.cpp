#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarJsonExtra.hpp"
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
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;
#include "StarRoot.hpp"

import star.liquids_database;

namespace Star {

EntitySplashConfig::EntitySplashConfig() {}

EntitySplashConfig::EntitySplashConfig(Json const& config) {
  splashSpeedMin = config.get("splashSpeedMin").toFloat();
  splashMinWaterLevel = config.get("splashMinWaterLevel").toFloat();
  splashBottomSensor = jsonToVec2F(config.get("splashBottomSensor"));
  splashTopSensor = jsonToVec2F(config.get("splashTopSensor"));
  numSplashParticles = config.get("numSplashParticles").toInt();
  splashYVelocityFactor = config.get("splashYVelocityFactor").toFloat();
  splashParticle = Particle(config.get("splashParticle").toObject());
  splashParticleVariance = Particle(config.get("splashParticleVariance").toObject());
}

List<Particle> EntitySplashConfig::doSplash(Vec2F position, Vec2F velocity, World* world) const {
  List<Particle> particles;
  if (std::fabs(velocity[1]) >= splashSpeedMin) {
    auto liquidDb = Root::singleton().liquidsDatabase();
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
        newSplashParticle.applyVariance(splashParticleVariance);
        particles.append(newSplashParticle);
      }
    }
  }
  return particles;
}

}
