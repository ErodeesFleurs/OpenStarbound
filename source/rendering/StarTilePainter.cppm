module;
import std;
import star.xx_hash;
import star.json;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.image;
import star.list;
import star.map;
import star.bi_map;
import star.game_types;
import star.material_types;
import star.vector;
import star.version;
import star.color;
import star.string;
import star.data_stream;
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
import star.ref_ptr;
import star.renderer;

import star.tile_drawer;

namespace Star {

class WorldCamera;

class Assets;
using AssetsPtr = std::shared_ptr<Assets>;
using AssetsConstPtr = std::shared_ptr<Assets const>;
using AssetsWeakPtr = std::weak_ptr<Assets>;
using AssetsConstWeakPtr = std::weak_ptr<Assets const>;
using AssetsUPtr = std::unique_ptr<Assets>;
using AssetsConstUPtr = std::unique_ptr<Assets const>;
class MaterialDatabase;
using MaterialDatabasePtr = std::shared_ptr<MaterialDatabase>;
using MaterialDatabaseConstPtr = std::shared_ptr<MaterialDatabase const>;
using MaterialDatabaseWeakPtr = std::weak_ptr<MaterialDatabase>;
using MaterialDatabaseConstWeakPtr = std::weak_ptr<MaterialDatabase const>;
using MaterialDatabaseUPtr = std::unique_ptr<MaterialDatabase>;
using MaterialDatabaseConstUPtr = std::unique_ptr<MaterialDatabase const>;
class TilePainter;
using TilePainterPtr = std::shared_ptr<TilePainter>;
using TilePainterConstPtr = std::shared_ptr<TilePainter const>;
using TilePainterWeakPtr = std::weak_ptr<TilePainter>;
using TilePainterConstWeakPtr = std::weak_ptr<TilePainter const>;
using TilePainterUPtr = std::unique_ptr<TilePainter>;
using TilePainterConstUPtr = std::unique_ptr<TilePainter const>;

class TilePainter : public TileDrawer {
public:
  // The rendered tiles are split and cached in chunks of RenderChunkSize x
  // RenderChunkSize.  This means that, around the border, there may be as many
  // as RenderChunkSize - 1 tiles rendered outside of the viewing area from
  // chunk alignment.  In addition to this, there is also a region around each
  // tile that is used for neighbor based rendering rules which has a max of
  // MaterialRenderProfileMaxNeighborDistance.  If the given tile data does not
  // extend RenderChunkSize + MaterialRenderProfileMaxNeighborDistance - 1
  // around the viewing area, then border chunks can continuously change hash,
  // and will be recomputed too often.
  static unsigned const RenderChunkSize = 16;
  static unsigned const BorderTileSize = RenderChunkSize + MaterialRenderProfileMaxNeighborDistance - 1;

  TilePainter(RendererPtr renderer);

  // Adjusts lighting levels for liquids.
  void adjustLighting(WorldRenderData& renderData) const;

  // Sets up chunk data for every chunk that intersects the rendering region
  // and prepares it for rendering.  Do not call cleanup in between calling
  // setup and each render method.
  void setup(WorldCamera const& camera, WorldRenderData& renderData);

  void renderBackground(WorldCamera const& camera);
  void renderMidground(WorldCamera const& camera);
  void renderLiquid(WorldCamera const& camera);
  void renderForeground(WorldCamera const& camera);

  // Clears any render data, as well as cleaning up old cached textures and
  // chunks.
  void cleanup();

private:
  typedef std::uint64_t QuadZLevel;
  typedef std::uint64_t ChunkHash;

  enum class TerrainLayer { Background, Midground, Foreground };

  struct LiquidInfo {
    TexturePtr texture;
    Vec4B color;
    Vec3F bottomLightMix;
    float textureMovementFactor;
  };

  typedef HashMap<TerrainLayer, HashMap<QuadZLevel, RenderBufferPtr>> TerrainChunk;
  typedef HashMap<LiquidId, RenderBufferPtr> LiquidChunk;

  typedef std::tuple<MaterialId, MaterialRenderPieceIndex, MaterialHue, bool> MaterialPieceTextureKey;
  typedef String AssetTextureKey;
  typedef Variant<MaterialPieceTextureKey, AssetTextureKey> TextureKey;

  struct TextureKeyHash {
    std::size_t operator()(TextureKey const& key) const;
  };

  // chunkIndex here is the index of the render chunk such that chunkIndex *
  // RenderChunkSize results in the coordinate of the lower left most tile in
  // the render chunk.

  static ChunkHash terrainChunkHash(WorldRenderData& renderData, Vec2I chunkIndex);
  static ChunkHash liquidChunkHash(WorldRenderData& renderData, Vec2I chunkIndex);

  void renderTerrainChunks(WorldCamera const& camera, TerrainLayer terrainLayer);

  std::shared_ptr<TerrainChunk const> getTerrainChunk(WorldRenderData& renderData, Vec2I chunkIndex);
  std::shared_ptr<LiquidChunk const> getLiquidChunk(WorldRenderData& renderData, Vec2I chunkIndex);

  bool produceTerrainPrimitives(HashMap<QuadZLevel, List<RenderPrimitive>>& primitives,
      TerrainLayer terrainLayer, Vec2I const& pos, WorldRenderData const& renderData);
  void produceLiquidPrimitives(HashMap<LiquidId, List<RenderPrimitive>>& primitives, Vec2I const& pos, WorldRenderData const& renderData);

  float liquidDrawLevel(float liquidLevel) const;

  List<LiquidInfo> m_liquids;

  RendererPtr m_renderer;
  TextureGroupPtr m_textureGroup;

  HashTtlCache<TextureKey, TexturePtr, TextureKeyHash> m_textureCache;
  HashTtlCache<std::pair<Vec2I, ChunkHash>, std::shared_ptr<TerrainChunk const>> m_terrainChunkCache;
  HashTtlCache<std::pair<Vec2I, ChunkHash>, std::shared_ptr<LiquidChunk const>> m_liquidChunkCache;

  List<std::shared_ptr<TerrainChunk const>> m_pendingTerrainChunks;
  List<std::shared_ptr<LiquidChunk const>> m_pendingLiquidChunks;

  Maybe<Vec2F> m_lastCameraCenter;
  Vec2F m_cameraPan;
};

}

export module star.tile_painter;

export namespace Star {
  using ::Star::WorldCamera;
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
  using ::Star::TilePainterPtr;
  using ::Star::TilePainterConstPtr;
  using ::Star::TilePainterWeakPtr;
  using ::Star::TilePainterConstWeakPtr;
  using ::Star::TilePainterUPtr;
  using ::Star::TilePainterConstUPtr;
}
