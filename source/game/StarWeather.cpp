import star.memory;
import star.json;
import star.data_stream_extra;
import star.ordered_map;
import star.rect;
import star.thread;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.version;
import star.time;
import star.id_map;
import star.variant;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.list;
import star.rpc_promise;
import star.lua;
import star.random;
import star.weighted_pool;
import star.color;
import star.bi_map;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;

import star.asset_source;
import star.assets;
import star.configuration;
import star.root;
import star.damage_types;
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
import star.lua_root;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.mobile_entity;
import star.scripted_entity;
import star.status_effect_entity;
import star.movement_controller;
import star.lua_components;
import star.effect_emitter;
import star.projectile;
import star.animation;
import star.particle;
import star.weather_types;
import star.world_geometry;

import star.weather;

import star.projectile_database;
import star.biome_database;


import star.data_stream_devices;

import star.net_compatibility;
import star.byte_array;
import star.math_common;
import star.algorithm;
namespace Star {

ServerWeather::ServerWeather(Root& root) : m_root(&root) {
  m_undergroundLevel = 0.0f;
  m_currentWeatherIndex = NPos;
  m_currentWeatherIntensity = 0.0f;
  m_currentWind = 0.0f;
  m_forceWeather = false;

  m_currentTime = 0.0;
  m_lastWeatherChangeTime = 0.0;
  m_nextWeatherChangeTime = 0.0;

  m_netGroup.addNetElement(&m_weatherPoolNetState);
  m_netGroup.addNetElement(&m_undergroundLevelNetState);
  m_netGroup.addNetElement(&m_currentWeatherIndexNetState);
  m_netGroup.addNetElement(&m_currentWeatherIntensityNetState);
  m_netGroup.addNetElement(&m_currentWindNetState);
}

void ServerWeather::setup(WeatherPool weatherPool, float undergroundLevel, WorldGeometry worldGeometry,
    WeatherEffectsActiveQuery weatherEffectsActiveQuery) {
  m_weatherPool = weatherPool;
  m_undergroundLevel = undergroundLevel;

  m_worldGeometry = worldGeometry;
  m_weatherEffectsActiveQuery = weatherEffectsActiveQuery;

  m_currentWeatherIndex = NPos;
  m_currentWeatherType = {};

  m_currentTime = 0.0;
  m_lastWeatherChangeTime = 0.0;
  m_nextWeatherChangeTime = 0.0;
}

void ServerWeather::setReferenceClock(std::shared_ptr<Clock const> referenceClock) {
  m_referenceClock = std::move(referenceClock);
  if (m_referenceClock)
    m_clockTrackingTime = m_referenceClock->time();
  else
    m_clockTrackingTime = {};
}

void ServerWeather::setClientVisibleRegions(List<RectI> regions) {
  m_clientVisibleRegions = std::move(regions);
}

std::pair<ByteArray, std::uint64_t> ServerWeather::writeUpdate(std::uint64_t fromVersion, NetCompatibilityRules rules) {
  setNetStates();
  return m_netGroup.writeNetState(fromVersion, rules);
}

void ServerWeather::update(double dt) {
  spawnWeatherProjectiles(dt);

  if (m_referenceClock) {
    double clockTime = m_referenceClock->time();
    if (!m_clockTrackingTime) {
      m_clockTrackingTime = clockTime;
    } else {
      // If our reference clock is set, and we have a valid tracking time, then
      // the dt should be driven by the reference clock.
      dt = clockTime - *m_clockTrackingTime;
      m_clockTrackingTime = clockTime;
    }
  }

  m_currentTime += dt;

  if (m_forceWeather) {
    if (m_currentWeatherType)
      m_currentWeatherIntensity = 1.0f;
    else
      m_currentWeatherIntensity = 0.0f;
  } else if (!m_weatherPool.empty()) {
    auto assets = m_root->assets();
    double weatherCooldownTime = assets->json("/weather.config:weatherCooldownTime").toDouble();
    double weatherWarmupTime = assets->json("/weather.config:weatherWarmupTime").toDouble();

    if (m_currentTime >= m_nextWeatherChangeTime) {
      m_currentWeatherIndex = m_weatherPool.selectIndex(m_root->random());
      if (m_currentWeatherIndex == NPos)
        m_currentWeatherType = {};
      else
        m_currentWeatherType = m_root->biomeDatabase()->weatherType(m_weatherPool.item(m_currentWeatherIndex));

      m_lastWeatherChangeTime = m_nextWeatherChangeTime;
      m_nextWeatherChangeTime = m_currentTime + m_root->random().randd(m_currentWeatherType.value().duration[0], m_currentWeatherType.value().duration[1]);

      // TODO: For now just set the wind at maximum either left or right, nothing exciting.
      m_currentWind = m_currentWeatherType.value().maximumWind * (m_root->random().randb() ? 1 : -1);
    }

    m_currentWeatherIntensity = std::min(clamp((m_currentTime - m_lastWeatherChangeTime) / weatherWarmupTime, 0.0, 1.0),
        clamp((m_nextWeatherChangeTime - m_currentTime) / weatherCooldownTime, 0.0, 1.0));

  } else {
    m_currentWeatherIndex = NPos;
    m_currentWeatherType = {};
  }
}

float ServerWeather::wind() const {
  return m_currentWind * m_currentWeatherIntensity;
}

float ServerWeather::weatherIntensity() const {
  return m_currentWeatherIntensity;
}

StringList ServerWeather::statusEffects() const {
  if (m_currentWeatherType && m_currentWeatherIntensity == 1.0)
    return m_currentWeatherType->statusEffects;
  return {};
}

List<std::shared_ptr<Projectile>> ServerWeather::pullNewProjectiles() {
  return take(m_newProjectiles);
}

std::optional<String> ServerWeather::activeWeather() const {
  if (m_currentWeatherIndex == NPos)
    return {};
  else
    return m_weatherPool.item(m_currentWeatherIndex);
}

StringList ServerWeather::weatherList() const {
  StringList weatherList;
  for (std::size_t i = 0; i < m_weatherPool.size(); ++i)
    weatherList.append(m_weatherPool.item(i));
  return weatherList;
}

void ServerWeather::setWeather(String const& weatherName, bool force) {
  std::size_t index = NPos;
  for (std::size_t i = 0; i < m_weatherPool.size(); ++i) {
    if (m_weatherPool.item(i) == weatherName) {
      index = i;
      break;
    }
  }

  setWeatherIndex(index, force);
}

void ServerWeather::setWeatherIndex(std::size_t weatherIndex, bool force) {
  m_forceWeather = force;
  if (weatherIndex == NPos || weatherIndex >= m_weatherPool.size()) {
    m_currentWeatherIndex = NPos;
    m_currentWeatherType = {};
    m_currentWeatherIntensity = 0.0f;
    m_currentWind = 0.0f;
  } else {
    m_currentWeatherIndex = weatherIndex;
    m_currentWeatherType =
      m_root->biomeDatabase()->weatherType(m_weatherPool.item(m_currentWeatherIndex));
    m_currentWeatherIntensity = 1.0f;
    m_currentWind = m_currentWeatherType->maximumWind * (m_root->random().randb() ? 1 : -1);
  }

  m_lastWeatherChangeTime = m_currentTime;
  if (m_forceWeather)
    m_nextWeatherChangeTime = std::numeric_limits<float>::infinity();
  else
    m_nextWeatherChangeTime =
      m_currentWeatherType ? m_currentTime + m_root->random().randd(m_currentWeatherType->duration[0], m_currentWeatherType->duration[1])
                           : m_currentTime;

  setNetStates();
  }

void ServerWeather::forceWeather(bool force) {
  m_forceWeather = force;
  if (force)
    m_nextWeatherChangeTime = std::numeric_limits<float>::infinity();
}

void ServerWeather::setNetStates() {
  m_weatherPoolNetState.set(DataStreamBuffer::serializeContainer(m_weatherPool.items()));
  m_undergroundLevelNetState.set(m_undergroundLevel);
  m_currentWeatherIndexNetState.set(m_currentWeatherIndex);
  m_currentWeatherIntensityNetState.set(m_currentWeatherIntensity);
  m_currentWindNetState.set(m_currentWind);
}

void ServerWeather::spawnWeatherProjectiles(float dt) {
  if (!m_currentWeatherType || m_clientVisibleRegions.empty())
    return;

  auto projectileDatabase = m_root->projectileDatabase();

  // TODO: The complexity of this method is TERRIBLE, if this becomes a problem
  // for any reason there are large numbers of ways to make this much better,
  // but this was the lazy, simple-ish, and clear (hah) way.

  for (auto const& projectileConfig : m_currentWeatherType->projectiles) {
    // Gather all the tops of the client regions together with the proper
    // padding, splitting at the world wrap boundary.
    List<std::pair<Vec2I, int>> baseSpawnRegions;
    for (auto const& clientRegion : m_clientVisibleRegions) {
      Vec2I baseRegion = {clientRegion.xMin() - projectileConfig.spawnHorizontalPad, clientRegion.xMax() + projectileConfig.spawnHorizontalPad};
      int height = clientRegion.yMax();
      for (auto const& region : m_worldGeometry.splitXRegion(baseRegion))
        baseSpawnRegions.append({region, height});
    }

    // We are going to have to eliminate vertically redundant sections of
    // spawning regions, so gather up every left and right edge of a spawn
    // region is a "split point"
    List<int> splitPoints;
    for (auto const& baseSpawnRegion : baseSpawnRegions) {
      splitPoints.append(baseSpawnRegion.first[0]);
      splitPoints.append(baseSpawnRegion.first[1]);
    }

    // Split every spawn region on every split point.
    List<std::pair<Vec2I, int>> splitSpawnRegions;
    for (auto const& baseSpawnRegion : baseSpawnRegions) {
      List<Vec2I> regions = {baseSpawnRegion.first};
      for (auto splitPoint : splitPoints) {
        auto prevRegions = take(regions);
        for (auto const& region : prevRegions) {
          if (splitPoint > region[0] && splitPoint < region[1]) {
            regions.append({region[0], splitPoint});
            regions.append({splitPoint, region[1]});
          } else {
            regions.append(region);
          }
        }
      }
      for (auto const& region : regions)
        splitSpawnRegions.append({region, baseSpawnRegion.second});
    }

    // Sort the split spawn regions by leftmost point then height, preparing to
    // remove the lower overlapping sections.
    std::sort(splitSpawnRegions.begin(), splitSpawnRegions.end(),
        [](std::pair<Vec2I, int> const& lhs, std::pair<Vec2I, int> rhs) {
          return std::tie(lhs.first[0], lhs.second) < std::tie(rhs.first[0], rhs.second);
        });

    // For each region, at this point, if the region to the right shares the
    // same starting X, because we've split up each region on each possible
    // overlapping point, then they totally overlap.  The lower region (which
    // should come before in the list) is totally redundant and should be
    // removed.
    for (auto sit = splitSpawnRegions.begin(); sit != splitSpawnRegions.end();) {
      auto right = std::next(sit);
      if (right != splitSpawnRegions.end() && sit->first[0] == right->first[0])
        sit = splitSpawnRegions.erase(sit);
      else
        ++sit;
    }

    for (auto const& spawnRegion : splitSpawnRegions) {
      RectF spawnRect = RectF(spawnRegion.first[0],
          spawnRegion.second,
          spawnRegion.first[1],
          spawnRegion.second + projectileConfig.spawnAboveRegion);

      // Figure out a good target value based on the rate per x tile, making
      // sure to handle very low count values appropriately on average.
      float count = projectileConfig.ratePerX * spawnRect.width() * dt * m_currentWeatherIntensity;
      if (m_root->random().randf() > fpart(count))
        count = std::floor(count);
      else
        count = std::ceil(count);

      for (int i = 0; i < count; ++i) {
        Vec2F position = {m_root->random().randf() * spawnRect.width() + spawnRect.xMin(), m_root->random().randf() * spawnRect.height() + spawnRect.yMin()};

        if (position[1] > m_undergroundLevel && (!m_weatherEffectsActiveQuery || m_weatherEffectsActiveQuery(Vec2I::floor(position)))) {
          // Make sure not to spawn projectiles if they intersect any client
          // visible region.
          bool intersectsVisibleRegion = false;
          for (auto const& visibleRegion : m_clientVisibleRegions) {
            if (RectF(visibleRegion).contains(position)) {
              intersectsVisibleRegion = true;
              break;
            }
          }

          if (!intersectsVisibleRegion) {
            auto newProjectile = projectileDatabase->createProjectile(projectileConfig.projectile, projectileConfig.parameters);
            newProjectile->setInitialPosition(position);
            newProjectile->setInitialVelocity(projectileConfig.velocity + Vec2F(projectileConfig.windAffectAmount * wind(), 0));
            newProjectile->setTeam(EntityDamageTeam(TeamType::Environment));
            m_newProjectiles.append(newProjectile);
          }
        }
      }
    }
  }
}

ClientWeather::ClientWeather(Root& root) : m_root(&root) {
  m_undergroundLevel = 0.0f;
  m_currentWeatherIndex = NPos;
  m_currentWeatherIntensity = 0.0f;
  m_currentWind = 0.0f;
  m_currentTime = 0.0;

  m_netGroup.addNetElement(&m_weatherPoolNetState);
  m_netGroup.addNetElement(&m_undergroundLevelNetState);
  m_netGroup.addNetElement(&m_currentWeatherIndexNetState);
  m_netGroup.addNetElement(&m_currentWeatherIntensityNetState);
  m_netGroup.addNetElement(&m_currentWindNetState);
}

void ClientWeather::setup(WorldGeometry worldGeometry, WeatherEffectsActiveQuery weatherEffectsActiveQuery) {
  m_worldGeometry = worldGeometry;
  m_weatherEffectsActiveQuery = weatherEffectsActiveQuery;
  m_currentTime = 0.0;
}

void ClientWeather::readUpdate(ByteArray data, NetCompatibilityRules rules) {
  if (!data.empty()) {
    m_netGroup.readNetState(data, 0.0f, rules);
    getNetStates();
  }
}

void ClientWeather::clear() {
  m_currentWeatherIndex = NPos;
  m_currentWeatherType.reset();
  m_currentWeatherIntensity = 0.0f;
  m_currentWind = 0.0f;
  m_visibleRegion = {};
  m_particles.clear();
  m_lastParticleVisibleRegion = {};
}

void ClientWeather::setVisibleRegion(RectI visibleRegion) {
  m_visibleRegion = visibleRegion;
}

void ClientWeather::update(double dt) {
  m_currentTime += dt;

  if (m_currentWeatherIndex == NPos) {
    m_currentWeatherType = {};
  } else {
    if (m_visibleRegion == RectI() || m_visibleRegion.yMax() > m_undergroundLevel)
      m_currentWeatherType = m_root->biomeDatabase()->weatherType(m_weatherPool.item(m_currentWeatherIndex));
    else
      m_currentWeatherType = {};
  }

  if (m_currentWeatherType && m_visibleRegion != RectI())
    spawnWeatherParticles(RectF(m_visibleRegion), dt);
}

float ClientWeather::wind() const {
  return m_currentWind * m_currentWeatherIntensity;
}

float ClientWeather::weatherIntensity() const {
  return m_currentWeatherIntensity;
}

StringList ClientWeather::statusEffects() const {
  if (m_currentWeatherIntensity == 1.0 && m_currentWeatherType)
    return m_currentWeatherType->statusEffects;
  return {};
}

List<Particle> ClientWeather::pullNewParticles() {
  return take(m_particles);
}

std::optional<String> ClientWeather::activeWeather() const {
  if (m_currentWeatherIndex == NPos)
    return {};
  else
    return m_weatherPool.item(m_currentWeatherIndex);
}

StringList ClientWeather::weatherTrackOptions() const {
  if (m_currentWeatherType)
    return m_currentWeatherType->weatherNoises;
  return {};
}

std::optional<String> ClientWeather::weatherParallax() const {
  if (m_currentWeatherType)
    return m_currentWeatherType->parallax;
  return {};
}

void ClientWeather::getNetStates() {
  if (m_weatherPoolNetState.pullUpdated())
    m_weatherPool = WeatherPool(DataStreamBuffer::deserializeContainer<WeatherPool::ItemsList>(m_weatherPoolNetState.get()));
  m_undergroundLevel = m_undergroundLevelNetState.get();
  m_currentWeatherIndex = m_currentWeatherIndexNetState.get();
  m_currentWeatherIntensity = m_currentWeatherIntensityNetState.get();
  m_currentWind = m_currentWindNetState.get();
}

void ClientWeather::spawnWeatherParticles(RectF newClientRegion, float dt) {
  if (!m_currentWeatherType)
    return;

  for (auto const& particleConfig : m_currentWeatherType->particles) {
    // Move client region to same wrap region as newClientRegion
    RectF visibleRegion(m_worldGeometry.nearestTo(newClientRegion.min(), m_lastParticleVisibleRegion.min()),
        m_worldGeometry.nearestTo(newClientRegion.max(), m_lastParticleVisibleRegion.max()));

    Vec2F targetVelocity = particleConfig.particle.velocity + Vec2F(wind(), 0);
    float angleChange = Vec2F::angleBetween2(Vec2F(0, 1), targetVelocity);
    visibleRegion.translate(targetVelocity * dt);

    for (auto const& renderZone : newClientRegion.subtract(visibleRegion)) {
      float count = particleConfig.density * renderZone.width() * renderZone.height() * m_currentWeatherIntensity;
      if (m_root->random().randf() > fpart(count))
        count = std::floor(count);
      else
        count = std::ceil(count);

      for (int i = 0; i < count; ++i) {
        auto newParticle = particleConfig.particle;
        float x = m_root->random().randf() * renderZone.width() + renderZone.xMin();
        float y = m_root->random().randf() * renderZone.height() + renderZone.yMin();
        newParticle.position += m_worldGeometry.xwrap(Vec2F(x, y));
        newParticle.velocity = targetVelocity;
        if (y > m_undergroundLevel && (!m_weatherEffectsActiveQuery || m_weatherEffectsActiveQuery(Vec2I::floor(newParticle.position)))) {
          if (particleConfig.autoRotate)
            newParticle.rotation += angleChange;
          m_particles.append(std::move(newParticle));
        }
      }
    }
  }

  m_lastParticleVisibleRegion = newClientRegion;
}

}
