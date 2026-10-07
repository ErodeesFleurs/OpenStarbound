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

namespace Star {

STAR_CLASS(BiomeItemDistribution);

struct BiomeExceptionTag {
  static constexpr char const* name() { return "BiomeException"; }
};
using BiomeException = StarError<BiomeExceptionTag, StarException>;

typedef pair<TreeVariant, TreeVariant> TreePair;

// Weighted pairs of object name / parameters.
typedef WeightedPool<pair<String, Json>> ObjectPool;

strong_typedef(String, TreasureBoxSet);
strong_typedef(StringSet, MicroDungeonNames);

typedef Variant<GrassVariant, BushVariant, TreePair, ObjectPool, TreasureBoxSet, MicroDungeonNames> BiomeItem;
BiomeItem variantToBiomeItem(Json const& store);
Json variantFromBiomeItem(BiomeItem const& biomeItem);

enum class BiomePlacementArea { Surface, Underground };
enum class BiomePlacementMode { Floor, Ceiling, Background, Ocean };
extern EnumMap<BiomePlacementMode> const BiomePlacementModeNames;

struct BiomeItemPlacement {
  BiomeItemPlacement(BiomeItem item, Vec2I position, float priority);

  // Orders by priority
  bool operator<(BiomeItemPlacement const& rhs) const;

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

  static Maybe<BiomeItem> createItem(Json const& itemSettings, RandomSource& rand, float biomeHueShift);

  BiomeItemDistribution();
  BiomeItemDistribution(Json const& config, uint64_t seed, float biomeHueShift = 0.0f);
  BiomeItemDistribution(Json const& store);

  Json toJson() const;

  BiomePlacementMode mode() const;
  List<BiomeItem> allItems() const;

  // Returns the best BiomeItem for this position out of the weighted item set,
  // if the density function specifies that an item should go in this position.
  Maybe<BiomeItemPlacement> itemToPlace(int x, int y) const;

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
  uint64_t m_blockSeed;
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
  List<pair<BiomeItem, PerlinF>> m_weightedItems;
};

}

export module star.biome_placement;

export namespace Star {
  using ::Star::BiomeItemDistribution;
  using ::Star::BiomeItemDistributionPtr;
  using ::Star::BiomeItemDistributionConstPtr;
  using ::Star::BiomeItemDistributionWeakPtr;
  using ::Star::BiomeItemDistributionConstWeakPtr;
  using ::Star::BiomeItemDistributionUPtr;
  using ::Star::BiomeItemDistributionConstUPtr;
  using ::Star::BiomeExceptionTag;
  using ::Star::BiomeException;
  using ::Star::TreePair;
  using ::Star::ObjectPool;
  using ::Star::TreasureBoxSetWrapper;
  using ::Star::TreasureBoxSet;
  using ::Star::MicroDungeonNamesWrapper;
  using ::Star::MicroDungeonNames;
  using ::Star::BiomeItem;
  using ::Star::variantToBiomeItem;
  using ::Star::variantFromBiomeItem;
  using ::Star::BiomePlacementArea;
  using ::Star::BiomePlacementMode;
  using ::Star::BiomePlacementModeNames;
  using ::Star::BiomeItemPlacement;
}
