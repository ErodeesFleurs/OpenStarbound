module;
#include "StarJson.hpp"
#include "StarPerlin.hpp"
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"

import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;

namespace Star {

STAR_STRUCT(Biome);
STAR_STRUCT(TerrainSelector);
STAR_STRUCT(WorldRegion);
STAR_CLASS(WorldLayout);

typedef uint8_t BiomeIndex;
inline constexpr BiomeIndex NullBiomeIndex = 0;

typedef uint32_t TerrainSelectorIndex;
inline constexpr TerrainSelectorIndex NullTerrainSelectorIndex = 0;

struct WorldRegionLiquids {
  LiquidId caveLiquid;
  float caveLiquidSeedDensity;

  LiquidId oceanLiquid;
  int oceanLiquidLevel;

  bool encloseLiquids;
  bool fillMicrodungeons;
};

struct WorldRegion {
  WorldRegion();
  explicit WorldRegion(Json const& store);

  Json toJson() const;

  TerrainSelectorIndex terrainSelectorIndex;
  TerrainSelectorIndex foregroundCaveSelectorIndex;
  TerrainSelectorIndex backgroundCaveSelectorIndex;

  BiomeIndex blockBiomeIndex;
  BiomeIndex environmentBiomeIndex;

  List<TerrainSelectorIndex> subBlockSelectorIndexes;
  List<TerrainSelectorIndex> foregroundOreSelectorIndexes;
  List<TerrainSelectorIndex> backgroundOreSelectorIndexes;

  WorldRegionLiquids regionLiquids;
};

class WorldLayout {
  friend DataStream& operator>>(DataStream& ds, WorldLayout& worldTemplateDescriptor);
  friend DataStream& operator<<(DataStream& ds, WorldLayout& worldTemplateDescriptor);
public:
  struct BlockNoise {
    static BlockNoise build(Json const& config, uint64_t seed);

    BlockNoise();
    explicit BlockNoise(Json const& store);

    Json toJson() const;

    Vec2I apply(Vec2I const& input, Vec2U const& worldSize) const;

    // Individual noise only applied for horizontal / vertical biome transitions
    PerlinF horizontalNoise;
    PerlinF verticalNoise;

    // 2 dimensional biome noise field for fine grained noise
    PerlinF xNoise;
    PerlinF yNoise;
  };

  struct RegionWeighting {
    float weight;
    int xValue;
    WorldRegion const* region;
  };

  static WorldLayout buildTerrestrialLayout(TerrestrialWorldParameters const& terrestrialParameters, uint64_t seed);
  static WorldLayout buildAsteroidsLayout(AsteroidsWorldParameters const& asteroidParameters, uint64_t seed);
  static WorldLayout buildFloatingDungeonLayout(FloatingDungeonWorldParameters const& floatingDungeonParameters, uint64_t seed);

  WorldLayout();
  WorldLayout(Json const& store);

  Json toJson() const;

  Maybe<BlockNoise> const& blockNoise() const;
  Maybe<PerlinF> const& blendNoise() const;

  List<RectI> playerStartSearchRegions() const;

  BiomeConstPtr const& getBiome(BiomeIndex index) const;
  TerrainSelectorConstPtr const& getTerrainSelector(TerrainSelectorIndex index) const;

  // Will return region weighting in order of greatest to least weighting.
  List<RegionWeighting> getWeighting(int x, int y) const;

  List<RectI> previewAddBiomeRegion(Vec2I const& position, int width) const;
  List<RectI> previewExpandBiomeRegion(Vec2I const& position, int width) const;

