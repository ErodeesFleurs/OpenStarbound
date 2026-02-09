#include "StarWorldLayout.hpp"

#include "StarBiomeDatabase.hpp"
#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarParallax.hpp"// IWYU pragma: export
#include "StarRoot.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

WorldRegion::WorldRegion()
    : terrainSelectorIndex(NullTerrainSelectorIndex),
      foregroundCaveSelectorIndex(NullTerrainSelectorIndex),
      backgroundCaveSelectorIndex(NullTerrainSelectorIndex),
      blockBiomeIndex(NullBiomeIndex),
      environmentBiomeIndex(NullBiomeIndex) {}

WorldRegion::WorldRegion(Json const& store) {
  terrainSelectorIndex = store.getUInt("terrainSelectorIndex");
  foregroundCaveSelectorIndex = store.getUInt("foregroundCaveSelectorIndex");
  backgroundCaveSelectorIndex = store.getUInt("backgroundCaveSelectorIndex");

  blockBiomeIndex = store.getUInt("blockBiomeIndex");
  environmentBiomeIndex = store.getUInt("environmentBiomeIndex");

  regionLiquids.caveLiquid = store.getUInt("caveLiquid");
  regionLiquids.caveLiquidSeedDensity = store.getFloat("caveLiquidSeedDensity");

  regionLiquids.oceanLiquid = store.getUInt("oceanLiquid");
  regionLiquids.oceanLiquidLevel = store.getInt("oceanLiquidLevel");

  regionLiquids.encloseLiquids = store.getBool("encloseLiquids");
  regionLiquids.fillMicrodungeons = store.getBool("fillMicrodungeons");

  subBlockSelectorIndexes = transform<List<TerrainSelectorIndex>>(store.getArray("subBlockSelectorIndexes"), std::mem_fn(&Json::toUInt));
  foregroundOreSelectorIndexes = transform<List<TerrainSelectorIndex>>(store.getArray("foregroundOreSelectorIndexes"), std::mem_fn(&Json::toUInt));
  backgroundOreSelectorIndexes = transform<List<TerrainSelectorIndex>>(store.getArray("backgroundOreSelectorIndexes"), std::mem_fn(&Json::toUInt));
}

auto WorldRegion::toJson() const -> Json {
  return JsonObject{
    {"terrainSelectorIndex", terrainSelectorIndex},
    {"foregroundCaveSelectorIndex", foregroundCaveSelectorIndex},
    {"backgroundCaveSelectorIndex", backgroundCaveSelectorIndex},

    {"blockBiomeIndex", blockBiomeIndex},
    {"environmentBiomeIndex", environmentBiomeIndex},

    {"caveLiquid", regionLiquids.caveLiquid},
    {"caveLiquidSeedDensity", regionLiquids.caveLiquidSeedDensity},

    {"oceanLiquid", regionLiquids.oceanLiquid},
    {"oceanLiquidLevel", regionLiquids.oceanLiquidLevel},

    {"encloseLiquids", regionLiquids.encloseLiquids},
    {"fillMicrodungeons", regionLiquids.fillMicrodungeons},

    {"subBlockSelectorIndexes", subBlockSelectorIndexes.transformed(construct<Json>())},
    {"foregroundOreSelectorIndexes", foregroundOreSelectorIndexes.transformed(construct<Json>())},
    {"backgroundOreSelectorIndexes", backgroundOreSelectorIndexes.transformed(construct<Json>())}};
}

auto WorldLayout::BlockNoise::build(Json const& config, std::uint64_t seed) -> WorldLayout::BlockNoise {
  BlockNoise blockNoise;
  blockNoise.horizontalNoise = PerlinF(config.get("horizontalNoise"), staticRandomU64(seed, "HorizontalNoise"));
  blockNoise.verticalNoise = PerlinF(config.get("verticalNoise"), staticRandomU64(seed, "VerticalNoise"));
  blockNoise.xNoise = PerlinF(config.get("noise"), staticRandomU64(seed, "XNoise"));
  blockNoise.yNoise = PerlinF(config.get("noise"), staticRandomU64(seed, "yNoise"));
  return blockNoise;
}

WorldLayout::BlockNoise::BlockNoise() = default;

WorldLayout::BlockNoise::BlockNoise(Json const& store) {
  horizontalNoise = PerlinF(store.get("horizontalNoise"));
  verticalNoise = PerlinF(store.get("verticalNoise"));
  xNoise = PerlinF(store.get("xNoise"));
  yNoise = PerlinF(store.get("yNoise"));
}

auto WorldLayout::BlockNoise::toJson() const -> Json {
  return JsonObject{
    {"horizontalNoise", horizontalNoise.toJson()},
    {"verticalNoise", verticalNoise.toJson()},
    {"xNoise", xNoise.toJson()},
    {"yNoise", yNoise.toJson()},
  };
}

auto WorldLayout::BlockNoise::apply(Vec2I const& input, Vec2U const& worldSize) const -> Vec2I {
  float angle = (input[0] / (float)worldSize[0]) * 2 * Constants::pi;
  float xc = std::sin(angle) / (2 * Constants::pi) * worldSize[0];
  float zc = std::cos(angle) / (2 * Constants::pi) * worldSize[0];

  Vec2I noisePos = Vec2I(
    std::floor(input[0] + horizontalNoise.get(input[1]) + xNoise.get(xc, input[1], zc)),
    std::floor(input[1] + verticalNoise.get(xc, zc) + yNoise.get(xc, input[1], zc)));
  noisePos[1] = clamp<int>(noisePos[1], 0, worldSize[1]);

  return noisePos;
}

