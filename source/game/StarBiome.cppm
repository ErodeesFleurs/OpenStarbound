module;
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;
import star.weighted_pool;
#include "StarBiMap.hpp"
#include "StarIdMap.hpp"
#include "StarSet.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarStrongTypedef.hpp"


import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_damage;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;

import star.spawn_type_database;

namespace Star {

STAR_STRUCT(AmbientNoisesDescription);
STAR_CLASS(Parallax);
STAR_STRUCT(BiomePlaceables);
STAR_STRUCT(Biome);

struct BiomePlaceables {
  BiomePlaceables();
  explicit BiomePlaceables(Json const& json);

  Json toJson() const;

  // If any of the item distributions contain trees, this returns the first
  // tree type.
  Maybe<TreeVariant> firstTreeType() const;

  ModId grassMod;
  float grassModDensity;
  ModId ceilingGrassMod;
  float ceilingGrassModDensity;

  List<BiomeItemDistribution> itemDistributions;
};

struct Biome {
  Biome();
  explicit Biome(Json const& store);

  Json toJson() const;

  String baseName;
  String description;

  MaterialId mainBlock;
  List<MaterialId> subBlocks;
  // Pairs the ore type with the commonality multiplier.
  List<pair<ModId, float>> ores;

  float hueShift;
  MaterialHue materialHueShift;

  BiomePlaceables surfacePlaceables;
  BiomePlaceables undergroundPlaceables;

  SpawnProfile spawnProfile;

  ParallaxPtr parallax;

  AmbientNoisesDescriptionPtr ambientNoises;
  AmbientNoisesDescriptionPtr musicTrack;
};

}

export module star.biome;

export namespace Star {
  using ::Star::AmbientNoisesDescription;
  using ::Star::AmbientNoisesDescriptionPtr;
  using ::Star::AmbientNoisesDescriptionConstPtr;
  using ::Star::AmbientNoisesDescriptionWeakPtr;
  using ::Star::AmbientNoisesDescriptionConstWeakPtr;
  using ::Star::AmbientNoisesDescriptionUPtr;
  using ::Star::AmbientNoisesDescriptionConstUPtr;
  using ::Star::Parallax;
  using ::Star::ParallaxPtr;
  using ::Star::ParallaxConstPtr;
  using ::Star::ParallaxWeakPtr;
  using ::Star::ParallaxConstWeakPtr;
  using ::Star::ParallaxUPtr;
  using ::Star::ParallaxConstUPtr;
  using ::Star::BiomePlaceables;
  using ::Star::BiomePlaceablesPtr;
  using ::Star::BiomePlaceablesConstPtr;
  using ::Star::BiomePlaceablesWeakPtr;
  using ::Star::BiomePlaceablesConstWeakPtr;
  using ::Star::BiomePlaceablesUPtr;
  using ::Star::BiomePlaceablesConstUPtr;
  using ::Star::Biome;
  using ::Star::BiomePtr;
  using ::Star::BiomeConstPtr;
  using ::Star::BiomeWeakPtr;
  using ::Star::BiomeConstWeakPtr;
  using ::Star::BiomeUPtr;
  using ::Star::BiomeConstUPtr;
}
