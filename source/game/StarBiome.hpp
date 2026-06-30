#pragma once

#include "StarBiomePlacement.hpp"
#include "StarSpawner.hpp"

namespace Star {

struct AmbientNoisesDescription;
using AmbientNoisesDescriptionPtr = SharedPtr<AmbientNoisesDescription>;
class Parallax;
using ParallaxPtr = SharedPtr<Parallax>;
struct BiomePlaceables;
struct Biome;
using BiomePtr = SharedPtr<Biome>;
using BiomeConstPtr = SharedPtr<Biome const>;

struct BiomePlaceables {
  BiomePlaceables() = default;
  explicit BiomePlaceables(Json const& json);

  Json toJson() const;

  // If any of the item distributions contain trees, this returns the first
  // tree type.
  Maybe<TreeVariant> firstTreeType() const;

  ModId grassMod = NoModId;
  float grassModDensity = 0.0f;
  ModId ceilingGrassMod = NoModId;
  float ceilingGrassModDensity = 0.0f;

  List<BiomeItemDistribution> itemDistributions;
};

struct Biome {
  Biome() = default;
  explicit Biome(Json const& store);

  Json toJson() const;

  String baseName;
  String description;

  MaterialId mainBlock = EmptyMaterialId;
  List<MaterialId> subBlocks;
  // Pairs the ore type with the commonality multiplier.
  List<pair<ModId, float>> ores;

  float hueShift = 0.0f;
  MaterialHue materialHueShift{};

  BiomePlaceables surfacePlaceables;
  BiomePlaceables undergroundPlaceables;

  SpawnProfile spawnProfile;

  ParallaxPtr parallax;

  AmbientNoisesDescriptionPtr ambientNoises;
  AmbientNoisesDescriptionPtr musicTrack;
};

}
