module;

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
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
#include "StarGameTypes.hpp"
import star.entity_rendering_types;
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"
import star.sky_types;
#include "StarEither.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
import star.sky_parameters;
import star.sky_render_data;
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarDirectives.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"
import star.tile_damage;
import star.plant_database;
import star.parallax;
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
import star.animation;
import star.particle;
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
import star.animation;
import star.particle;
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

namespace Star {

struct EntityDrawables {
  EntityHighlightEffect highlightEffect;
  Map<EntityRenderLayer, List<Drawable>> layers;
};


struct WorldRenderData {
  void clear();

  WorldGeometry geometry;

  Vec2I tileMinPosition;
  RenderTileArray tiles;
  Vec2I lightMinPosition;
  Lightmap lightMap;

  List<EntityDrawables> entityDrawables;
  List<Particle> const* particles;

  List<OverheadBar> overheadBars;
  List<Drawable> nametags;

  List<Drawable> backgroundOverlays;
  List<Drawable> foregroundOverlays;

  List<ParallaxLayer> parallaxLayers;

  SkyRenderData skyRenderData;

  bool isFullbright = false;
  float dimLevel = 0.0f;
  Vec3B dimColor;
};

inline void WorldRenderData::clear() {
  tiles.resize({0, 0}); // keep reserved

  entityDrawables.clear();
  particles = nullptr;
  overheadBars.clear();
  nametags.clear();
  backgroundOverlays.clear();
  foregroundOverlays.clear();
  parallaxLayers.clear();
}

}

export module star.world_render_data;

export namespace Star {
  using ::Star::EntityDrawables;
  using ::Star::WorldRenderData;
}
