module;
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarEither.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarThread.hpp"
#include "StarEither.hpp"
#include "StarRect.hpp"
#include "StarImage.hpp"
#include "StarColor.hpp"
#include "StarInterpolation.hpp"
#include "StarList.hpp"
#include "StarVector.hpp"
#include "StarThread.hpp"
#include <functional>
#include "StarSet.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
import star.perlin;
#include "StarVariant.hpp"


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
import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.animation;
import star.particle;
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