  void addBiomeRegion(TerrestrialWorldParameters const& terrestrialParameters, uint64_t seed, Vec2I const& position, String biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  // sets the environment biome index for all regions in the current layer
  // to the biome at the specified position, and returns the name of the biome
  String setLayerEnvironmentBiome(Vec2I const& position);

  pair<size_t, size_t> findLayerAndCell(int x, int y) const;

private:
  struct WorldLayer {
    WorldLayer();

    int yStart;
    Deque<int> boundaries;
    Deque<WorldRegionPtr> cells;
  };

  struct RegionParams {
    int baseHeight;
    float threatLevel;
    Maybe<String> biomeName;
    Maybe<String> terrainSelector;
    Maybe<String> fgCaveSelector;
    Maybe<String> bgCaveSelector;
    Maybe<String> fgOreSelector;
    Maybe<String> bgOreSelector;
    Maybe<String> subBlockSelector;
    WorldRegionLiquids regionLiquids;
  };

  pair<WorldLayer, List<RectI>> expandRegionInLayer(WorldLayer targetLayer, size_t cellIndex, int newWidth) const;

  BiomeIndex registerBiome(BiomeConstPtr biome);
  TerrainSelectorIndex registerTerrainSelector(TerrainSelectorConstPtr terrainSelector);

  WorldRegion buildRegion(uint64_t seed, RegionParams const& regionParams);
  void addLayer(uint64_t seed, int yStart, RegionParams regionParams);
  void addLayer(uint64_t seed, int yStart, int yBase, String const& primaryBiome,
      RegionParams primaryRegionParams, RegionParams primarySubRegionParams,
      List<RegionParams> secondaryRegions, List<RegionParams> secondarySubRegions,
      Vec2F secondaryRegionSize, Vec2F subRegionSize);
  void finalize(Color mainSkyColor);

  pair<size_t, int> findContainingCell(WorldLayer const& layer, int x) const;
  pair<size_t, int> leftCell(WorldLayer const& layer, size_t cellIndex, int x) const;
  pair<size_t, int> rightCell(WorldLayer const& layer, size_t cellIndex, int x) const;

  Vec2U m_worldSize;

  List<BiomeConstPtr> m_biomes;
  List<TerrainSelectorConstPtr> m_terrainSelectors;

  List<WorldLayer> m_layers;

  float m_regionBlending;
  Maybe<BlockNoise> m_blockNoise;
  Maybe<PerlinF> m_blendNoise;
  List<RectI> m_playerStartSearchRegions;
};

DataStream& operator>>(DataStream& ds, WorldLayout& worldTemplateDescriptor);
DataStream& operator<<(DataStream& ds, WorldLayout& worldTemplateDescriptor);

inline BiomeConstPtr const& WorldLayout::getBiome(BiomeIndex index) const {
  if (index == NullBiomeIndex || index > m_biomes.size())
    throw StarException("WorldLayout::getTerrainSelector called with null or out of range BiomeIndex");
  return m_biomes[index - 1];
}

inline TerrainSelectorConstPtr const& WorldLayout::getTerrainSelector(TerrainSelectorIndex index) const {
  if (index == NullBiomeIndex || index > m_terrainSelectors.size())
    throw StarException("WorldLayout::getTerrainSelector called with null or out of range TerrainSelectorIndex");
  return m_terrainSelectors[index - 1];
}

}

export module star.world_layout;

export namespace Star {
  using ::Star::Biome;
  using ::Star::BiomePtr;
  using ::Star::BiomeConstPtr;
  using ::Star::BiomeWeakPtr;
  using ::Star::BiomeConstWeakPtr;
  using ::Star::BiomeUPtr;
  using ::Star::BiomeConstUPtr;
  using ::Star::TerrainSelector;
  using ::Star::TerrainSelectorPtr;
  using ::Star::TerrainSelectorConstPtr;
  using ::Star::TerrainSelectorWeakPtr;
  using ::Star::TerrainSelectorConstWeakPtr;
  using ::Star::TerrainSelectorUPtr;
  using ::Star::TerrainSelectorConstUPtr;
  using ::Star::WorldRegion;
  using ::Star::WorldRegionPtr;
  using ::Star::WorldRegionConstPtr;
  using ::Star::WorldRegionWeakPtr;
  using ::Star::WorldRegionConstWeakPtr;
  using ::Star::WorldRegionUPtr;
  using ::Star::WorldRegionConstUPtr;
  using ::Star::WorldLayout;
  using ::Star::WorldLayoutPtr;
  using ::Star::WorldLayoutConstPtr;
  using ::Star::WorldLayoutWeakPtr;
  using ::Star::WorldLayoutConstWeakPtr;
  using ::Star::WorldLayoutUPtr;
  using ::Star::WorldLayoutConstUPtr;
  using ::Star::BiomeIndex;
  using ::Star::NullBiomeIndex;
  using ::Star::TerrainSelectorIndex;
  using ::Star::NullTerrainSelectorIndex;
  using ::Star::WorldRegionLiquids;
}
