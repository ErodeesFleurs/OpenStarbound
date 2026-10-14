module;
import star.json;
import star.xx_hash;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.image;
import star.list;
import star.bi_map;
import star.game_types;
import star.material_types;
import star.vector;
import star.map;
import star.version;
import star.color;
import star.string;
import star.data_stream;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.either;
import star.maybe;
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import std;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.variant;


import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.material_render_profile;

import star.array;
namespace Star {

class Assets;
using AssetsPtr = std::shared_ptr<Assets>;
using AssetsConstPtr = std::shared_ptr<const Assets>;
using AssetsWeakPtr = std::weak_ptr<Assets>;
using AssetsConstWeakPtr = std::weak_ptr<const Assets>;
using AssetsUPtr = std::unique_ptr<Assets>;
using AssetsConstUPtr = std::unique_ptr<const Assets>;
class MaterialDatabase;
using MaterialDatabasePtr = std::shared_ptr<MaterialDatabase>;
using MaterialDatabaseConstPtr = std::shared_ptr<const MaterialDatabase>;
using MaterialDatabaseWeakPtr = std::weak_ptr<MaterialDatabase>;
using MaterialDatabaseConstWeakPtr = std::weak_ptr<const MaterialDatabase>;
using MaterialDatabaseUPtr = std::unique_ptr<MaterialDatabase>;
using MaterialDatabaseConstUPtr = std::unique_ptr<const MaterialDatabase>;
class TileDrawer;
using TileDrawerPtr = std::shared_ptr<TileDrawer>;
using TileDrawerConstPtr = std::shared_ptr<const TileDrawer>;
using TileDrawerWeakPtr = std::weak_ptr<TileDrawer>;
using TileDrawerConstWeakPtr = std::weak_ptr<const TileDrawer>;
using TileDrawerUPtr = std::unique_ptr<TileDrawer>;
using TileDrawerConstUPtr = std::unique_ptr<const TileDrawer>;
class TilePainter;

class TileDrawer {
public:
  typedef std::uint64_t QuadZLevel;
  typedef HashMap<QuadZLevel, List<Drawable>> Drawables;

  typedef std::size_t MaterialRenderPieceIndex;
  typedef List<std::pair<MaterialRenderPieceConstPtr, Vec2F>> MaterialPieceResultList;

  enum class TerrainLayer { Background, Midground, Foreground };

  static RenderTile DefaultRenderTile;

  static TileDrawer* singletonPtr();
  static TileDrawer& singleton();

  TileDrawer();
  ~TileDrawer();

  bool produceTerrainDrawables(Drawables& drawables, TerrainLayer terrainLayer, Vec2I const& pos,
    WorldRenderData const& renderData, float scale = 1.0f, Vec2I variantOffset = {}, Maybe<TerrainLayer> variantLayer = {});

  WorldRenderData& renderData();
  MutexLocker lockRenderData();

  template <typename Function>
  static void forEachRenderTile(WorldRenderData const& renderData, RectI const& worldCoordRange, Function&& function);
private:
  friend class TilePainter;

  static TileDrawer* s_singleton;

  static RenderTile const& getRenderTile(WorldRenderData const& renderData, Vec2I const& worldPos);

  static QuadZLevel materialZLevel(std::uint32_t zLevel, MaterialId material, MaterialHue hue, MaterialColorVariant colorVariant);
  static QuadZLevel modZLevel(std::uint32_t zLevel, ModId mod, MaterialHue hue, MaterialColorVariant colorVariant);
  static QuadZLevel damageZLevel();

  static bool determineMatchingPieces(MaterialPieceResultList& resultList, bool* occlude, MaterialDatabaseConstPtr const& materialDb, MaterialRenderMatchList const& matchList,
    WorldRenderData const& renderData, Vec2I const& basePos, TileLayer layer, bool isMod);

  Vec4B m_backgroundLayerColor;
  Vec4B m_foregroundLayerColor;
  Vec2F m_liquidDrawLevels;

  WorldRenderData m_tempRenderData;
  Mutex m_tempRenderDataMutex;
};

template <typename Function>
void TileDrawer::forEachRenderTile(WorldRenderData const& renderData, RectI const& worldCoordRange, Function&& function) {
  RectI indexRect = RectI::withSize(Vec2I(renderData.geometry.pdiff(worldCoordRange.min()[0], renderData.tileMinPosition[0]), worldCoordRange.min()[1] - renderData.tileMinPosition[1]), worldCoordRange.size());
  indexRect.limit(RectI::withSize(Vec2I(0, 0), Vec2I(renderData.tiles.size())));

  if (!indexRect.isEmpty()) {
    renderData.tiles.forEach(Array2S(indexRect.min()), Array2S(indexRect.size()), [&](Array2S const& index, RenderTile const& tile) {
      return function(worldCoordRange.min() + (Vec2I(index) - indexRect.min()), tile);
      });
  }
}

}

export module star.tile_drawer;

export namespace Star {
  using ::Star::Assets;
  using ::Star::AssetsPtr;
  using ::Star::AssetsConstPtr;
  using ::Star::AssetsWeakPtr;
  using ::Star::AssetsConstWeakPtr;
  using ::Star::AssetsUPtr;
  using ::Star::AssetsConstUPtr;
  using ::Star::MaterialDatabase;
  using ::Star::MaterialDatabasePtr;
  using ::Star::MaterialDatabaseConstPtr;
  using ::Star::MaterialDatabaseWeakPtr;
  using ::Star::MaterialDatabaseConstWeakPtr;
  using ::Star::MaterialDatabaseUPtr;
  using ::Star::MaterialDatabaseConstUPtr;
  using ::Star::TilePainter;
  using ::Star::TileDrawer;
  using ::Star::TileDrawerPtr;
  using ::Star::TileDrawerConstPtr;
  using ::Star::TileDrawerWeakPtr;
  using ::Star::TileDrawerConstWeakPtr;
  using ::Star::TileDrawerUPtr;
  using ::Star::TileDrawerConstUPtr;
}
