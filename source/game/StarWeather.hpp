#pragma once

#include "StarNetElementSystem.hpp"
#include "StarWeatherTypes.hpp"
#include "StarWorldGeometry.hpp"
#include "StarAssets.hpp"
#include "StarBiomeDatabase.hpp"
#include "StarProjectileDatabase.hpp"

namespace Star {

class Clock;
using ClockConstPtr = SharedPtr<Clock const>;
class Projectile;
using ProjectilePtr = SharedPtr<Projectile>;

// Callback used to determine whether weather effects should be spawned in
// the given tile location.  Other checks that enable / disable weather such as
// whether or not the region is below the underground level are performed
// separately of this, this is just to check the actual tile data.
using WeatherEffectsActiveQuery = function<bool(Vec2I)>;

class ServerWeather {
public:
  ServerWeather();

  void setup(AssetsConstPtr assets, WeatherPool weatherPool, float undergroundLevel, WorldGeometry worldGeometry,
      WeatherEffectsActiveQuery weatherEffectsActiveQuery, BiomeDatabaseConstPtr biomeDatabase, ProjectileDatabaseConstPtr projectileDatabase);

  void setReferenceClock(ClockConstPtr referenceClock = {});

  void setClientVisibleRegions(List<RectI> regions);

  [[nodiscard]] pair<ByteArray, uint64_t> writeUpdate(uint64_t fromVersion = 0, NetCompatibilityRules rules = {});

  void update(double dt);

  // Immediately sets the active weather index. If the index is NPos or out of
  // range, weather is cleared.  If force is true, weather will not automatically
  // change until setWeatherIndex/SetWeather is called again.
  void setWeatherIndex(size_t weatherIndex, bool force = false);
  // Immediately sets the active weather type by name. If not found, weather is
  // cleared.  Behavior of |force| is the same as above.
  void setWeather(String const& weatherName, bool force = false);

  [[nodiscard]] StringList weatherList() const;

  // Set or clear forcing without changing the current weather
  void forceWeather(bool force);


  [[nodiscard]] float wind() const;
  [[nodiscard]] float weatherIntensity() const;

  [[nodiscard]] StringList statusEffects() const;

  [[nodiscard]] List<ProjectilePtr> pullNewProjectiles();

private:
  void setNetStates();

  void spawnWeatherProjectiles(float dt);

  WeatherPool m_weatherPool;
  AssetsConstPtr m_assets;
  BiomeDatabaseConstPtr m_biomeDatabase;
  ProjectileDatabaseConstPtr m_projectileDatabase;
  float m_undergroundLevel = 0.0f;
  WorldGeometry m_worldGeometry;
  WeatherEffectsActiveQuery m_weatherEffectsActiveQuery;

  List<RectI> m_clientVisibleRegions;

  size_t m_currentWeatherIndex = NPos;
  Maybe<WeatherType> m_currentWeatherType;
  float m_currentWeatherIntensity = 0.0f;
  float m_currentWind = 0.0f;

  bool m_forceWeather = false;

  ClockConstPtr m_referenceClock;
  Maybe<double> m_clockTrackingTime;

  double m_currentTime = 0.0;
  double m_lastWeatherChangeTime = 0.0;
  double m_nextWeatherChangeTime = 0.0;

  List<ProjectilePtr> m_newProjectiles;

  NetElementTopGroup m_netGroup;
  NetElementBytes m_weatherPoolNetState;
  NetElementFloat m_undergroundLevelNetState;
  NetElementSize m_currentWeatherIndexNetState;
  NetElementFloat m_currentWeatherIntensityNetState;
  NetElementFloat m_currentWindNetState;
};

class ClientWeather {
public:
  ClientWeather();

  void setup(WorldGeometry worldGeometry, WeatherEffectsActiveQuery weatherEffectsActiveQuery, BiomeDatabaseConstPtr biomeDatabase);

  void readUpdate(ByteArray data, NetCompatibilityRules rules);

  void setVisibleRegion(RectI visibleRegion);

  void update(double dt);

  [[nodiscard]] float wind() const;
  [[nodiscard]] float weatherIntensity() const;

  [[nodiscard]] StringList statusEffects() const;

  [[nodiscard]] List<Particle> pullNewParticles();
  [[nodiscard]] StringList weatherTrackOptions() const;

private:
  void getNetStates();

  void spawnWeatherParticles(RectF newClientRegion, float dt);

  WeatherPool m_weatherPool;
  BiomeDatabaseConstPtr m_biomeDatabase;
  float m_undergroundLevel = 0.0f;
  WorldGeometry m_worldGeometry;
  WeatherEffectsActiveQuery m_weatherEffectsActiveQuery;

  size_t m_currentWeatherIndex = NPos;
  Maybe<WeatherType> m_currentWeatherType;
  float m_currentWeatherIntensity = 0.0f;
  float m_currentWind = 0.0f;

  double m_currentTime = 0.0;
  RectI m_visibleRegion;

  List<Particle> m_particles;
  RectF m_lastParticleVisibleRegion;

  NetElementTopGroup m_netGroup;
  NetElementBytes m_weatherPoolNetState;
  NetElementFloat m_undergroundLevelNetState;
  NetElementSize m_currentWeatherIndexNetState;
  NetElementFloat m_currentWeatherIntensityNetState;
  NetElementFloat m_currentWindNetState;
};

}
