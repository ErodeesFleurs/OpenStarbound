#pragma once

#include "StarBiomePlacement.hpp"
#include "StarConfig.hpp"
#include "StarMaterialTypes.hpp"
#include "StarSpawnTypeDatabase.hpp"

import std;

namespace Star {

class Parallax;
struct AmbientNoisesDescription;

struct BiomePlaceables {
  BiomePlaceables();
  explicit BiomePlaceables(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  // If any of the item distributions contain trees, this returns the first
  // tree type.
  [[nodiscard]] auto firstTreeType() const -> std::optional<TreeVariant>;

  ModId grassMod;
  float grassModDensity;
  ModId ceilingGrassMod;
  float ceilingGrassModDensity;

  List<BiomeItemDistribution> itemDistributions;
};

struct Biome {
  Biome();
  explicit Biome(Json const& store);

  [[nodiscard]] auto toJson() const -> Json;

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

  Ptr<Parallax> parallax;

  Ptr<AmbientNoisesDescription> ambientNoises;
  Ptr<AmbientNoisesDescription> musicTrack;
};

}// namespace Star