auto WorldLayout::buildTerrestrialLayout(TerrestrialWorldParameters const& terrestrialParameters, std::uint64_t seed) -> WorldLayout {
  auto& root = Root::singleton();
  auto assets = root.assets();
  auto terrainDatabase = root.terrainDatabase();
  auto biomeDatabase = root.biomeDatabase();

  RandomSource randSource(seed);

  WorldLayout layout;
  layout.m_worldSize = terrestrialParameters.worldSize;

  auto addLayer = [&](TerrestrialWorldParameters::TerrestrialLayer const& terrestrialLayer) -> void {
    RegionParams primaryRegionParams = {
      .baseHeight = terrestrialLayer.layerBaseHeight,
      .threatLevel = terrestrialParameters.threatLevel,
      .biomeName = terrestrialLayer.primaryRegion.biome,
      .terrainSelector = terrestrialLayer.primaryRegion.blockSelector,
      .fgCaveSelector = terrestrialLayer.primaryRegion.fgCaveSelector,
      .bgCaveSelector = terrestrialLayer.primaryRegion.bgCaveSelector,
      .fgOreSelector = terrestrialLayer.primaryRegion.fgOreSelector,
      .bgOreSelector = terrestrialLayer.primaryRegion.bgOreSelector,
      .subBlockSelector = terrestrialLayer.primaryRegion.subBlockSelector,
      .regionLiquids = {.caveLiquid = terrestrialLayer.primaryRegion.caveLiquid,
                        .caveLiquidSeedDensity = terrestrialLayer.primaryRegion.caveLiquidSeedDensity,
                        .oceanLiquid = terrestrialLayer.primaryRegion.oceanLiquid,
                        .oceanLiquidLevel = terrestrialLayer.primaryRegion.oceanLiquidLevel,
                        .encloseLiquids = terrestrialLayer.primaryRegion.encloseLiquids,
                        .fillMicrodungeons = terrestrialLayer.primaryRegion.fillMicrodungeons}};

    RegionParams primarySubRegionParams = {
      .baseHeight = terrestrialLayer.layerBaseHeight,
      .threatLevel = terrestrialParameters.threatLevel,
      .biomeName = terrestrialLayer.primarySubRegion.biome,
      .terrainSelector = terrestrialLayer.primarySubRegion.blockSelector,
      .fgCaveSelector = terrestrialLayer.primarySubRegion.fgCaveSelector,
      .bgCaveSelector = terrestrialLayer.primarySubRegion.bgCaveSelector,
      .fgOreSelector = terrestrialLayer.primarySubRegion.fgOreSelector,
      .bgOreSelector = terrestrialLayer.primarySubRegion.bgOreSelector,
      .subBlockSelector = terrestrialLayer.primarySubRegion.subBlockSelector,
      .regionLiquids = {.caveLiquid = terrestrialLayer.primarySubRegion.caveLiquid,
                        .caveLiquidSeedDensity = terrestrialLayer.primarySubRegion.caveLiquidSeedDensity,
                        .oceanLiquid = terrestrialLayer.primarySubRegion.oceanLiquid,
                        .oceanLiquidLevel = terrestrialLayer.primarySubRegion.oceanLiquidLevel,
                        .encloseLiquids = terrestrialLayer.primarySubRegion.encloseLiquids,
                        .fillMicrodungeons = terrestrialLayer.primarySubRegion.fillMicrodungeons}};

    List<RegionParams> secondaryRegions;
    for (auto const& secondaryRegion : terrestrialLayer.secondaryRegions) {
      RegionParams secondaryRegionParams = {
        .baseHeight = terrestrialLayer.layerBaseHeight,
        .threatLevel = terrestrialParameters.threatLevel,
        .biomeName = secondaryRegion.biome,
        .terrainSelector = secondaryRegion.blockSelector,
        .fgCaveSelector = secondaryRegion.fgCaveSelector,
        .bgCaveSelector = secondaryRegion.bgCaveSelector,
        .fgOreSelector = secondaryRegion.fgOreSelector,
        .bgOreSelector = secondaryRegion.bgOreSelector,
        .subBlockSelector = secondaryRegion.subBlockSelector,
        .regionLiquids = {.caveLiquid = secondaryRegion.caveLiquid,
                          .caveLiquidSeedDensity = secondaryRegion.caveLiquidSeedDensity,
                          .oceanLiquid = secondaryRegion.oceanLiquid,
                          .oceanLiquidLevel = secondaryRegion.oceanLiquidLevel,
                          .encloseLiquids = secondaryRegion.encloseLiquids,
                          .fillMicrodungeons = secondaryRegion.fillMicrodungeons}};

      secondaryRegions.append(secondaryRegionParams);
    }

    List<RegionParams> secondarySubRegions;
    for (auto const& secondarySubRegion : terrestrialLayer.secondarySubRegions) {
      RegionParams secondarySubRegionParams = {
        .baseHeight = terrestrialLayer.layerBaseHeight,
        .threatLevel = terrestrialParameters.threatLevel,
        .biomeName = secondarySubRegion.biome,
        .terrainSelector = secondarySubRegion.blockSelector,
        .fgCaveSelector = secondarySubRegion.fgCaveSelector,
        .bgCaveSelector = secondarySubRegion.bgCaveSelector,
        .fgOreSelector = secondarySubRegion.fgOreSelector,
        .bgOreSelector = secondarySubRegion.bgOreSelector,
        .subBlockSelector = secondarySubRegion.subBlockSelector,
        .regionLiquids = {.caveLiquid = secondarySubRegion.caveLiquid,
                          .caveLiquidSeedDensity = secondarySubRegion.caveLiquidSeedDensity,
                          .oceanLiquid = secondarySubRegion.oceanLiquid,
                          .oceanLiquidLevel = secondarySubRegion.oceanLiquidLevel,
                          .encloseLiquids = secondarySubRegion.encloseLiquids,
                          .fillMicrodungeons = secondarySubRegion.fillMicrodungeons}};

      secondarySubRegions.append(secondarySubRegionParams);
    }

    layout.addLayer(seed,
                    terrestrialLayer.layerMinHeight,
                    terrestrialLayer.layerBaseHeight,
                    terrestrialParameters.primaryBiome,
                    primaryRegionParams,
                    primarySubRegionParams,
                    secondaryRegions,
                    secondarySubRegions,
                    terrestrialLayer.secondaryRegionSizeRange,
                    terrestrialLayer.subRegionSizeRange);
  };

  addLayer(terrestrialParameters.coreLayer);
  for (auto const& undergroundLayer : reverseIterate(terrestrialParameters.undergroundLayers))
    addLayer(undergroundLayer);

  addLayer(terrestrialParameters.subsurfaceLayer);
  addLayer(terrestrialParameters.surfaceLayer);
  addLayer(terrestrialParameters.atmosphereLayer);
  addLayer(terrestrialParameters.spaceLayer);

  layout.m_regionBlending = terrestrialParameters.blendSize;
  if (terrestrialParameters.blockNoiseConfig)
    layout.m_blockNoise = BlockNoise::build(terrestrialParameters.blockNoiseConfig, seed);
  if (terrestrialParameters.blendNoiseConfig)
    layout.m_blendNoise = PerlinF(terrestrialParameters.blendNoiseConfig, staticRandomU64(seed, "BlendNoise"));

  layout.finalize(terrestrialParameters.skyColoring.mainColor);

  return layout;
}

