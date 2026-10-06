#pragma once

#include "StarPoly.hpp"
import star.world_geometry;
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"
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
#include "StarEither.hpp"
#include "StarRect.hpp"
#include "StarImage.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarInterpolation.hpp"
#include "StarList.hpp"
#include "StarVector.hpp"
import star.cellular_light_array;
#include "StarThread.hpp"
import star.cellular_lighting;
import star.world_render_data;
#include "StarTilePainter.hpp"
#include "StarEnvironmentPainter.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
import star.font_texture_group;
#include "StarBiMap.hpp"
import star.anchor_types;
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
import star.text_painter;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
#include "StarRenderer.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarListener.hpp"
import star.asset_texture_group;
import star.drawable_painter;
#include "StarRenderer.hpp"

import star.world_camera;

namespace Star {

STAR_CLASS(WorldPainter);

// Will update client rendering window internally
class WorldPainter {
public:
  WorldPainter();

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

  // Updated every frame

  AssetsConstPtr m_assets;
  RectF m_worldScreenRect;

  Vec2F m_previousCameraCenter;
  Vec2F m_parallaxWorldPosition;

  float m_preloadTextureChance;
};

}
