#pragma once

#include "StarAssets.hpp"
#include "StarBiomeDatabase.hpp"
#include "StarCelestialParameters.hpp"
#include "StarGameTypes.hpp"
#include "StarPerlin.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarWeatherTypes.hpp"

namespace Star {

struct Biome;
using BiomeConstPtr = SharedPtr<Biome const>;
struct TerrainSelector;
using TerrainSelectorConstPtr = SharedPtr<TerrainSelector const>;
struct WorldRegion;
using WorldRegionPtr = SharedPtr<WorldRegion>;
class WorldLayout;
using WorldLayoutPtr = SharedPtr<WorldLayout>;

using BiomeIndex = uint8_t;
BiomeIndex const NullBiomeIndex = 0;

using TerrainSelectorIndex = uint32_t;
TerrainSelectorIndex const NullTerrainSelectorIndex = 0;

struct WorldRegionLiquids {
  LiquidId caveLiquid = EmptyLiquidId;
  float caveLiquidSeedDensity = 0.0f;

  LiquidId oceanLiquid = EmptyLiquidId;
  int oceanLiquidLevel = 0;

  bool encloseLiquids = false;
  bool fillMicrodungeons = false;
};

struct WorldRegion {
  WorldRegion() = default;
  explicit WorldRegion(Json const& store);

  [[nodiscard]] Json toJson() const;

  TerrainSelectorIndex terrainSelectorIndex = NullTerrainSelectorIndex;
  TerrainSelectorIndex foregroundCaveSelectorIndex = NullTerrainSelectorIndex;
  TerrainSelectorIndex backgroundCaveSelectorIndex = NullTerrainSelectorIndex;

  BiomeIndex blockBiomeIndex = NullBiomeIndex;
  BiomeIndex environmentBiomeIndex = NullBiomeIndex;

  List<TerrainSelectorIndex> subBlockSelectorIndexes;
  List<TerrainSelectorIndex> foregroundOreSelectorIndexes;
  List<TerrainSelectorIndex> backgroundOreSelectorIndexes;

  WorldRegionLiquids regionLiquids;
};

class WorldLayout {
public:
  struct BlockNoise {
    [[nodiscard]] static BlockNoise build(Json const& config, uint64_t seed);

    BlockNoise() = default;
    explicit BlockNoise(Json const& store);

    [[nodiscard]] Json toJson() const;

    [[nodiscard]] Vec2I apply(Vec2I const& input, Vec2U const& worldSize) const;

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

  [[nodiscard]] static WorldLayout buildTerrestrialLayout(AssetsConstPtr assets, TerrainDatabaseConstPtr terrainDatabase, BiomeDatabaseConstPtr biomeDatabase, TerrestrialWorldParameters const& terrestrialParameters, uint64_t seed);
  [[nodiscard]] static WorldLayout buildAsteroidsLayout(AssetsConstPtr assets, TerrainDatabaseConstPtr terrainDatabase, BiomeDatabaseConstPtr biomeDatabase, AsteroidsWorldParameters const& asteroidParameters, uint64_t seed);
  [[nodiscard]] static WorldLayout buildFloatingDungeonLayout(AssetsConstPtr assets, TerrainDatabaseConstPtr terrainDatabase, BiomeDatabaseConstPtr biomeDatabase, FloatingDungeonWorldParameters const& floatingDungeonParameters, uint64_t seed);

  WorldLayout() = default;
  WorldLayout(Json const& store, TerrainDatabaseConstPtr terrainDatabase, BiomeDatabaseConstPtr biomeDatabase);

  [[nodiscard]] Json toJson() const;

  [[nodiscard]] Maybe<BlockNoise> const& blockNoise() const;
  [[nodiscard]] Maybe<PerlinF> const& blendNoise() const;

  [[nodiscard]] List<RectI> playerStartSearchRegions() const;

  [[nodiscard]] BiomeConstPtr const& getBiome(BiomeIndex index) const;
  [[nodiscard]] TerrainSelectorConstPtr const& getTerrainSelector(TerrainSelectorIndex index) const;

  // Will return region weighting in order of greatest to least weighting.
  [[nodiscard]] List<RegionWeighting> getWeighting(int x, int y) const;

  [[nodiscard]] List<RectI> previewAddBiomeRegion(Vec2I const& position, int width) const;
  [[nodiscard]] List<RectI> previewExpandBiomeRegion(Vec2I const& position, int width) const;

  void addBiomeRegion(TerrestrialWorldParameters const& terrestrialParameters, uint64_t seed, Vec2I const& position, String biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  // sets the environment biome index for all regions in the current layer
  // to the biome at the specified position, and returns the name of the biome
  [[nodiscard]] String setLayerEnvironmentBiome(Vec2I const& position);

  [[nodiscard]] pair<size_t, size_t> findLayerAndCell(int x, int y) const;

private:
  struct WorldLayer {
    WorldLayer() = default;

    int yStart = 0;
    Deque<int> boundaries;
    Deque<WorldRegionPtr> cells;
  };

  struct RegionParams {
    int baseHeight = 0;
    float threatLevel = 0.0f;
    Maybe<String> biomeName;
    Maybe<String> terrainSelector;
    Maybe<String> fgCaveSelector;
    Maybe<String> bgCaveSelector;
    Maybe<String> fgOreSelector;
    Maybe<String> bgOreSelector;
    Maybe<String> subBlockSelector;
    WorldRegionLiquids regionLiquids;
  };

  [[nodiscard]] pair<WorldLayer, List<RectI>> expandRegionInLayer(WorldLayer targetLayer, size_t cellIndex, int newWidth) const;

  [[nodiscard]] BiomeIndex registerBiome(BiomeConstPtr biome);
  [[nodiscard]] TerrainSelectorIndex registerTerrainSelector(TerrainSelectorConstPtr terrainSelector);

  [[nodiscard]] WorldRegion buildRegion(uint64_t seed, RegionParams const& regionParams);
  void addLayer(uint64_t seed, int yStart, RegionParams regionParams);
  void addLayer(uint64_t seed, int yStart, int yBase, String const& primaryBiome,
                RegionParams primaryRegionParams, RegionParams primarySubRegionParams,
                List<RegionParams> secondaryRegions, List<RegionParams> secondarySubRegions,
                Vec2F secondaryRegionSize, Vec2F subRegionSize,
                bool useSecondaryEnvironmentBiomeIndex, int playerStartSearchYRange);
  void finalize(Color mainSkyColor);

  [[nodiscard]] pair<size_t, int> findContainingCell(WorldLayer const& layer, int x) const;
  [[nodiscard]] pair<size_t, int> leftCell(WorldLayer const& layer, size_t cellIndex, int x) const;
  [[nodiscard]] pair<size_t, int> rightCell(WorldLayer const& layer, size_t cellIndex, int x) const;

  Vec2U m_worldSize;

  List<BiomeConstPtr> m_biomes;
  List<TerrainSelectorConstPtr> m_terrainSelectors;

  List<WorldLayer> m_layers;

  float m_regionBlending = 0.0f;
  Maybe<BlockNoise> m_blockNoise;
  Maybe<PerlinF> m_blendNoise;
  List<RectI> m_playerStartSearchRegions;

  TerrainDatabaseConstPtr m_terrainDatabase;
  BiomeDatabaseConstPtr m_biomeDatabase;
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

}// namespace Star
