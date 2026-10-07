module;
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVersion.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarAssetPath.hpp"
#include "StarEither.hpp"
#include "StarWeightedPool.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarThread.hpp"
#include "StarRect.hpp"
#include "StarInterpolation.hpp"
#include "StarListener.hpp"
#include "StarPerlin.hpp"
#include "StarRandomPoint.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarVariant.hpp"


import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.collision_block;
import star.liquid_types;
import star.worker_pool;
import star.tile_sector_array;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.drawable;
import star.entity_rendering_types;
import star.sky_types;
import star.celestial_coordinate;
import star.sky_parameters;
import star.sky_render_data;
import star.animation;
import star.particle;
import star.weather_types;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
#include "StarRenderer.hpp"
import star.asset_texture_group;

namespace Star {

class WorldCamera;

STAR_CLASS(EnvironmentPainter);

class EnvironmentPainter {
public:
  EnvironmentPainter(RendererPtr renderer);

  void update(float dt);

  void renderStars(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderDebrisFields(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderBackOrbiters(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderPlanetHorizon(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderFrontOrbiters(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderSky(Vec2F const& screenSize, SkyRenderData const& sky);

  void renderParallaxLayers(Vec2F parallaxWorldPosition, WorldCamera const& camera, ParallaxLayers const& layers, SkyRenderData const& sky);

  void cleanup(int64_t textureTimeout);

private:
  static float const SunriseTime;
  static float const SunsetTime;
  static float const SunFadeRate;
  static float const MaxFade;
  static float const RayPerlinFrequency;
  static float const RayPerlinAmplitude;
  static int const RayCount;
  static float const RayMinWidth;
  static float const RayWidthVariance;
  static float const RayAngleVariance;
  static float const SunRadius;
  static float const RayColorDependenceLevel;
  static float const RayColorDependenceScale;
  static float const RayUnscaledAlphaVariance;
  static float const RayMinUnscaledAlpha;
  static Vec3B const RayColor;

  void drawRays(float pixelRatio, SkyRenderData const& sky, Vec2F start, float length, double time, float alpha);
  void drawRay(float pixelRatio,
      SkyRenderData const& sky,
      Vec2F start,
      float width,
      float length,
      float angle,
      double time,
      Vec3B color,
      float alpha);
  void drawOrbiter(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky, SkyOrbiter const& orbiter);

  uint64_t starsHash(SkyRenderData const& sky, Vec2F const& viewSize) const;
  void setupStars(SkyRenderData const& sky);

  RendererPtr m_renderer;
  AssetTextureGroupPtr m_textureGroup;

  double m_timer;
  PerlinF m_rayPerlin;

  uint64_t m_starsHash{};
  List<TexturePtr> m_starTextures;
  shared_ptr<Random2dPointGenerator<pair<size_t, float>>> m_starGenerator;
  List<shared_ptr<Random2dPointGenerator<pair<String, float>, double>>> m_debrisGenerators;
};

}

export module star.environment_painter;

export namespace Star {
  using ::Star::WorldCamera;
  using ::Star::EnvironmentPainter;
  using ::Star::EnvironmentPainterPtr;
  using ::Star::EnvironmentPainterConstPtr;
  using ::Star::EnvironmentPainterWeakPtr;
  using ::Star::EnvironmentPainterConstWeakPtr;
  using ::Star::EnvironmentPainterUPtr;
  using ::Star::EnvironmentPainterConstUPtr;
}
