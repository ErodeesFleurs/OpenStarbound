module;

#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
import star.animation;
import star.particle;

namespace Star {

struct WeatherType {
  friend DataStream& operator>>(DataStream& ds, WeatherType& weatherType);
  friend DataStream& operator<<(DataStream& ds, WeatherType const& weatherType);
  struct ParticleConfig {
    Particle particle;
    float density;
    bool autoRotate;
  };

  struct ProjectileConfig {
    String projectile;
    Json parameters;
    Vec2F velocity;
    float ratePerX;
    int spawnAboveRegion;
    int spawnHorizontalPad;
    float windAffectAmount;
  };

  WeatherType();
  WeatherType(Json config, String path = String());

  Json toJson() const;

  String name;

  List<ParticleConfig> particles;
  List<ProjectileConfig> projectiles;
  StringList statusEffects;

  float maximumWind;
  Vec2F duration;
  StringList weatherNoises;

  // Resolved from local assets and intentionally excluded from legacy binary serialization.
  Maybe<String> parallax;
};

typedef WeightedPool<String> WeatherPool;

DataStream& operator>>(DataStream& ds, WeatherType& weatherType);
DataStream& operator<<(DataStream& ds, WeatherType const& weatherType);
}

export module star.weather_types;

export namespace Star {
  using ::Star::WeatherType;
  using ::Star::WeatherPool;
}
