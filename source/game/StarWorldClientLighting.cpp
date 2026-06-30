#include "StarWorldClientLighting.hpp"
#include "StarWorldClient.hpp"
#include "StarWorldImpl.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarLogging.hpp"

namespace Star {

StarWorldClientLighting::StarWorldClientLighting(WorldClient* worldClient)
  : m_worldClient(worldClient) {}

bool StarWorldClientLighting::fullBright() const {
  return m_fullBright;
}

void StarWorldClientLighting::setFullBright(bool fullBright) {
  m_fullBright = fullBright;
}

bool StarWorldClientLighting::asyncLighting() const {
  return m_asyncLighting;
}

void StarWorldClientLighting::setAsyncLighting(bool asyncLighting) {
  m_asyncLighting = asyncLighting;
}

bool StarWorldClientLighting::interactiveHighlightMode() const {
  return m_interactiveHighlightMode;
}

void StarWorldClientLighting::setInteractiveHighlightMode(bool enabled) {
  m_interactiveHighlightMode = enabled;
}

float StarWorldClientLighting::lightLevel(Vec2F const& pos) const {
  if (!m_worldClient->inWorld())
    return 0.0f;
  return WorldImpl::lightLevel(m_worldClient->m_tileArray, m_worldClient->m_entityMap, m_worldClient->m_geometry, m_worldClient->m_worldTemplate, m_worldClient->m_sky, m_lightIntensityCalculator, pos, m_worldClient->m_materialDatabase, m_worldClient->m_liquidsDatabase);
}

bool StarWorldClientLighting::waitForLighting(WorldRenderData* renderData) {
  MutexLocker prepLocker(m_lightMapPrepMutex);
  MutexLocker lightMapLocker(m_lightMapMutex);
  if (renderData && !m_lightMap.empty()) {
    for (auto& previewTile : m_worldClient->m_previewTiles) {
      if (previewTile.updateLight) {
        Vec2I lightArrayPos = m_worldClient->m_geometry.diff(previewTile.position, m_lightMinPosition);
        if (lightArrayPos[0] >= 0 && lightArrayPos[0] < static_cast<int>(m_lightMap.width())
         && lightArrayPos[1] >= 0 && lightArrayPos[1] < static_cast<int>(m_lightMap.height()))
          m_lightMap.set(lightArrayPos[0], lightArrayPos[1], Color::v3bToFloat(previewTile.light));
      }
    }
    renderData->lightMap = std::move(m_lightMap);
    renderData->lightMinPosition = m_lightMinPosition;
    return true;
  }
  return false;
}

void StarWorldClientLighting::lightingTileGather() {
  int64_t start = Time::monotonicMicroseconds();
  Vec3F environmentLight = m_worldClient->m_sky->environmentLight().toRgbF();
  float undergroundLevel = m_worldClient->m_worldTemplate->undergroundLevel();
  auto liquidsDatabase = m_worldClient->m_liquidsDatabase;
  auto materialDatabase = m_worldClient->m_materialDatabase;

  m_worldClient->m_tileArray->tileEvalColumnsParallel(m_lightingCalculator.calculationRegion(), [&](Vec2I const& pos, ClientTile const* column, size_t ySize) {
    size_t baseIndex = m_lightingCalculator.baseIndexFor(pos);
    for (size_t y = 0; y < ySize; ++y) {
      auto& tile = column[y];
      Vec3F light;
      if (tile.foreground != EmptyMaterialId || tile.foregroundMod != NoModId)
        light += materialDatabase->radiantLight(tile.foreground, tile.foregroundMod);

      if (tile.liquid.liquid != EmptyLiquidId && tile.liquid.level != 0.0f)
        light += liquidsDatabase->radiantLight(tile.liquid);
      if (tile.foregroundLightTransparent) {
        if (tile.background != EmptyMaterialId || tile.backgroundMod != NoModId)
          light += materialDatabase->radiantLight(tile.background, tile.backgroundMod);
        if (tile.backgroundLightTransparent && pos[1] + y > undergroundLevel)
          light += environmentLight;
      }
      m_lightingCalculator.setCellIndex(baseIndex + y, light, !tile.foregroundLightTransparent);
    }
  });
  LogMap::set("client_render_world_async_light_gather", strf("{:05d}\xC2\xB5s", Time::monotonicMicroseconds() - start));
}

void StarWorldClientLighting::lightingCalc() {
  MutexLocker prepLocker(m_lightMapPrepMutex);
  if (!m_pendingLightReady.load())
    return;
  m_pendingLightReady = false;
  RectI lightRange = m_pendingLightRange;
  List<LightSource> lights = std::move(m_pendingLights);
  List<std::pair<Vec2F, Vec3F>> particleLights = std::move(m_pendingParticleLights);
  bool newLighting = m_worldClient->m_configuration->get("newLighting").optBool().value(true);
  bool monochrome = m_worldClient->m_configuration->get("monochromeLighting").toBool();
  m_lightingCalculator.setParameters(m_lightingConfig.set("pointAdditive", newLighting));
  m_lightingCalculator.setMonochrome(monochrome);
  m_lightingCalculator.begin(lightRange);
  lightingTileGather();

  prepLocker.unlock();

  for (auto const& light : lights) {
    Vec2F position = m_worldClient->m_geometry.nearestTo(Vec2F(m_lightingCalculator.calculationRegion().min()), light.position);
    if (light.type == LightType::Spread)
      m_lightingCalculator.addSpreadLight(position, light.color);
    else {
      if (light.type == LightType::PointAsSpread) {
        if (!newLighting)
          m_lightingCalculator.addSpreadLight(position, light.color);
        else {
          m_lightingCalculator.addSpreadLight(position, light.color * 0.85f);
          m_lightingCalculator.addPointLight(position, light.color, light.pointBeam, light.beamAngle, light.beamAmbience, true);
        }
      } else {
        m_lightingCalculator.addPointLight(position, light.color, light.pointBeam, light.beamAngle, light.beamAmbience);
      }
    }
  }

  for (auto const& lightPair : particleLights) {
    Vec2F position = m_worldClient->m_geometry.nearestTo(Vec2F(m_lightingCalculator.calculationRegion().min()), lightPair.first);
    m_lightingCalculator.addSpreadLight(position, lightPair.second);
  }

  m_lightingCalculator.calculate(m_pendingLightMap);
  {
    MutexLocker mapLocker(m_lightMapMutex);
    m_lightMinPosition = lightRange.min();
    m_lightMap = std::move(m_pendingLightMap);
  }
}

void StarWorldClientLighting::lightingMain() {
  MutexLocker condLocker(m_lightingMutex);
  while (true) {
    m_lightingCond.wait(m_lightingMutex);
    if (m_stopLightingThread)
      return;

    int64_t start = Time::monotonicMicroseconds();
    lightingCalc();
    LogMap::set("client_render_world_async_light_calc", strf("{:05d}\xC2\xB5s", Time::monotonicMicroseconds() - start));
  }
}

}