auto WorldLayout::buildAsteroidsLayout(AsteroidsWorldParameters const& asteroidParameters, std::uint64_t seed) -> WorldLayout {
  auto assets = Root::singleton().assets();

  RandomSource randSource(seed);

  auto asteroidsConfig = assets->json("/asteroids_worlds.config");
  auto asteroidTerrainConfig = randSource.randFrom(asteroidsConfig.get("terrains").toArray());
  auto emptyTerrainConfig = asteroidsConfig.get("emptyTerrain");

  WorldLayout layout;
  layout.m_worldSize = asteroidParameters.worldSize;

  RegionParams asteroidRegion{
    .baseHeight = (int)asteroidParameters.worldSize[1] / 2,
    .threatLevel = asteroidParameters.threatLevel,
    .biomeName = asteroidParameters.asteroidBiome,
    .terrainSelector = asteroidTerrainConfig.getString("terrainSelector"),
    .fgCaveSelector = asteroidTerrainConfig.getString("caveSelector"),
    .bgCaveSelector = asteroidTerrainConfig.getString("bgCaveSelector"),
    .fgOreSelector = asteroidTerrainConfig.getString("oreSelector"),
    .bgOreSelector = asteroidTerrainConfig.getString("oreSelector"),
    .subBlockSelector = asteroidTerrainConfig.getString("subBlockSelector"),
    .regionLiquids = {.caveLiquid = EmptyLiquidId, .caveLiquidSeedDensity = 0.0f, .oceanLiquid = EmptyLiquidId, .oceanLiquidLevel = 0, .encloseLiquids = false, .fillMicrodungeons = false}};

  RegionParams emptyRegion{
    .baseHeight = (int)asteroidParameters.worldSize[1] / 2,
    .threatLevel = asteroidParameters.threatLevel,
    .biomeName = asteroidParameters.asteroidBiome,
    .terrainSelector = emptyTerrainConfig.getString("terrainSelector"),
    .fgCaveSelector = emptyTerrainConfig.getString("caveSelector"),
    .bgCaveSelector = emptyTerrainConfig.getString("bgCaveSelector"),
    .fgOreSelector = emptyTerrainConfig.getString("oreSelector"),
    .bgOreSelector = emptyTerrainConfig.getString("oreSelector"),
    .subBlockSelector = emptyTerrainConfig.getString("subBlockSelector"),
    .regionLiquids = {.caveLiquid = EmptyLiquidId, .caveLiquidSeedDensity = 0.0f, .oceanLiquid = EmptyLiquidId, .oceanLiquidLevel = 0, .encloseLiquids = false, .fillMicrodungeons = false}};

  layout.addLayer(seed, 0, emptyRegion);
  layout.addLayer(seed, asteroidParameters.asteroidBottomLevel, asteroidRegion);
  layout.addLayer(seed, asteroidParameters.asteroidTopLevel, emptyRegion);

  layout.m_regionBlending = asteroidParameters.blendSize;
  layout.m_blockNoise = asteroidsConfig.opt("blockNoise").transform([seed](auto&& PH1) -> auto { return BlockNoise::build(std::forward<decltype(PH1)>(PH1), seed); });

  layout.m_playerStartSearchRegions.append(RectI(0, asteroidParameters.asteroidBottomLevel, asteroidParameters.worldSize[0], asteroidParameters.asteroidTopLevel));

  layout.finalize(Color::Black);

  return layout;
}

auto WorldLayout::buildFloatingDungeonLayout(FloatingDungeonWorldParameters const& floatingDungeonParameters, std::uint64_t seed) -> WorldLayout {
  auto assets = Root::singleton().assets();
  ConstPtr<BiomeDatabase> biomeDatabase = Root::singleton().biomeDatabase();

  RandomSource randSource(seed);

  WorldLayout layout;
  layout.m_worldSize = floatingDungeonParameters.worldSize;

  RegionParams biomeRegion{
    .baseHeight = (int)floatingDungeonParameters.dungeonSurfaceHeight,
    .threatLevel = floatingDungeonParameters.threatLevel,
    .biomeName = floatingDungeonParameters.biome,
    .terrainSelector = {},
    .fgCaveSelector = {},
    .bgCaveSelector = {},
    .fgOreSelector = {},
    .bgOreSelector = {},
    .subBlockSelector = {},
    .regionLiquids = {.caveLiquid = EmptyLiquidId, .caveLiquidSeedDensity = 0.0f, .oceanLiquid = EmptyLiquidId, .oceanLiquidLevel = 0, .encloseLiquids = false, .fillMicrodungeons = false}};

  layout.addLayer(seed, 0, biomeRegion);
  if (floatingDungeonParameters.biome)
    auto _ = biomeDatabase->biomeSkyColoring(*floatingDungeonParameters.biome, seed);
  else
    layout.finalize(Color::Black);

  return layout;
}

WorldLayout::WorldLayout() : m_regionBlending(0.0f) {}

