#pragma once

#include "StarImage.hpp"
#include "StarWorldTiles.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarDrawable.hpp"
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
#include "StarParallax.hpp"
#include "StarParticle.hpp"
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarParticle.hpp"
import star.weather_types;
#include "StarEntity.hpp"
#include "StarThread.hpp"
#include "StarCellularLighting.hpp"

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
