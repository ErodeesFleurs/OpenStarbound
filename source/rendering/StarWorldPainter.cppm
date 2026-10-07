module;
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"
#include "StarImage.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarEither.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarThread.hpp"
#include "StarRect.hpp"
#include "StarFont.hpp"
#include "StarStringView.hpp"
import star.text;
import star.listener;
#include <functional>
#include "StarSet.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
import star.perlin;
#include "StarVariant.hpp"


namespace Star {
STAR_CLASS(Assets);
STAR_CLASS(EnvironmentPainter);
STAR_CLASS(TilePainter);
}
import star.world_geometry;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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
import star.plant_database;
import star.parallax;
import star.animation;
import star.particle;
import star.weather_types;

import star.damage_types;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
#include "StarRefPtr.hpp"
import star.renderer;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.asset_texture_group;
import star.drawable_painter;

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

export module star.world_painter;

export namespace Star {
  using ::Star::Assets;
  using ::Star::WorldPainter;
  using ::Star::WorldPainterPtr;
  using ::Star::WorldPainterConstPtr;
  using ::Star::WorldPainterWeakPtr;
  using ::Star::WorldPainterConstWeakPtr;
  using ::Star::WorldPainterUPtr;
  using ::Star::WorldPainterConstUPtr;
}
