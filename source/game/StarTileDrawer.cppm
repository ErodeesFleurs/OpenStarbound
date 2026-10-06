module;

#include "StarTtlCache.hpp"
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
#include "StarRect.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
import star.tile_damage;
#include "StarDirectives.hpp"
import star.material_render_profile;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;

namespace Star {

STAR_CLASS(Assets);
STAR_CLASS(MaterialDatabase);
STAR_CLASS(TileDrawer);
class TilePainter;

class TileDrawer {
public:
  typedef uint64_t QuadZLevel;
  typedef HashMap<QuadZLevel, List<Drawable>> Drawables;

  typedef size_t MaterialRenderPieceIndex;
  typedef List<pair<MaterialRenderPieceConstPtr, Vec2F>> MaterialPieceResultList;

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

  static QuadZLevel materialZLevel(uint32_t zLevel, MaterialId material, MaterialHue hue, MaterialColorVariant colorVariant);
  static QuadZLevel modZLevel(uint32_t zLevel, ModId mod, MaterialHue hue, MaterialColorVariant colorVariant);
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
