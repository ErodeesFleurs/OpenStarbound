#pragma once

#include "StarAmbient.hpp"
#include "StarBiomePlacement.hpp"
#include "StarCelestialDatabase.hpp"
#include "StarConfig.hpp"
#include "StarLruCache.hpp"
#include "StarMaterialTypes.hpp"
#include "StarSkyParameters.hpp"
#include "StarWorldGeometry.hpp"
#include "StarWorldLayout.hpp"

import std;

namespace Star {

// Reference object that describes the generation of a single world, and all
// of the world metadata.  Meant to remain static (or relatively static)
// throughout the life of a World.
class WorldTemplate {
public:
  struct Dungeon {
    String dungeon;
    int baseHeight;
    int baseX;
    int xVariance;
    bool force;
    bool blendWithTerrain;
  };

  struct BlockInfo {
    BiomeIndex blockBiomeIndex = NullBiomeIndex;
    BiomeIndex environmentBiomeIndex = NullBiomeIndex;

    bool biomeTransition = false;

    bool terrain = false;
    bool foregroundCave = false;
    bool backgroundCave = false;

    MaterialId foreground = EmptyMaterialId;
    ModId foregroundMod = NoModId;

    MaterialId background = EmptyMaterialId;
    ModId backgroundMod = NoModId;

    LiquidId caveLiquid = EmptyLiquidId;
    float caveLiquidSeedDensity = 0.0f;

    LiquidId oceanLiquid = EmptyLiquidId;
    int oceanLiquidLevel = 0;

    bool encloseLiquids = false;
    bool fillMicrodungeons = false;
  };

  struct PotentialBiomeItems {
    // Potential items that would spawn at the given block assuming it is at
    // the surface.
    List<BiomeItemPlacement> surfaceBiomeItems;
    // ... Or on a cave surface.
    List<BiomeItemPlacement> caveSurfaceBiomeItems;
    // ... Or on a cave ceiling.
    List<BiomeItemPlacement> caveCeilingBiomeItems;
    // ... Or on a cave background wall.
    List<BiomeItemPlacement> caveBackgroundBiomeItems;
    // ... Or in the ocean
    List<BiomeItemPlacement> oceanItems;
  };

  // Creates a blank world with the given size
  WorldTemplate(Vec2U const& size);
  // Creates a world from the given visitable celestial object.
  WorldTemplate(CelestialCoordinate const& celestialCoordinate, Ptr<CelestialDatabase> const& celestialDatabase);
  // Creates a world from a bare VisitableWorldParameters structure
  WorldTemplate(ConstPtr<VisitableWorldParameters> const& worldParameters, SkyParameters const& skyParameters, uint64_t seed);
  // Load a world template from the given stored data.
  WorldTemplate(Json const& store);

  auto store() const -> Json;

  auto celestialParameters() const -> std::optional<CelestialParameters> const&;
  auto worldParameters() const -> ConstPtr<VisitableWorldParameters>;
  auto skyParameters() const -> SkyParameters;
  auto worldLayout() const -> Ptr<WorldLayout>;

  void setWorldParameters(Ptr<VisitableWorldParameters> newParameters);
  void setWorldLayout(Ptr<WorldLayout> newLayout);
  void setSkyParameters(SkyParameters newParameters);

  auto worldSeed() const -> uint64_t;
  auto worldName() const -> String;

  auto size() const -> Vec2U;

  // The average (ish) surface level for this world, off of which terrain
  // generators modify the surface height.
  auto surfaceLevel() const -> float;

  // The constant height at which everything below is considered "underground"
  auto undergroundLevel() const -> float;

  // returns true if the world is terrestrial and the specified position is within the
  // planet's surface layer
  auto inSurfaceLayer(Vec2I const& position) const -> bool;

  // If it is specified, searches the player start search region for an
  // acceptable player start area.  The block returned will be an empty block
  // above a terrain block, in a region of free space.  If no such block can be
  // found or the player start search region is not specified, returns nothing.
  auto findSensiblePlayerStart() const -> std::optional<Vec2I>;