WorldLayout::WorldLayout(Json const& store) : WorldLayout() {
  auto terrainDatabase = Root::singleton().terrainDatabase();

  m_worldSize = jsonToVec2U(store.get("worldSize"));

  m_biomes = store.getArray("biomes").transformed([](Json const& json) -> ConstPtr<Biome> {
    return {std::make_shared<Biome>(json)};
  });

  m_terrainSelectors = store.getArray("terrainSelectors").transformed([terrainDatabase](Json const& v) -> ConstPtr<TerrainSelector> {
    return ConstPtr<TerrainSelector>(terrainDatabase->loadSelector(v));
  });

  m_layers = store.getArray("layers").transformed([](Json const& l) -> WorldLayer {
    WorldLayer layer;
    layer.yStart = l.getInt("yStart");

    for (auto const& b : l.getArray("boundaries"))
      layer.boundaries.append(b.toInt());

    for (auto const& r : l.getArray("cells"))
      layer.cells.append(std::make_shared<WorldRegion>(r));

    return layer;
  });

  m_regionBlending = store.getFloat("regionBlending");
  m_blockNoise = store.opt("blockNoise").transform(construct<BlockNoise>());
  m_blendNoise = store.opt("blendNoise").transform(construct<PerlinF>());

  m_playerStartSearchRegions = store.getArray("playerStartSearchRegions").transformed(jsonToRectI);
}

auto WorldLayout::toJson() const -> Json {
  auto terrainDatabase = Root::singleton().terrainDatabase();

  return JsonObject{
    {"worldSize", jsonFromVec2U(m_worldSize)},
    {"biomes", transform<JsonArray>(m_biomes, [](auto const& biome) -> auto {
       return biome->toJson();
     })},
    {"terrainSelectors", transform<JsonArray>(m_terrainSelectors, [terrainDatabase](auto const& selector) -> auto {
       return terrainDatabase->storeSelector(selector);
     })},
    {"layers", m_layers.transformed([](WorldLayer const& layer) -> Json {
       return JsonObject{
         {"yStart", layer.yStart},
         {"boundaries", JsonArray::from(layer.boundaries.transformed(construct<Json>()))},
         {"cells", JsonArray::from(layer.cells.transformed(std::mem_fn(&WorldRegion::toJson)))}};
     })},
    {"regionBlending", m_regionBlending},
    {"blockNoise", m_blockNoise.transform(std::mem_fn(&BlockNoise::toJson)).value()},
    {"blendNoise", m_blendNoise.transform(std::mem_fn(&PerlinF::toJson)).value()},
    {"playerStartSearchRegions", JsonArray::from(m_playerStartSearchRegions.transformed(jsonFromRectI))}};
}

auto WorldLayout::blockNoise() const -> std::optional<WorldLayout::BlockNoise> const& {
  return m_blockNoise;
}

auto WorldLayout::blendNoise() const -> std::optional<PerlinF> const& {
  return m_blendNoise;
}

auto WorldLayout::playerStartSearchRegions() const -> List<RectI> {
  return m_playerStartSearchRegions;
}

auto WorldLayout::getWeighting(int x, int y) const -> List<WorldLayout::RegionWeighting> {
  List<RegionWeighting> weighting;
  WorldGeometry geometry(m_worldSize);

  auto cellWeighting = [&](WorldLayer const& layer, size_t cellIndex, int x) -> float {
    int xMin = 0;
    if (cellIndex > 0)
      xMin = layer.boundaries[cellIndex - 1];

    int xMax = m_worldSize[0];
    if (cellIndex < layer.boundaries.size())
      xMax = layer.boundaries[cellIndex];

    if (x > (xMin + xMax) / 2.0f)
      return clamp<float>(0.5f - (x - xMax) / m_regionBlending, 0.0f, 1.0f);
    else
      return clamp<float>(0.5f - (xMin - x) / m_regionBlending, 0.0f, 1.0f);
  };

  auto addLayerWeighting = [&](WorldLayer const& layer, int x, float weightFactor) -> void {
    if (layer.cells.empty())
      return;

    size_t innerCellIndex;
    int innerCellXValue;
    std::tie(innerCellIndex, innerCellXValue) = findContainingCell(layer, x);
    float innerCellWeight = cellWeighting(layer, innerCellIndex, innerCellXValue);

    size_t leftCellIndex;
    int leftCellXValue;
    std::tie(leftCellIndex, leftCellXValue) = leftCell(layer, innerCellIndex, innerCellXValue);
    float leftCellWeight = cellWeighting(layer, leftCellIndex, leftCellXValue);

    size_t rightCellIndex;
    int rightCellXValue;
    std::tie(rightCellIndex, rightCellXValue) = rightCell(layer, innerCellIndex, innerCellXValue);
    float rightCellWeight = cellWeighting(layer, rightCellIndex, rightCellXValue);

    float totalWeight = innerCellWeight + leftCellWeight + rightCellWeight;
    if (totalWeight <= 0.0f)
      return;

    innerCellWeight *= weightFactor / totalWeight;
    leftCellWeight *= weightFactor / totalWeight;
    rightCellWeight *= weightFactor / totalWeight;

    if (innerCellWeight > 0.0f)
      weighting.append(RegionWeighting{.weight = innerCellWeight, .xValue = innerCellXValue, .region = layer.cells[innerCellIndex].get()});

    if (leftCellWeight > 0.0f)
      weighting.append(RegionWeighting{.weight = leftCellWeight, .xValue = leftCellXValue, .region = layer.cells[leftCellIndex].get()});

    if (rightCellWeight > 0.0f)
      weighting.append(RegionWeighting{.weight = rightCellWeight, .xValue = rightCellXValue, .region = layer.cells[rightCellIndex].get()});
  };

  auto yi = std::ranges::lower_bound(m_layers, y, std::less{}, &WorldLayer::yStart);

  if (yi == m_layers.end() || yi->yStart != y) {
    if (yi == m_layers.begin())
      return {};
    else
      --yi;
  }

  if (y - yi->yStart < (m_regionBlending / 2)) {
    if (yi == m_layers.begin()) {
      addLayerWeighting(*yi, x, 1.0f);
    } else {
      auto ypi = yi;
      --ypi;

      float yWeight = 0.5f + (y - yi->yStart) / m_regionBlending;
      addLayerWeighting(*yi, x, yWeight);
      addLayerWeighting(*ypi, x, 1.0f - yWeight);
    }
  } else {
    auto yni = yi;
    ++yni;
    if (yni == m_layers.end()) {
      addLayerWeighting(*yi, x, 1.0f);
    } else if (y <= yni->yStart - (m_regionBlending / 2)) {
      addLayerWeighting(*yi, x, 1.0f);
    } else {
      float yWeight = 0.5f - (yni->yStart - y) / m_regionBlending;
      addLayerWeighting(*yi, x, 1.0f - yWeight);
      addLayerWeighting(*yni, x, yWeight);
    }
  }

  // Need to return weighting in order of greatest to least
  sort(weighting, [](RegionWeighting const& lhs, RegionWeighting const& rhs) -> bool {
    return lhs.weight > rhs.weight;
  });

  return weighting;
}

