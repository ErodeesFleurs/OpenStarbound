#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarPerlin.hpp"
#include "StarPlantDatabase.hpp"
#include "StarString.hpp"
#include "StarStrongTypedef.hpp"
#include "StarWeightedPool.hpp"

import std;

namespace Star {

using BiomeException = ExceptionDerived<"BiomeException">;

using TreePair = std::pair<TreeVariant, TreeVariant>;

// Weighted pairs of object name / parameters.
using ObjectPool = WeightedPool<std::pair<String, Json>>;

using TreasureBoxSet = StrongTypedef<String>;
using MicroDungeonNames = StrongTypedef<StringSet>;

using BiomeItem = Variant<GrassVariant, BushVariant, TreePair, ObjectPool, TreasureBoxSet, MicroDungeonNames>;
auto variantToBiomeItem(Json const& store) -> BiomeItem;
auto variantFromBiomeItem(BiomeItem const& biomeItem) -> Json;

enum class BiomePlacementArea { Surface,
                                Underground };
enum class BiomePlacementMode { Floor,
                                Ceiling,
                                Background,
                                Ocean };
extern EnumMap<BiomePlacementMode> const BiomePlacementModeNames;

struct BiomeItemPlacement {
  BiomeItemPlacement(BiomeItem item, Vec2I position, float priority);

  // Orders by priority
  auto operator<(BiomeItemPlacement const& rhs) const -> bool;

  BiomeItem item;
  Vec2I position;
  float priority;
};

class BiomeItemDistribution {
public:
  struct PeriodicWeightedItem {
    BiomeItem item;
    PerlinF weight;
  };

  static auto createItem(Json const& itemSettings, RandomSource& rand, float biomeHueShift) -> std::optional<BiomeItem>;

  BiomeItemDistribution();
  BiomeItemDistribution(Json const& config, std::uint64_t seed, float biomeHueShift = 0.0f);
  BiomeItemDistribution(Json const& store);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto mode() const -> BiomePlacementMode;
  [[nodiscard]] auto allItems() const -> List<BiomeItem>;

  // Returns the best BiomeItem for this position out of the weighted item set,
  // if the density function specifies that an item should go in this position.
  [[nodiscard]] auto itemToPlace(int x, int y) const -> std::optional<BiomeItemPlacement>;

private:
  enum class DistributionType {
    // Pure random distribution
    Random,
    // Uses perlin noise to morph a periodic function into a less predictable
    // periodic clumpy noise.
    Periodic
  };
  static EnumMap<DistributionType> const DistributionTypeNames;

  BiomePlacementMode m_mode;
  DistributionType m_distribution;
  float m_priority;

  // Used if the distribution type is Random

  float m_blockProbability;
  std::uint64_t m_blockSeed;
  List<BiomeItem> m_randomItems;

  // Used if the distribution type is Periodic

  PerlinF m_densityFunction;
  PerlinF m_modulusDistortion;
  int m_modulus;
  int m_modulusOffset;
  // Pairs items with a periodic weight.  Weight will vary over the space of
  // the distribution, If multiple items are present, this can be used to
  // select one of the items (with the highest weight) out of a list of items,
  // causing items to be grouped spatially in a way determined by the shape of
  // each weight function.
  List<std::pair<BiomeItem, PerlinF>> m_weightedItems;
};

}// namespace Star
