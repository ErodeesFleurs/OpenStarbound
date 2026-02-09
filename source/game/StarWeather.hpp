#pragma once

#include "StarConfig.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarWeatherTypes.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

class Clock;
class Projectile;

// Callback used to determine whether weather effects should be spawned in
// the given tile location.  Other checks that enable / disable weather such as
// whether or not the region is below the underground level are performed
// separately of this, this is just to check the actual tile data.
using WeatherEffectsActiveQuery = std::function<bool(Vec2I)>;

class ServerWeather {
public:
  ServerWeather();

  void setup(WeatherPool weatherPool, float undergroundLevel, WorldGeometry worldGeometry,
             WeatherEffectsActiveQuery weatherEffectsActiveQuery);

  void setReferenceClock(ConstPtr<Clock> referenceClock = {});

  void setClientVisibleRegions(List<RectI> regions);

  auto writeUpdate(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t>;

  void update(double dt);

  // Immediately sets the active weather index. If the index is std::numeric_limits<std::size_t>::max() or out of
  // range, weather is cleared.  If force is true, weather will not automatically
  // change until setWeatherIndex/SetWeather is called again.
  void setWeatherIndex(std::size_t weatherIndex, bool force = false);
  // Immediately sets the active weather type by name. If not found, weather is
  // cleared.  Behavior of |force| is the same as above.
  void setWeather(String const& weatherName, bool force = false);

  auto weatherList() const -> StringList;

  // Set or clear forcing without changing the current weather
  void forceWeather(bool force);

  auto wind() const -> float;
  auto weatherIntensity() const -> float;

  auto statusEffects() const -> StringList;

  auto pullNewProjectiles() -> List<Ptr<Projectile>>;

private:
  void setNetStates();

  void spawnWeatherProjectiles(float dt);

  WeatherPool m_weatherPool;
  float m_undergroundLevel;
  WorldGeometry m_worldGeometry;
  WeatherEffectsActiveQuery m_weatherEffectsActiveQuery;

  List<RectI> m_clientVisibleRegions;

  std::size_t m_currentWeatherIndex;
  std::optional<WeatherType> m_currentWeatherType;
  float m_currentWeatherIntensity;
  float m_currentWind;

  bool m_forceWeather;

  ConstPtr<Clock> m_referenceClock;
  std::optional<double> m_clockTrackingTime;

  double m_currentTime;
  double m_lastWeatherChangeTime;
  double m_nextWeatherChangeTime;

  List<Ptr<Projectile>> m_newProjectiles;

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

  void setup(WorldGeometry worldGeometry, WeatherEffectsActiveQuery weatherEffectsActiveQuery);

  void readUpdate(ByteArray data, NetCompatibilityRules rules);

  void setVisibleRegion(RectI visibleRegion);

  void update(double dt);

  auto wind() const -> float;
  auto weatherIntensity() const -> float;

  auto statusEffects() const -> StringList;

  auto pullNewParticles() -> List<Particle>;
  auto weatherTrackOptions() const -> StringList;

private:
  void getNetStates();

  void spawnWeatherParticles(RectF newClientRegion, float dt);

  WeatherPool m_weatherPool;
  float m_undergroundLevel;
  WorldGeometry m_worldGeometry;
  WeatherEffectsActiveQuery m_weatherEffectsActiveQuery;

  std::size_t m_currentWeatherIndex;
  std::optional<WeatherType> m_currentWeatherType;
  float m_currentWeatherIntensity;
  float m_currentWind;

  double m_currentTime;
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

}// namespace Star
