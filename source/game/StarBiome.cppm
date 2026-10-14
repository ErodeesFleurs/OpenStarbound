module;
import std;
import star.string;
import star.maybe;
import star.json;
import star.bi_map;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.id_map;
import star.set;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
import star.material_types;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
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

struct AmbientNoisesDescription;
using AmbientNoisesDescriptionPtr = std::shared_ptr<AmbientNoisesDescription>;
using AmbientNoisesDescriptionConstPtr = std::shared_ptr<const AmbientNoisesDescription>;
using AmbientNoisesDescriptionWeakPtr = std::weak_ptr<AmbientNoisesDescription>;
using AmbientNoisesDescriptionConstWeakPtr = std::weak_ptr<const AmbientNoisesDescription>;
using AmbientNoisesDescriptionUPtr = std::unique_ptr<AmbientNoisesDescription>;
using AmbientNoisesDescriptionConstUPtr = std::unique_ptr<const AmbientNoisesDescription>;
class Parallax;
using ParallaxPtr = std::shared_ptr<Parallax>;
using ParallaxConstPtr = std::shared_ptr<const Parallax>;
using ParallaxWeakPtr = std::weak_ptr<Parallax>;
using ParallaxConstWeakPtr = std::weak_ptr<const Parallax>;
using ParallaxUPtr = std::unique_ptr<Parallax>;
using ParallaxConstUPtr = std::unique_ptr<const Parallax>;
struct BiomePlaceables;
using BiomePlaceablesPtr = std::shared_ptr<BiomePlaceables>;
using BiomePlaceablesConstPtr = std::shared_ptr<const BiomePlaceables>;
using BiomePlaceablesWeakPtr = std::weak_ptr<BiomePlaceables>;
using BiomePlaceablesConstWeakPtr = std::weak_ptr<const BiomePlaceables>;
using BiomePlaceablesUPtr = std::unique_ptr<BiomePlaceables>;
using BiomePlaceablesConstUPtr = std::unique_ptr<const BiomePlaceables>;
struct Biome;
using BiomePtr = std::shared_ptr<Biome>;
using BiomeConstPtr = std::shared_ptr<const Biome>;
using BiomeWeakPtr = std::weak_ptr<Biome>;
using BiomeConstWeakPtr = std::weak_ptr<const Biome>;
using BiomeUPtr = std::unique_ptr<Biome>;
using BiomeConstUPtr = std::unique_ptr<const Biome>;

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
  List<std::pair<ModId, float>> ores;

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