auto WorldLayout::previewAddBiomeRegion(Vec2I const& position, int width) const -> List<RectI> {
  auto layerAndCell = findLayerAndCell(position[0], position[1]);
  auto targetLayer = m_layers[layerAndCell.first];
  auto targetRegion = targetLayer.cells[layerAndCell.second];

  int insertX = position[0] > 0 ? position[0] : 1;

  // need a dummy region to expand
  std::shared_ptr<WorldRegion> dummyRegion;

  targetLayer.boundaries.insertAt(layerAndCell.second, insertX);
  targetLayer.cells.insertAt(layerAndCell.second, dummyRegion);

  targetLayer.boundaries.insertAt(layerAndCell.second, insertX - 1);
  targetLayer.cells.insertAt(layerAndCell.second, targetRegion);

  auto expandResult = expandRegionInLayer(targetLayer, layerAndCell.second + 1, width);

  return expandResult.second;
}

auto WorldLayout::previewExpandBiomeRegion(Vec2I const& position, int width) const -> List<RectI> {
  auto layerAndCell = findLayerAndCell(position[0], position[1]);
  auto targetLayer = m_layers[layerAndCell.first];

  auto expandResult = expandRegionInLayer(targetLayer, layerAndCell.second, width);

  return expandResult.second;
}

auto WorldLayout::setLayerEnvironmentBiome(Vec2I const& position) -> String {
  auto layerAndCell = findLayerAndCell(position[0], position[1]);
  auto targetLayer = m_layers[layerAndCell.first];
  auto targetBiomeIndex = targetLayer.cells[layerAndCell.second]->blockBiomeIndex;

  for (const auto& cell : targetLayer.cells)
    cell->environmentBiomeIndex = targetBiomeIndex;

  m_layers[layerAndCell.first] = targetLayer;

  return getBiome(targetBiomeIndex)->baseName;
}

void WorldLayout::addBiomeRegion(
  TerrestrialWorldParameters const& terrestrialParameters,
  std::uint64_t seed,
  Vec2I const& position,
  String biomeName,
  String const& subBlockSelector,
  int width) {

  auto layerAndCell = findLayerAndCell(position[0], position[1]);

  // Logger::info("inserting biome {} into region with layerIndex {} cellIndex {}", biomeName, layerAndCell.first, layerAndCell.second);

  auto targetLayer = m_layers[layerAndCell.first];

  // do this annoying dance to figure out which terrestrial layer we're in, so
  // we can extract the base height
  TerrestrialWorldParameters::TerrestrialLayer terrestrialLayer = terrestrialParameters.coreLayer;
  auto checkLayer = [targetLayer, &terrestrialLayer](TerrestrialWorldParameters::TerrestrialLayer const& layer) -> void {
    if (layer.layerMinHeight == targetLayer.yStart)
      terrestrialLayer = layer;
  };
  for (auto undergroundLayer : terrestrialParameters.undergroundLayers)
    checkLayer(undergroundLayer);
  checkLayer(terrestrialParameters.subsurfaceLayer);
  checkLayer(terrestrialParameters.surfaceLayer);
  checkLayer(terrestrialParameters.atmosphereLayer);
  checkLayer(terrestrialParameters.spaceLayer);

  // build a new region using the biomeName and the parameters from the target region
  auto targetRegion = targetLayer.cells[layerAndCell.second];

  WorldRegion newRegion;
  newRegion.terrainSelectorIndex = targetRegion->terrainSelectorIndex;
  newRegion.foregroundCaveSelectorIndex = targetRegion->foregroundCaveSelectorIndex;
  newRegion.backgroundCaveSelectorIndex = targetRegion->backgroundCaveSelectorIndex;
  newRegion.foregroundOreSelectorIndexes = targetRegion->foregroundOreSelectorIndexes;
  newRegion.backgroundOreSelectorIndexes = targetRegion->backgroundOreSelectorIndexes;
  newRegion.regionLiquids = targetRegion->regionLiquids;

  auto biomeDatabase = Root::singleton().biomeDatabase();

  auto newBiome = biomeDatabase->createBiome(biomeName, staticRandomU64(seed, "BiomeSeed"), terrestrialLayer.layerBaseHeight, terrestrialParameters.threatLevel);

  auto oldBiome = getBiome(targetRegion->blockBiomeIndex);

  newBiome->ores = oldBiome->ores;

  // build new sub block selectors; this is the only region-level property that needs to be
  // newly constructed for the biome

  TerrainSelectorParameters baseSelectorParameters;
  baseSelectorParameters.worldWidth = m_worldSize[0];
  baseSelectorParameters.baseHeight = terrestrialLayer.layerBaseHeight;

  auto terrainDatabase = Root::singleton().terrainDatabase();
  for (size_t i = 0; i < newBiome->subBlocks.size(); ++i) {
    auto selector = terrainDatabase->createNamedSelector(subBlockSelector, baseSelectorParameters.withSeed(staticRandomU64(seed, i, "subBlocks")));
    newRegion.subBlockSelectorIndexes.append(registerTerrainSelector(selector));
  }

  newRegion.environmentBiomeIndex = targetRegion->environmentBiomeIndex;
  newRegion.blockBiomeIndex = registerBiome(newBiome);

  Ptr<WorldRegion> newRegionPtr = std::make_shared<WorldRegion>(newRegion);

  // Logger::info("boundaries before region insertion are {}", targetLayer.boundaries);

  // handle case where insert x position is exactly at world wrap
  int insertX = position[0] > 0 ? position[0] : 1;

  // insert the new region boundary
  targetLayer.boundaries.insertAt(layerAndCell.second, insertX);
  targetLayer.cells.insertAt(layerAndCell.second, newRegionPtr);

  // insert the left side of the (now split) target region
  targetLayer.boundaries.insertAt(layerAndCell.second, insertX - 1);
  targetLayer.cells.insertAt(layerAndCell.second, targetRegion);

  // Logger::info("boundaries after region insertion are {}", targetLayer.boundaries);

  // expand the cell to the desired size
  auto expandResult = expandRegionInLayer(targetLayer, layerAndCell.second + 1, width);

  // update the layer in the template
  m_layers[layerAndCell.first] = expandResult.first;
}

