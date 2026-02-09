#pragma once

#include "StarConfig.hpp"
#include "StarMaterialRenderProfile.hpp"
#include "StarWorldRenderData.hpp"

import std;

namespace Star {

class MaterialDatabase;

class TileDrawer {
public:
  using QuadZLevel = std::uint64_t;
  using Drawables = HashMap<QuadZLevel, List<Drawable>>;

  using MaterialRenderPieceIndex = size_t;
  using MaterialPieceResultList = List<std::pair<ConstPtr<MaterialRenderPiece>, Vec2F>>;

  enum class TerrainLayer { Background,
                            Midground,
                            Foreground };

  static RenderTile DefaultRenderTile;

  static auto singletonPtr() -> TileDrawer*;
  static auto singleton() -> TileDrawer&;

  TileDrawer();
  ~TileDrawer();

  auto produceTerrainDrawables(Drawables& drawables, TerrainLayer terrainLayer, Vec2I const& pos,
                               WorldRenderData const& renderData, float scale = 1.0f, Vec2I variantOffset = {}, std::optional<TerrainLayer> variantLayer = {}) -> bool;

  auto renderData() -> WorldRenderData&;
  auto lockRenderData() -> MutexLocker;

  template <typename Function>
  static void forEachRenderTile(WorldRenderData const& renderData, RectI const& worldCoordRange, Function&& function);

private:
  friend class TilePainter;

  static TileDrawer* s_singleton;

  static auto getRenderTile(WorldRenderData const& renderData, Vec2I const& worldPos) -> RenderTile const&;

  static auto materialZLevel(std::uint32_t zLevel, MaterialId material, MaterialHue hue, MaterialColorVariant colorVariant) -> QuadZLevel;
  static auto modZLevel(std::uint32_t zLevel, ModId mod, MaterialHue hue, MaterialColorVariant colorVariant) -> QuadZLevel;
  static auto damageZLevel() -> QuadZLevel;

  static auto determineMatchingPieces(MaterialPieceResultList& resultList, bool* occlude, ConstPtr<MaterialDatabase> const& materialDb, MaterialRenderMatchList const& matchList,
                                      WorldRenderData const& renderData, Vec2I const& basePos, TileLayer layer, bool isMod) -> bool;

  Vec4B m_backgroundLayerColor;
  Vec4B m_foregroundLayerColor;
  Vec2F m_liquidDrawLevels;

  WorldRenderData m_tempRenderData;
  Mutex m_tempRenderDataMutex;
};

template <typename Function>
void TileDrawer::forEachRenderTile(WorldRenderData const& renderData, RectI const& worldCoordRange, Function&& function) {
  RectI indexRect = RectI::withSize(renderData.geometry.diff(worldCoordRange.min(), renderData.tileMinPosition), worldCoordRange.size());
  indexRect.limit(RectI::withSize(Vec2I(0, 0), Vec2I(renderData.tiles.size())));

  if (!indexRect.isEmpty()) {
    renderData.tiles.forEach(Array2S(indexRect.min()), Array2S(indexRect.size()), [&](Array2S const& index, RenderTile const& tile) -> auto {
      return function(worldCoordRange.min() + (Vec2I(index) - indexRect.min()), tile);
    });
  }
}

}// namespace Star
