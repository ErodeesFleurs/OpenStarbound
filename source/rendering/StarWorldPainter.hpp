#pragma once

#include "StarAssets.hpp"
#include "StarConfig.hpp"
#include "StarDrawablePainter.hpp"
#include "StarEnvironmentPainter.hpp"
#include "StarRenderer.hpp"
#include "StarTextPainter.hpp"
#include "StarTilePainter.hpp"
#include "StarWorldRenderData.hpp"

import std;

namespace Star {

// Will update client rendering window internally
class WorldPainter {
public:
  WorldPainter();

  void renderInit(Ptr<Renderer> renderer);

  void setCameraPosition(WorldGeometry const& worldGeometry, Vec2F const& position);

  auto camera() -> WorldCamera&;

  void update(float dt);
  void render(WorldRenderData& renderData, std::function<bool()> lightWaiter);
  void adjustLighting(WorldRenderData& renderData);

private:
  void renderParticles(WorldRenderData& renderData, Particle::Layer layer);
  void renderBars(WorldRenderData& renderData);

  void drawEntityLayer(List<Drawable> drawables, EntityHighlightEffect highlightEffect = EntityHighlightEffect());

  void drawDrawable(Drawable drawable);
  void drawDrawableSet(List<Drawable>& drawable);

  WorldCamera m_camera;

  Ptr<Renderer> m_renderer;

  Ptr<TextPainter> m_textPainter;
  Ptr<DrawablePainter> m_drawablePainter;
  Ptr<EnvironmentPainter> m_environmentPainter;
  Ptr<TilePainter> m_tilePainter;

  Json m_highlightConfig;
  Map<EntityHighlightEffectType, std::pair<Directives, Directives>> m_highlightDirectives;

  Vec2F m_entityBarOffset;
  Vec2F m_entityBarSpacing;
  Vec2F m_entityBarSize;
  Vec2F m_entityBarIconOffset;

  // Updated every frame

  ConstPtr<Assets> m_assets;
  RectF m_worldScreenRect;

  Vec2F m_previousCameraCenter;
  Vec2F m_parallaxWorldPosition;

  float m_preloadTextureChance;
};

}// namespace Star
