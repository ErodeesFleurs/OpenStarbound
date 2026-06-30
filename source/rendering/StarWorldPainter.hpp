#pragma once

#include "StarWorldRenderData.hpp"
#include "StarTilePainter.hpp"
#include "StarEnvironmentPainter.hpp"
#include "StarTextPainter.hpp"
#include "StarDrawablePainter.hpp"
#include "StarConfiguration.hpp"
#include "StarRenderer.hpp"

namespace Star {

class MaterialDatabase;
using MaterialDatabaseConstPtr = SharedPtr<MaterialDatabase const>;
class LiquidsDatabase;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;
class WorldPainter;
using WorldPainterPtr = SharedPtr<WorldPainter>;

// Will update client rendering window internally
class WorldPainter {
public:
  WorldPainter(AssetsConstPtr assets, ConfigurationPtr configuration, function<void(ListenerWeakPtr)> registerReloadListener, MaterialDatabaseConstPtr materialDatabase, LiquidsDatabaseConstPtr liquidsDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  void renderInit(RendererPtr renderer);

  void setCameraPosition(WorldGeometry const& worldGeometry, Vec2F const& position);

  WorldCamera& camera();

  void update(float dt);
  void render(WorldRenderData& renderData, function<bool()> lightWaiter);
  void adjustLighting(WorldRenderData& renderData);

private:
  void renderParticles(WorldRenderData& renderData, Particle::Layer layer);
  void renderBars(WorldRenderData& renderData);

  void drawEntityLayer(List<Drawable> drawables, EntityHighlightEffect highlightEffect = EntityHighlightEffect());

  void drawDrawable(Drawable drawable);
  void drawDrawableSet(List<Drawable>& drawable);

  WorldCamera m_camera;

  RendererPtr m_renderer;

  TextPainterPtr m_textPainter;
  DrawablePainterPtr m_drawablePainter;
  EnvironmentPainterPtr m_environmentPainter;
  TilePainterPtr m_tilePainter;

  Json m_highlightConfig;
  Map<EntityHighlightEffectType, pair<Directives, Directives>> m_highlightDirectives;

  Vec2F m_entityBarOffset;
  Vec2F m_entityBarSpacing;
  Vec2F m_entityBarSize;
  Vec2F m_entityBarIconOffset;
  float m_lightMapMultiplier;

  // Updated every frame

  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  function<void(ListenerWeakPtr)> m_registerReloadListener;
  MaterialDatabaseConstPtr m_materialDatabase;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  RectF m_worldScreenRect;

  Vec2F m_previousCameraCenter;
  Vec2F m_parallaxWorldPosition;

  float m_preloadTextureChance;
};

}
