#pragma once

#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarDirectives.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"
import star.tile_damage;
import star.plant_database;
import star.parallax;
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
import star.collision_block;
#include "StarMultiArray.hpp"
#include <functional>
#include "StarGameTypes.hpp"
#include "StarXXHash.hpp"
#include "StarMathCommon.hpp"
import star.liquid_types;
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
import star.tile_damage;
#include "StarTileSectorArray.hpp"
#include "StarWorldLayout.hpp"
#include "StarVersion.hpp"
import star.collision_generator;
import star.world_tiles;
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
#include "StarGameTypes.hpp"
import star.entity_rendering_types;
#include "StarBiMap.hpp"
import star.sky_types;
#include "StarEither.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
import star.sky_parameters;
import star.sky_render_data;
#include "StarMaybe.hpp"
#include "StarDirectives.hpp"
#include "StarNetElementSystem.hpp"
import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.animation;
import star.particle;
#include "StarWeightedPool.hpp"
import star.weather_types;
#include "StarEntity.hpp"
#include "StarThread.hpp"
#include "StarCellularLighting.hpp"
import star.world_render_data;
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarBiMap.hpp"
#include "StarListener.hpp"
#include "StarRenderer.hpp"
#include "StarAssetPath.hpp"
import star.asset_texture_group;
#include "StarRenderer.hpp"
#include "StarPerlin.hpp"
#include "StarRandomPoint.hpp"

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
