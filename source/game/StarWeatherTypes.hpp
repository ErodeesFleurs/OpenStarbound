#pragma once

#include "StarAssets.hpp"
#include "StarWeightedPool.hpp"
#include "StarParticle.hpp"

namespace Star {

class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

struct WeatherType {
  struct ParticleConfig {
    Particle particle;
    float density{};
    bool autoRotate = false;
  };

  struct ProjectileConfig {
    String projectile;
    Json parameters;
    Vec2F velocity;
    float ratePerX{};
    int spawnAboveRegion{};
    int spawnHorizontalPad{};
    float windAffectAmount = 0.0f;
  };

  WeatherType() = default;
  WeatherType(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json config, String path = String());

  [[nodiscard]] Json toJson() const;

  String name;

  List<ParticleConfig> particles;
  List<ProjectileConfig> projectiles;
  StringList statusEffects;

  float maximumWind = 0.0f;
  Vec2F duration;
  StringList weatherNoises;
};

using WeatherPool = WeightedPool<String>;

DataStream& operator>>(DataStream& ds, WeatherType& weatherType);
DataStream& operator<<(DataStream& ds, WeatherType const& weatherType);
}
