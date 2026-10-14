module;
import std;
import star.xx_hash;
import star.json;
import star.id_map;
import star.game_types;
import star.image;
import star.list;
import star.map;
import star.bi_map;
import star.vector;
import star.version;
import star.color;
import star.string;
import star.data_stream;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.either;
import star.maybe;
import star.random;
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import star.font;
import star.text;
import star.listener;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.variant;


namespace Star {
class Assets;
using AssetsPtr = std::shared_ptr<Assets>;
using AssetsConstPtr = std::shared_ptr<Assets const>;
using AssetsWeakPtr = std::weak_ptr<Assets>;
using AssetsConstWeakPtr = std::weak_ptr<Assets const>;
using AssetsUPtr = std::unique_ptr<Assets>;
using AssetsConstUPtr = std::unique_ptr<Assets const>;
class EnvironmentPainter;
using EnvironmentPainterPtr = std::shared_ptr<EnvironmentPainter>;
using EnvironmentPainterConstPtr = std::shared_ptr<EnvironmentPainter const>;
using EnvironmentPainterWeakPtr = std::weak_ptr<EnvironmentPainter>;
using EnvironmentPainterConstWeakPtr = std::weak_ptr<EnvironmentPainter const>;
using EnvironmentPainterUPtr = std::unique_ptr<EnvironmentPainter>;
using EnvironmentPainterConstUPtr = std::unique_ptr<EnvironmentPainter const>;
class TilePainter;
using TilePainterPtr = std::shared_ptr<TilePainter>;
using TilePainterConstPtr = std::shared_ptr<TilePainter const>;
using TilePainterWeakPtr = std::weak_ptr<TilePainter>;
using TilePainterConstWeakPtr = std::weak_ptr<TilePainter const>;
using TilePainterUPtr = std::unique_ptr<TilePainter>;
using TilePainterConstUPtr = std::unique_ptr<TilePainter const>;
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
import star.ref_ptr;
import star.renderer;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.asset_texture_group;
import star.drawable_painter;

import star.world_camera;


namespace Star {

class WorldPainter;
using WorldPainterPtr = std::shared_ptr<WorldPainter>;
using WorldPainterConstPtr = std::shared_ptr<WorldPainter const>;
using WorldPainterWeakPtr = std::weak_ptr<WorldPainter>;
using WorldPainterConstWeakPtr = std::weak_ptr<WorldPainter const>;
using WorldPainterUPtr = std::unique_ptr<WorldPainter>;
using WorldPainterConstUPtr = std::unique_ptr<WorldPainter const>;

// Will update client rendering window internally
class WorldPainter {
public:
  WorldPainter();

  void renderInit(RendererPtr renderer);

  void setCameraPosition(WorldGeometry const& worldGeometry, Vec2F const& position);

  WorldCamera& camera();

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

  RendererPtr m_renderer;

  TextPainterPtr m_textPainter;
  DrawablePainterPtr m_drawablePainter;
  EnvironmentPainterPtr m_environmentPainter;
  TilePainterPtr m_tilePainter;

  Json m_highlightConfig;
  Map<EntityHighlightEffectType, std::pair<Directives, Directives>> m_highlightDirectives;

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
