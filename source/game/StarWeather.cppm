module;
import std;
import star.string;
import star.json;
import star.net_compatibility;
import star.byte_array;

import star.maybe;
import star.random;
import star.weighted_pool;
import star.color;
import star.bi_map;
import star.list;
import star.rect;
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

import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_system;

import star.vector;
namespace Star {

class Clock;
using ClockPtr = std::shared_ptr<Clock>;
using ClockConstPtr = std::shared_ptr<const Clock>;
using ClockWeakPtr = std::weak_ptr<Clock>;
using ClockConstWeakPtr = std::weak_ptr<const Clock>;
using ClockUPtr = std::unique_ptr<Clock>;
using ClockConstUPtr = std::unique_ptr<const Clock>;
class Projectile;
using ProjectilePtr = std::shared_ptr<Projectile>;
using ProjectileConstPtr = std::shared_ptr<const Projectile>;
using ProjectileWeakPtr = std::weak_ptr<Projectile>;
using ProjectileConstWeakPtr = std::weak_ptr<const Projectile>;
using ProjectileUPtr = std::unique_ptr<Projectile>;
using ProjectileConstUPtr = std::unique_ptr<const Projectile>;

class ServerWeather;
using ServerWeatherPtr = std::shared_ptr<ServerWeather>;
using ServerWeatherConstPtr = std::shared_ptr<const ServerWeather>;
using ServerWeatherWeakPtr = std::weak_ptr<ServerWeather>;
using ServerWeatherConstWeakPtr = std::weak_ptr<const ServerWeather>;
using ServerWeatherUPtr = std::unique_ptr<ServerWeather>;
using ServerWeatherConstUPtr = std::unique_ptr<const ServerWeather>;
class ClientWeather;
using ClientWeatherPtr = std::shared_ptr<ClientWeather>;
using ClientWeatherConstPtr = std::shared_ptr<const ClientWeather>;
using ClientWeatherWeakPtr = std::weak_ptr<ClientWeather>;
using ClientWeatherConstWeakPtr = std::weak_ptr<const ClientWeather>;
using ClientWeatherUPtr = std::unique_ptr<ClientWeather>;
using ClientWeatherConstUPtr = std::unique_ptr<const ClientWeather>;

// Callback used to determine whether weather effects should be spawned in
// the given tile location.  Other checks that enable / disable weather such as
// whether or not the region is below the underground level are performed
// separately of this, this is just to check the actual tile data.
typedef std::function<bool(Vec2I)> WeatherEffectsActiveQuery;

class ServerWeather {
public:
  ServerWeather();

  void setup(WeatherPool weatherPool, float undergroundLevel, WorldGeometry worldGeometry,
      WeatherEffectsActiveQuery weatherEffectsActiveQuery);

  void setReferenceClock(ClockConstPtr referenceClock = {});

  void setClientVisibleRegions(List<RectI> regions);

  std::pair<ByteArray, std::uint64_t> writeUpdate(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {});

  void update(double dt);

  // Immediately sets the active weather index. If the index is NPos or out of
  // range, weather is cleared.  If force is true, weather will not automatically
  // change until setWeatherIndex/SetWeather is called again.
  void setWeatherIndex(std::size_t weatherIndex, bool force = false);
  // Immediately sets the active weather type by name. If not found, weather is
  // cleared.  Behavior of |force| is the same as above.
  void setWeather(String const& weatherName, bool force = false);

  StringList weatherList() const;

  // Set or clear forcing without changing the current weather
  void forceWeather(bool force);


  float wind() const;
  float weatherIntensity() const;

  StringList statusEffects() const;

  List<ProjectilePtr> pullNewProjectiles();
  
  Maybe<String> activeWeather() const;

private:
  void setNetStates();

  void spawnWeatherProjectiles(float dt);

  WeatherPool m_weatherPool;
  float m_undergroundLevel;
  WorldGeometry m_worldGeometry;
  WeatherEffectsActiveQuery m_weatherEffectsActiveQuery;

  List<RectI> m_clientVisibleRegions;

  std::size_t m_currentWeatherIndex;
  Maybe<WeatherType> m_currentWeatherType;
  float m_currentWeatherIntensity;
  float m_currentWind;

  bool m_forceWeather;

  ClockConstPtr m_referenceClock;
  Maybe<double> m_clockTrackingTime;

  double m_currentTime;
  double m_lastWeatherChangeTime;
  double m_nextWeatherChangeTime;

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

  void setup(WorldGeometry worldGeometry, WeatherEffectsActiveQuery weatherEffectsActiveQuery);

  void readUpdate(ByteArray data, NetCompatibilityRules rules);

  void clear();

  void setVisibleRegion(RectI visibleRegion);

  void update(double dt);

  float wind() const;
  float weatherIntensity() const;

  StringList statusEffects() const;

  List<Particle> pullNewParticles();
  StringList weatherTrackOptions() const;
  Maybe<String> weatherParallax() const;
  
  Maybe<String> activeWeather() const;

private:
  void getNetStates();

  void spawnWeatherParticles(RectF newClientRegion, float dt);

  WeatherPool m_weatherPool;
  float m_undergroundLevel;
  WorldGeometry m_worldGeometry;
  WeatherEffectsActiveQuery m_weatherEffectsActiveQuery;

  std::size_t m_currentWeatherIndex;
  Maybe<WeatherType> m_currentWeatherType;
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

}

export module star.weather;

export namespace Star {
using ::Star::Clock;
using ::Star::ClockPtr;
using ::Star::ClockConstPtr;
using ::Star::ClockWeakPtr;
using ::Star::ClockConstWeakPtr;
using ::Star::ClockUPtr;
using ::Star::ClockConstUPtr;
using ::Star::Projectile;
using ::Star::ProjectilePtr;
using ::Star::ProjectileConstPtr;
using ::Star::ProjectileWeakPtr;
using ::Star::ProjectileConstWeakPtr;
using ::Star::ProjectileUPtr;
using ::Star::ProjectileConstUPtr;
using ::Star::ServerWeather;
using ::Star::ServerWeatherPtr;
using ::Star::ServerWeatherConstPtr;
using ::Star::ServerWeatherWeakPtr;
using ::Star::ServerWeatherConstWeakPtr;
using ::Star::ServerWeatherUPtr;
using ::Star::ServerWeatherConstUPtr;
using ::Star::ClientWeather;
using ::Star::ClientWeatherPtr;
using ::Star::ClientWeatherConstPtr;
using ::Star::ClientWeatherWeakPtr;
using ::Star::ClientWeatherConstWeakPtr;
using ::Star::ClientWeatherUPtr;
using ::Star::ClientWeatherConstUPtr;
using ::Star::WeatherEffectsActiveQuery;
}
