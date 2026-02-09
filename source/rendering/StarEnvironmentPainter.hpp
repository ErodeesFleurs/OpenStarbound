#pragma once

#include "StarAssetTextureGroup.hpp"
#include "StarConfig.hpp"
#include "StarParallax.hpp"
#include "StarPerlin.hpp"
#include "StarRandomPoint.hpp"
#include "StarRenderer.hpp"
#include "StarSkyRenderData.hpp"
#include "StarWorldCamera.hpp"

import std;

namespace Star {

class EnvironmentPainter {
public:
  EnvironmentPainter(Ptr<Renderer> renderer);

  void update(float dt);

  void renderStars(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderDebrisFields(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderBackOrbiters(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderPlanetHorizon(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderFrontOrbiters(float pixelRatio, Vec2F const& screenSize, SkyRenderData const& sky);
  void renderSky(Vec2F const& screenSize, SkyRenderData const& sky);

  void renderParallaxLayers(Vec2F parallaxWorldPosition, WorldCamera const& camera, ParallaxLayers const& layers, SkyRenderData const& sky);

  void cleanup(std::int64_t textureTimeout);

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

  [[nodiscard]] auto starsHash(SkyRenderData const& sky, Vec2F const& viewSize) const -> std::uint64_t;
  void setupStars(SkyRenderData const& sky);

  Ptr<Renderer> m_renderer;
  Ptr<AssetTextureGroup> m_textureGroup;

  double m_timer;
  PerlinF m_rayPerlin;

  std::uint64_t m_starsHash{};
  List<RefPtr<Texture>> m_starTextures;
  std::shared_ptr<Random2dPointGenerator<std::pair<std::size_t, float>>> m_starGenerator;
  List<std::shared_ptr<Random2dPointGenerator<std::pair<String, float>, double>>> m_debrisGenerators;
};

}// namespace Star
