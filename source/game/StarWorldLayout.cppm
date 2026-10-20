module;
import std;
import star.string;
import star.format;
import star.json;
import star.bi_map;
import star.random;
import star.perlin;
import star.maybe;
import star.weighted_pool;
import star.color;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.game_types;
import star.liquid_types;
import star.vector;
import star.variant;

import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;

namespace Star {

struct Biome;
struct TerrainSelector;

typedef std::uint8_t BiomeIndex;
inline constexpr BiomeIndex NullBiomeIndex = 0;

typedef std::uint32_t TerrainSelectorIndex;
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

class Root;
class WorldLayout {
  Root& m_root;
  Root& root() const { return m_root; }
  friend DataStream& operator>>(DataStream& ds, WorldLayout& worldTemplateDescriptor);
  friend DataStream& operator<<(DataStream& ds, WorldLayout& worldTemplateDescriptor);
public:
  struct BlockNoise {
    static BlockNoise build(Json const& config, std::uint64_t seed);

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

  static WorldLayout buildTerrestrialLayout(Root& root, TerrestrialWorldParameters const& terrestrialParameters, std::uint64_t seed);
  static WorldLayout buildAsteroidsLayout(Root& root, AsteroidsWorldParameters const& asteroidParameters, std::uint64_t seed);
  static WorldLayout buildFloatingDungeonLayout(Root& root, FloatingDungeonWorldParameters const& floatingDungeonParameters, std::uint64_t seed);

  WorldLayout(Root& root);
  WorldLayout(Root& root, Json const& store);

  Json toJson() const;

  Maybe<BlockNoise> const& blockNoise() const;
  Maybe<PerlinF> const& blendNoise() const;

  List<RectI> playerStartSearchRegions() const;

  std::shared_ptr<Biome const> const& getBiome(BiomeIndex index) const;
  std::shared_ptr<TerrainSelector const> const& getTerrainSelector(TerrainSelectorIndex index) const;

  // Will return region weighting in order of greatest to least weighting.
  List<RegionWeighting> getWeighting(int x, int y) const;

  List<RectI> previewAddBiomeRegion(Vec2I const& position, int width) const;
  List<RectI> previewExpandBiomeRegion(Vec2I const& position, int width) const;

  void addBiomeRegion(TerrestrialWorldParameters const& terrestrialParameters, std::uint64_t seed, Vec2I const& position, String biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  // sets the environment biome index for all regions in the current layer
  // to the biome at the specified position, and returns the name of the biome
  String setLayerEnvironmentBiome(Vec2I const& position);

  std::pair<std::size_t, std::size_t> findLayerAndCell(int x, int y) const;

private:
  struct WorldLayer {
    WorldLayer();

    int yStart;
    Deque<int> boundaries;
    Deque<std::shared_ptr<WorldRegion>> cells;
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

  std::pair<WorldLayer, List<RectI>> expandRegionInLayer(WorldLayer targetLayer, std::size_t cellIndex, int newWidth) const;

  BiomeIndex registerBiome(std::shared_ptr<Biome const> biome);
  TerrainSelectorIndex registerTerrainSelector(std::shared_ptr<TerrainSelector const> terrainSelector);

  WorldRegion buildRegion(std::uint64_t seed, RegionParams const& regionParams);
  void addLayer(std::uint64_t seed, int yStart, RegionParams regionParams);
  void addLayer(std::uint64_t seed, int yStart, int yBase, String const& primaryBiome,
      RegionParams primaryRegionParams, RegionParams primarySubRegionParams,
      List<RegionParams> secondaryRegions, List<RegionParams> secondarySubRegions,
      Vec2F secondaryRegionSize, Vec2F subRegionSize);
  void finalize(Color mainSkyColor);

  std::pair<std::size_t, int> findContainingCell(WorldLayer const& layer, int x) const;
  std::pair<std::size_t, int> leftCell(WorldLayer const& layer, std::size_t cellIndex, int x) const;
  std::pair<std::size_t, int> rightCell(WorldLayer const& layer, std::size_t cellIndex, int x) const;

  Vec2U m_worldSize;

  List<std::shared_ptr<Biome const>> m_biomes;
  List<std::shared_ptr<TerrainSelector const>> m_terrainSelectors;

  List<WorldLayer> m_layers;

  float m_regionBlending;
  Maybe<BlockNoise> m_blockNoise;
  Maybe<PerlinF> m_blendNoise;
  List<RectI> m_playerStartSearchRegions;
};

DataStream& operator>>(DataStream& ds, WorldLayout& worldTemplateDescriptor);
DataStream& operator<<(DataStream& ds, WorldLayout& worldTemplateDescriptor);

inline std::shared_ptr<Biome const> const& WorldLayout::getBiome(BiomeIndex index) const {
  if (index == NullBiomeIndex || index > m_biomes.size())
    throw StarException("WorldLayout::getTerrainSelector called with null or out of range BiomeIndex");
  return m_biomes[index - 1];
}

inline std::shared_ptr<TerrainSelector const> const& WorldLayout::getTerrainSelector(TerrainSelectorIndex index) const {
  if (index == NullBiomeIndex || index > m_terrainSelectors.size())
    throw StarException("WorldLayout::getTerrainSelector called with null or out of range TerrainSelectorIndex");
  return m_terrainSelectors[index - 1];
}

}

export module star.world_layout;

export namespace Star {
  using ::Star::Biome;
  using ::Star::TerrainSelector;
  using ::Star::WorldRegion;
  using ::Star::WorldLayout;
  using ::Star::BiomeIndex;
  using ::Star::NullBiomeIndex;
  using ::Star::TerrainSelectorIndex;
  using ::Star::NullTerrainSelectorIndex;
  using ::Star::WorldRegionLiquids;
}
