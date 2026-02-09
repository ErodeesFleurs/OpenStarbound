#pragma once

#include "StarConfig.hpp"
#include "StarMaterialRenderProfile.hpp"
#include "StarRenderer.hpp"
#include "StarTileDrawer.hpp"
#include "StarTtlCache.hpp"
#include "StarWorldCamera.hpp"
#include "StarWorldRenderData.hpp"

import std;

namespace Star {

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

  TilePainter(Ptr<Renderer> renderer);

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
  using QuadZLevel = std::uint64_t;
  using ChunkHash = std::uint64_t;

  enum class TerrainLayer { Background,
                            Midground,
                            Foreground };

  struct LiquidInfo {
    RefPtr<Texture> texture;
    Vec4B color;
    Vec3F bottomLightMix;
    float textureMovementFactor;
  };

  using TerrainChunk = HashMap<TerrainLayer, HashMap<QuadZLevel, Ptr<RenderBuffer>>>;
  using LiquidChunk = HashMap<LiquidId, Ptr<RenderBuffer>>;

  using MaterialPieceTextureKey = std::tuple<MaterialId, MaterialRenderPieceIndex, MaterialHue, bool>;
  using AssetTextureKey = String;
  using TextureKey = Variant<MaterialPieceTextureKey, AssetTextureKey>;

  struct TextureKeyHash {
    auto operator()(TextureKey const& key) const -> size_t;
  };

  // chunkIndex here is the index of the render chunk such that chunkIndex *
  // RenderChunkSize results in the coordinate of the lower left most tile in
  // the render chunk.

  static auto terrainChunkHash(WorldRenderData& renderData, Vec2I chunkIndex) -> ChunkHash;
  static auto liquidChunkHash(WorldRenderData& renderData, Vec2I chunkIndex) -> ChunkHash;

  void renderTerrainChunks(WorldCamera const& camera, TerrainLayer terrainLayer);

  auto getTerrainChunk(WorldRenderData& renderData, Vec2I chunkIndex) -> std::shared_ptr<TerrainChunk const>;
  auto getLiquidChunk(WorldRenderData& renderData, Vec2I chunkIndex) -> std::shared_ptr<LiquidChunk const>;

  auto produceTerrainPrimitives(HashMap<QuadZLevel, List<RenderPrimitive>>& primitives,
                                TerrainLayer terrainLayer, Vec2I const& pos, WorldRenderData const& renderData) -> bool;
  void produceLiquidPrimitives(HashMap<LiquidId, List<RenderPrimitive>>& primitives, Vec2I const& pos, WorldRenderData const& renderData);

  [[nodiscard]] auto liquidDrawLevel(float liquidLevel) const -> float;

  List<LiquidInfo> m_liquids;

  Ptr<Renderer> m_renderer;
  Ptr<TextureGroup> m_textureGroup;

  HashTtlCache<TextureKey, RefPtr<Texture>, TextureKeyHash> m_textureCache;
  HashTtlCache<std::pair<Vec2I, ChunkHash>, std::shared_ptr<TerrainChunk const>> m_terrainChunkCache;
  HashTtlCache<std::pair<Vec2I, ChunkHash>, std::shared_ptr<LiquidChunk const>> m_liquidChunkCache;

  List<std::shared_ptr<TerrainChunk const>> m_pendingTerrainChunks;
  List<std::shared_ptr<LiquidChunk const>> m_pendingLiquidChunks;

  std::optional<Vec2F> m_lastCameraCenter;
  Vec2F m_cameraPan;
};

}// namespace Star