void WorldLayout::expandBiomeRegion(Vec2I const& position, int newWidth) {
  auto layerAndCell = findLayerAndCell(position[0], position[1]);

  auto targetLayer = m_layers[layerAndCell.first];

  auto expandResult = expandRegionInLayer(targetLayer, layerAndCell.second, newWidth);

  m_layers[layerAndCell.first] = expandResult.first;
}

auto WorldLayout::findLayerAndCell(int x, int y) const -> std::pair<size_t, size_t> {
  // find the target layer
  size_t targetLayerIndex{};
  for (size_t i = 0; i < m_layers.size(); ++i) {
    if (m_layers[i].yStart < y)
      targetLayerIndex = i;
    else
      break;
  }

  auto targetLayer = m_layers[targetLayerIndex];

  auto targetCell = findContainingCell(targetLayer, x);

  return {targetLayerIndex, targetCell.first};
}

WorldLayout::WorldLayer::WorldLayer() : yStart(0) {}

auto WorldLayout::expandRegionInLayer(WorldLayout::WorldLayer targetLayer, size_t cellIndex, int newWidth) const -> std::pair<WorldLayout::WorldLayer, List<RectI>> {
  struct RegionCell {
    int lBound;
    int rBound;
    Ptr<WorldRegion> region;
  };

  // auto printRegionCells = [](List<RegionCell> const& cells) {
  //   String output = "";
  //   for (auto cell : cells)
  //     output += strf("[{} {}]  ", cell.lBound, cell.rBound);
  //   return output;
  // };

  // Logger::info("expanding region in layer with cellIndex {} newWidth {}", cellIndex, newWidth);

  List<RectI> regionRects;

  if (targetLayer.cells.size() == 1) {
    Logger::info("Cannot expand region as it already fills the layer");
    return {targetLayer, regionRects};
  }

  // Logger::info("boundaries before expansion are {}", targetLayer.boundaries);

  // TODO: this is a messy way to get the top of the layer, but maybe it's ok
  int layerTop = (int)m_worldSize[1];
  for (size_t i = 0; i < m_layers.size(); ++i) {
    if (m_layers[i].yStart == targetLayer.yStart && m_layers.size() > i + 1) {
      layerTop = m_layers[i + 1].yStart;
      break;
    }
  }

  // if the region is going to cover the full layer width, this is much simpler
  if (newWidth == (int)m_worldSize[0]) {
    targetLayer.cells = {targetLayer.cells[cellIndex]};
    targetLayer.boundaries = {};

    regionRects.append(RectI{0, targetLayer.yStart, (int)m_worldSize[0], layerTop});
  } else {
    auto targetRegion = targetLayer.cells[cellIndex];

    // convert cells and boundaries into something more tractable
    List<RegionCell> targetCells;
    List<RegionCell> otherCells;

    int lastBoundary = 0;
    size_t lastCellIndex = targetLayer.cells.size() - 1;
    for (size_t i = 0; i <= lastCellIndex; ++i) {
      int nextBoundary = i == lastCellIndex ? (int)m_worldSize[0] : targetLayer.boundaries[i];
      if (i == cellIndex || (i == 0 && cellIndex == lastCellIndex && targetLayer.cells[i] == targetRegion) || (cellIndex == 0 && i == lastCellIndex && targetLayer.cells[i] == targetRegion))

        targetCells.append(RegionCell{.lBound = lastBoundary, .rBound = nextBoundary, .region = targetLayer.cells[i]});
      else
        otherCells.append(RegionCell{.lBound = lastBoundary, .rBound = nextBoundary, .region = targetLayer.cells[i]});
      lastBoundary = nextBoundary;
    }

    // Logger::info("before expansion:\ntarget cells are: {}\nother cells are: {}", printRegionCells(targetCells), printRegionCells(otherCells));

    // check the current width to see how much (if any) to expand
    int currentWidth = 0;
    for (auto regionCell : targetCells)
      currentWidth += (regionCell.rBound - regionCell.lBound);

    if (currentWidth >= newWidth) {
      Logger::info("New cell width ({}) must be greater than current cell width {}!", newWidth, currentWidth);
      return {targetLayer, regionRects};
    }

    // expand the leftmost cell to the right and the rightmost cell to the left (they may be the same cell)
    int expandRight = std::ceil(0.5 * (newWidth - currentWidth));
    int expandLeft = std::floor(0.5 * (newWidth - currentWidth));

    // build the rects for the areas NEWLY covered by the region; these don't need to be wrapped because
    // they'll be split when they're consumed
    regionRects.append(RectI{targetCells[0].rBound, targetLayer.yStart, targetCells[0].rBound + expandRight, layerTop});
    regionRects.append(RectI{targetCells[targetCells.size() - 1].lBound - expandLeft, targetLayer.yStart, targetCells[targetCells.size() - 1].lBound, layerTop});

    targetCells[0].rBound += expandRight;
    targetCells[targetCells.size() - 1].lBound -= expandLeft;

    // Logger::info("after expansion:\ntarget cells are: {}\nother cells are:  {}", printRegionCells(targetCells), printRegionCells(otherCells));

    // split any target cells that now cross the world wrap
    List<RegionCell> wrappedTargetCells;
    for (auto cell : targetCells) {
      if (cell.lBound < 0) {
        wrappedTargetCells.append(RegionCell{.lBound = 0, .rBound = cell.rBound, .region = cell.region});
        wrappedTargetCells.append(RegionCell{.lBound = (int)m_worldSize[0] + cell.lBound, .rBound = (int)m_worldSize[0], .region = cell.region});
      } else if (cell.rBound > (int)m_worldSize[0]) {
        wrappedTargetCells.append(RegionCell{.lBound = cell.lBound, .rBound = (int)m_worldSize[0], .region = cell.region});
        wrappedTargetCells.append(RegionCell{.lBound = 0, .rBound = cell.rBound - (int)m_worldSize[0], .region = cell.region});
      } else {
        wrappedTargetCells.append(cell);
      }
    }

    targetCells = wrappedTargetCells;

    // modify/delete any overlapped cells
    for (auto targetCell : targetCells) {
      List<RegionCell> newOtherCells;
      for (auto otherCell : otherCells) {
        bool rInside = otherCell.rBound <= targetCell.rBound && otherCell.rBound >= targetCell.lBound;
        bool lInside = otherCell.lBound <= targetCell.rBound && otherCell.lBound >= targetCell.lBound;
        if (rInside && lInside)
          continue;
        else if (rInside)
          newOtherCells.append(RegionCell{.lBound = otherCell.lBound, .rBound = targetCell.lBound, .region = otherCell.region});
        else if (lInside)
          newOtherCells.append(RegionCell{.lBound = targetCell.rBound, .rBound = otherCell.rBound, .region = otherCell.region});
        else
          newOtherCells.append(otherCell);
      }
      otherCells = newOtherCells;
    }

    // Logger::info("after de-overlapping:\ntarget cells are: {}\nother cells are:  {}", printRegionCells(targetCells), printRegionCells(otherCells));

    // combine lists and sort
    otherCells.appendAll(targetCells);
    otherCells.sort([](RegionCell const& a, RegionCell const& b) -> bool { return a.rBound < b.rBound; });

    // convert back into cells and boundaries
    targetLayer.cells.clear();
    targetLayer.boundaries.clear();
    for (size_t i = 0; i < otherCells.size(); ++i) {
      targetLayer.cells.append(otherCells[i].region);
      if (i < otherCells.size() - 1)
        targetLayer.boundaries.append(otherCells[i].rBound);
    }
  }

  // Logger::info("boundaries after expansion are {}", targetLayer.boundaries);

  return {targetLayer, regionRects};
}