  // Add either a solid region hint or a space region hint for the given
  // polygonal region.  Blending size and weighting is configured in the
  // WorldTemplate config file.
  void addCustomTerrainRegion(PolyF poly);
  void addCustomSpaceRegion(PolyF poly);
  void clearCustomTerrains();

  auto previewAddBiomeRegion(Vec2I const& position, int width) -> List<RectI>;
  auto previewExpandBiomeRegion(Vec2I const& position, int newWidth) -> List<RectI>;

  void addBiomeRegion(Vec2I const& position, String const& biomeName, String const& subBlockSelector, int width);
  void expandBiomeRegion(Vec2I const& position, int newWidth);

  auto dungeons() const -> List<Dungeon>;

  // Is this tile block naturally outside the terrain?
  auto isOutside(int x, int y) const -> bool;
  // Is this integral region of blocks outside the terrain?
  auto isOutside(RectI const& region) const -> bool;

  auto blockInfo(int x, int y) const -> BlockInfo;

  // partial blockinfo that doesn't use terrain selectors
  auto blockBiomeInfo(int x, int y) const -> BlockInfo;

  auto blockBiomeIndex(int x, int y) const -> BiomeIndex;
  auto environmentBiomeIndex(int x, int y) const -> BiomeIndex;
  auto biome(BiomeIndex biomeIndex) const -> ConstPtr<Biome>;

  auto blockBiome(int x, int y) const -> ConstPtr<Biome>;
  auto environmentBiome(int x, int y) const -> ConstPtr<Biome>;

  auto biomeMaterial(BiomeIndex biomeIndex, int x, int y) const -> MaterialId;

  // Returns the material and mod hue shift that should be applied to the given
  // material and mod for this biome.
  auto biomeMaterialHueShift(BiomeIndex biomeIndex, MaterialId material) const -> MaterialHue;
  auto biomeModHueShift(BiomeIndex biomeIndex, ModId mod) const -> MaterialHue;

  auto ambientNoises(int x, int y) const -> Ptr<AmbientNoisesDescription>;
  auto musicTrack(int x, int y) const -> Ptr<AmbientNoisesDescription>;

  auto environmentStatusEffects(int x, int y) const -> StringList;
  auto breathable(int x, int y) const -> bool;

  auto weathers() const -> WeatherPool;

  // Return potential items that would spawn at the given block.
  void addPotentialBiomeItems(int x, int y, PotentialBiomeItems& items, List<BiomeItemDistribution> const& distributions, BiomePlacementArea area, std::optional<BiomePlacementMode> mode = {}) const;
  auto potentialBiomeItemsAt(int x, int y) const -> PotentialBiomeItems;

  // Return only the potential items that can spawn at the given block.
  auto validBiomeItems(int x, int y, PotentialBiomeItems potentialBiomeItems) const -> List<BiomeItemPlacement>;

  auto gravity() const -> float;
  auto threatLevel() const -> float;

  // For consistently seeding object generation at this position
  auto seedFor(int x, int y) const -> uint64_t;

private:
  struct CustomTerrainRegion {
    PolyF region;
    RectF regionBounds;
    bool solid;
  };

  WorldTemplate();

  void determineWorldName();

  auto customTerrainWeighting(int x, int y) const -> std::pair<float, float>;

  // Calculates block info and adds to cache
  auto getBlockInfo(uint32_t x, uint32_t y) const -> BlockInfo;

  Json m_templateConfig;
  float m_customTerrainBlendSize;
  float m_customTerrainBlendWeight;

  std::optional<CelestialParameters> m_celestialParameters;
  ConstPtr<VisitableWorldParameters> m_worldParameters;
  SkyParameters m_skyParameters;
  uint64_t m_seed;
  WorldGeometry m_geometry;
  Ptr<WorldLayout> m_layout;
  String m_worldName;

  List<CustomTerrainRegion> m_customTerrainRegions;

  mutable HashLruCache<Vector<uint32_t, 2>, BlockInfo> m_blockCache;
};

}// namespace Star
