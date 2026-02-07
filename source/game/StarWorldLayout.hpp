#pragma once

#include "StarConfig.hpp"
#include "StarLiquidTypes.hpp"
#include "StarPerlin.hpp"
#include "StarVector.hpp"
#include "StarWorldParameters.hpp"

import std;

namespace Star {

class Biome;
class TerrainSelector;

using BiomeIndex = std::uint8_t;
BiomeIndex const NullBiomeIndex = 0;

using TerrainSelectorIndex = std::uint32_t;
TerrainSelectorIndex const NullTerrainSelectorIndex = 0;

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

  [[nodiscard]] auto toJson() const -> Json;

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
public:
  struct BlockNoise {
    static auto build(Json const& config, std::uint64_t seed) -> BlockNoise;

    BlockNoise();
    explicit BlockNoise(Json const& store);

    [[nodiscard]] auto toJson() const -> Json;

    [[nodiscard]] auto apply(Vec2I const& input, Vec2U const& worldSize) const -> Vec2I;

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

  static auto buildTerrestrialLayout(TerrestrialWorldParameters const& terrestrialParameters, std::uint64_t seed) -> WorldLayout;
  static auto buildAsteroidsLayout(AsteroidsWorldParameters const& asteroidParameters, std::uint64_t seed) -> WorldLayout;
  static auto buildFloatingDungeonLayout(FloatingDungeonWorldParameters const& floatingDungeonParameters, std::uint64_t seed) -> WorldLayout;

  WorldLayout();
  WorldLayout(Json const& store);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto blockNoise() const -> std::optional<BlockNoise> const&;
  [[nodiscard]] auto blendNoise() const -> std::optional<PerlinF> const&;

  [[nodiscard]] auto playerStartSearchRegions() const -> List<RectI>;

  [[nodiscard]] auto getBiome(BiomeIndex index) const -> ConstPtr<Biome> const&;
  [[nodiscard]] auto getTerrainSelector(TerrainSelectorIndex index) const -> ConstPtr<TerrainSelector> const&;

  // Will return region weighting in order of greatest to least weighting.
  [[nodiscard]] auto getWeighting(int x, int y) const -> List<RegionWeighting>;

  [[nodiscard]] auto previewAddBiomeRegion(Vec2I const& position, int width) const -> List<RectI>;
  [[nodiscard]] auto previewExpandBiomeRegion(Vec2I const& position, int width) const -> List<RectI>;

  void addBiomeRegion(TerrestrialWorldParameters const& terrestrialParameters, uint64_t seed, Vec2I const& position, String biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  // sets the environment biome index for all regions in the current layer
  // to the biome at the specified position, and returns the name of the biome
  auto setLayerEnvironmentBiome(Vec2I const& position) -> String;

  [[nodiscard]] auto findLayerAndCell(int x, int y) const -> std::pair<size_t, size_t>;

private:
  struct WorldLayer {
    WorldLayer();

    int yStart;
    Deque<int> boundaries;
    Deque<Ptr<WorldRegion>> cells;
  };

  struct RegionParams {
    int baseHeight;
    float threatLevel;
    std::optional<String> biomeName;
    std::optional<String> terrainSelector;
    std::optional<String> fgCaveSelector;
    std::optional<String> bgCaveSelector;
    std::optional<String> fgOreSelector;
    std::optional<String> bgOreSelector;
    std::optional<String> subBlockSelector;
    WorldRegionLiquids regionLiquids;
  };

  [[nodiscard]] auto expandRegionInLayer(WorldLayer targetLayer, size_t cellIndex, int newWidth) const -> std::pair<WorldLayer, List<RectI>>;

  auto registerBiome(ConstPtr<Biome> biome) -> BiomeIndex;
  auto registerTerrainSelector(ConstPtr<TerrainSelector> terrainSelector) -> TerrainSelectorIndex;

  auto buildRegion(uint64_t seed, RegionParams const& regionParams) -> WorldRegion;
  void addLayer(uint64_t seed, int yStart, RegionParams regionParams);
  void addLayer(uint64_t seed, int yStart, int yBase, String const& primaryBiome,
                RegionParams primaryRegionParams, RegionParams primarySubRegionParams,
                List<RegionParams> secondaryRegions, List<RegionParams> secondarySubRegions,
                Vec2F secondaryRegionSize, Vec2F subRegionSize);
  void finalize(Color mainSkyColor);

  [[nodiscard]] auto findContainingCell(WorldLayer const& layer, int x) const -> std::pair<size_t, int>;
  [[nodiscard]] auto leftCell(WorldLayer const& layer, size_t cellIndex, int x) const -> std::pair<size_t, int>;
  [[nodiscard]] auto rightCell(WorldLayer const& layer, size_t cellIndex, int x) const -> std::pair<size_t, int>;

  Vec2U m_worldSize;

  List<ConstPtr<Biome>> m_biomes;
  List<ConstPtr<TerrainSelector>> m_terrainSelectors;

  List<WorldLayer> m_layers;

  float m_regionBlending;
  std::optional<BlockNoise> m_blockNoise;
  std::optional<PerlinF> m_blendNoise;
  List<RectI> m_playerStartSearchRegions;
};

auto operator>>(DataStream& ds, WorldLayout& worldTemplateDescriptor) -> DataStream&;
auto operator<<(DataStream& ds, WorldLayout& worldTemplateDescriptor) -> DataStream&;

inline auto WorldLayout::getBiome(BiomeIndex index) const -> ConstPtr<Biome> const& {
  if (index == NullBiomeIndex || index > m_biomes.size())
    throw StarException("WorldLayout::getTerrainSelector called with null or out of range BiomeIndex");
  return m_biomes[index - 1];
}

inline auto WorldLayout::getTerrainSelector(TerrainSelectorIndex index) const -> ConstPtr<TerrainSelector> const& {
  if (index == NullBiomeIndex || index > m_terrainSelectors.size())
    throw StarException("WorldLayout::getTerrainSelector called with null or out of range TerrainSelectorIndex");
  return m_terrainSelectors[index - 1];
}

}// namespace Star