auto WorldLayout::registerBiome(ConstPtr<Biome> biome) -> BiomeIndex {
  size_t foundIndex = m_biomes.indexOf(biome);
  if (foundIndex != std::numeric_limits<std::size_t>::max())
    return foundIndex + 1;

  m_biomes.append(biome);
  return m_biomes.size();
}

auto WorldLayout::registerTerrainSelector(ConstPtr<TerrainSelector> terrainSelector) -> TerrainSelectorIndex {
  size_t foundIndex = m_terrainSelectors.indexOf(terrainSelector);
  if (foundIndex != std::numeric_limits<std::size_t>::max())
    return foundIndex + 1;

  m_terrainSelectors.append(terrainSelector);
  return m_terrainSelectors.size();
}

auto WorldLayout::buildRegion(std::uint64_t seed, RegionParams const& regionParams) -> WorldRegion {
  auto terrainDatabase = Root::singleton().terrainDatabase();
  auto biomeDatabase = Root::singleton().biomeDatabase();

  WorldRegion region;

  TerrainSelectorParameters baseSelectorParameters;
  baseSelectorParameters.worldWidth = m_worldSize[0];
  baseSelectorParameters.baseHeight = regionParams.baseHeight;

  TerrainSelectorParameters terrainSelectorParameters = baseSelectorParameters.withSeed(staticRandomU64(seed, "Terrain"));
  TerrainSelectorParameters foregroundCaveSelectorParameters = baseSelectorParameters.withSeed(staticRandomU64(seed, "ForegroundCaveSeed"));
  TerrainSelectorParameters backgroundCaveSelectorParameters = baseSelectorParameters.withSeed(staticRandomU64(seed, "BackgroundCave"));

  if (regionParams.terrainSelector)
    region.terrainSelectorIndex = registerTerrainSelector(terrainDatabase->createNamedSelector(*regionParams.terrainSelector, terrainSelectorParameters));
  if (regionParams.fgCaveSelector)
    region.foregroundCaveSelectorIndex = registerTerrainSelector(terrainDatabase->createNamedSelector(*regionParams.fgCaveSelector, foregroundCaveSelectorParameters));
  if (regionParams.bgCaveSelector)
    region.backgroundCaveSelectorIndex = registerTerrainSelector(terrainDatabase->createNamedSelector(*regionParams.bgCaveSelector, backgroundCaveSelectorParameters));

  if (regionParams.biomeName) {
    auto biome = biomeDatabase->createBiome(*regionParams.biomeName, staticRandomU64(seed, "BiomeSeed"), regionParams.baseHeight, regionParams.threatLevel);

    if (regionParams.subBlockSelector) {
      for (size_t i = 0; i < biome->subBlocks.size(); ++i) {
        auto selector = terrainDatabase->createNamedSelector(*regionParams.subBlockSelector, terrainSelectorParameters.withSeed(staticRandomU64(seed, i, "subBlocks")));
        region.subBlockSelectorIndexes.append(registerTerrainSelector(selector));
      }
    }

    for (auto const& p : enumerateIterator(biome->ores)) {
      auto oreSelectorTerrainParameters = terrainSelectorParameters.withCommonality(p.first.second);

      if (regionParams.fgOreSelector) {
        auto fgSelector = terrainDatabase->createNamedSelector(*regionParams.fgOreSelector, oreSelectorTerrainParameters.withSeed(staticRandomU64(seed, p.second, "FGOreSelector")));
        region.foregroundOreSelectorIndexes.append(registerTerrainSelector(fgSelector));
      }

      if (regionParams.bgOreSelector) {
        auto bgSelector = terrainDatabase->createNamedSelector(*regionParams.bgOreSelector, oreSelectorTerrainParameters.withSeed(staticRandomU64(seed, p.second, "BGOreSelector")));
        region.backgroundOreSelectorIndexes.append(registerTerrainSelector(bgSelector));
      }
    }

    region.blockBiomeIndex = registerBiome(biome);
    region.environmentBiomeIndex = region.blockBiomeIndex;
  }

  region.regionLiquids = regionParams.regionLiquids;

  return region;
}

void WorldLayout::addLayer(std::uint64_t seed, int yStart, RegionParams regionParams) {
  WorldLayer layer;
  layer.yStart = yStart;

  Ptr<WorldRegion> region = std::make_shared<WorldRegion>(buildRegion(seed, regionParams));
  layer.cells.append(region);

  m_layers.append(layer);
}

void WorldLayout::addLayer(std::uint64_t seed, int yStart, int yBase, String const& primaryBiome,
                           RegionParams primaryRegionParams, RegionParams primarySubRegionParams,
                           List<RegionParams> secondaryRegions, List<RegionParams> secondarySubRegions,
                           Vec2F secondaryRegionSize, Vec2F subRegionSize) {
  WorldLayer layer;
  layer.yStart = yStart;

  List<float> relativeRegionSizes;
  float totalRelativeSize = 0.0;
  int mix = 0;

  BiomeIndex primaryEnvironmentBiomeIndex = buildRegion(seed, primaryRegionParams).environmentBiomeIndex;

  Set<BiomeIndex> spawnBiomeIndexes;

  auto addRegion = [&](RegionParams const& regionParams, RegionParams const& subRegionParams, Vec2F const& regionSizeRange) -> void {
    Ptr<WorldRegion> region = std::make_shared<WorldRegion>(buildRegion(seed, regionParams));
    Ptr<WorldRegion> subRegion = std::make_shared<WorldRegion>(buildRegion(seed, subRegionParams));
    if (!Root::singleton().assets()->json("/terrestrial_worlds.config:useSecondaryEnvironmentBiomeIndex").toBool())
      region->environmentBiomeIndex = primaryEnvironmentBiomeIndex;
    subRegion->environmentBiomeIndex = region->environmentBiomeIndex;

    if (regionParams.biomeName == primaryBiome)
      spawnBiomeIndexes.add(region->blockBiomeIndex);
    if (subRegionParams.biomeName == primaryBiome)
      spawnBiomeIndexes.add(subRegion->blockBiomeIndex);

    layer.cells.append(region);
    layer.cells.append(subRegion);
    layer.cells.append(region);

    float regionRelativeSize = staticRandomFloatRange(regionSizeRange[0], regionSizeRange[1], seed, ++mix, yStart);
    float subRegionRelativeSize = staticRandomFloatRange(subRegionSize[0], subRegionSize[1], seed, ++mix, yStart);
    totalRelativeSize += regionRelativeSize;

    if (subRegionRelativeSize >= 1.0f)
      throw StarException("Relative size of subRegion must be less than 1.0!");

    subRegionRelativeSize *= regionRelativeSize;
    regionRelativeSize -= subRegionRelativeSize;

    relativeRegionSizes.append(regionRelativeSize / 2);
    relativeRegionSizes.append(subRegionRelativeSize);
    relativeRegionSizes.append(regionRelativeSize / 2);
  };

  // construct list of region cells and relative sizes
  addRegion(primaryRegionParams, primarySubRegionParams, Vec2F(1, 1));
  for (size_t i = 0; i < secondaryRegions.size(); ++i)
    addRegion(secondaryRegions[i], secondarySubRegions[i], secondaryRegionSize);

  // construct boundaries based on normalized sizes
  int nextBoundary = staticRandomI32Range(0, m_worldSize[0] - 1, seed, yStart, "LayerOffset");
  layer.boundaries.append(nextBoundary);
  for (size_t i = 0; i + 1 < relativeRegionSizes.size(); ++i) {
    int regionSize = (int)m_worldSize[0] * (relativeRegionSizes[i] / totalRelativeSize);
    nextBoundary += regionSize;
    layer.boundaries.append(nextBoundary);
  }

  // wrap cells + boundaries
  while (layer.boundaries.last() > (int)m_worldSize[0]) {
    layer.cells.prepend(layer.cells.takeLast());
    layer.boundaries.prepend(layer.boundaries.takeLast() - m_worldSize[0]);
  }
  layer.cells.prepend(layer.cells.last());

  int yRange = Root::singleton().assets()->json("/world_template.config:playerStartSearchYRange").toInt();
  int i = 0;
  int lastBoundary = 0;
  for (auto region : layer.cells) {
    int nextBoundary = i < (int)layer.boundaries.size() ? layer.boundaries[i] : m_worldSize[0];
    if (spawnBiomeIndexes.contains(region->blockBiomeIndex))
      m_playerStartSearchRegions.append(RectI(lastBoundary, std::max(0, yBase - yRange), nextBoundary, std::min((int)m_worldSize[1], yBase + yRange)));
    lastBoundary = nextBoundary;
    i++;
  }

  m_layers.append(layer);
}

void WorldLayout::finalize(Color mainSkyColor) {
  sort(m_layers, [](WorldLayer const& a, WorldLayer const& b) -> bool { return a.yStart < b.yStart; });

  // Post-process all parallaxes
  for (auto const& biome : m_biomes) {
    if (biome->parallax)
      biome->parallax->fadeToSkyColor(mainSkyColor);
  }
}

auto WorldLayout::findContainingCell(WorldLayer const& layer, int x) const -> std::pair<size_t, int> {
  x = WorldGeometry(m_worldSize).xwrap(x);
  auto xi = std::ranges::lower_bound(layer.boundaries, x);
  return {xi - layer.boundaries.begin(), x};
}

auto WorldLayout::leftCell(WorldLayer const& layer, size_t cellIndex, int x) const -> std::pair<size_t, int> {
  if (cellIndex == 0)
    return {layer.cells.size() - 1, x + (int)m_worldSize[0]};
  else
    return {cellIndex - 1, x};
}

auto WorldLayout::rightCell(WorldLayer const& layer, size_t cellIndex, int x) const -> std::pair<size_t, int> {
  if (cellIndex >= layer.cells.size() - 1)
    return {0, x - (int)m_worldSize[0]};
  else
    return {cellIndex + 1, x};
}

}// namespace Star
