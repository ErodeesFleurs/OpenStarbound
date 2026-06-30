#pragma once

#include "StarCellularLighting.hpp"
#include "StarThread.hpp"
#include "StarLightSource.hpp"
#include "StarWorldRenderData.hpp"

namespace Star {

class WorldClient;

class StarWorldClientLighting {
public:
  friend class WorldClient;

  explicit StarWorldClientLighting(WorldClient& worldClient);

  bool fullBright() const;
  void setFullBright(bool fullBright);
  bool asyncLighting() const;
  void setAsyncLighting(bool asyncLighting);
  bool interactiveHighlightMode() const;
  void setInteractiveHighlightMode(bool enabled);

  float lightLevel(Vec2F const& pos) const;

  bool waitForLighting(WorldRenderData* renderData = nullptr);

  void lightingMain();

private:
  void lightingTileGather();
  void lightingCalc();

  WorldClient& m_worldClient;

  Json m_lightingConfig;
  bool m_fullBright = false;
  bool m_asyncLighting = false;
  CellularLightingCalculator m_lightingCalculator;
  mutable CellularLightIntensityCalculator m_lightIntensityCalculator;
  ThreadFunction<void> m_lightingThread;
  Mutex m_lightingMutex;
  ConditionVariable m_lightingCond;
  atomic<bool> m_stopLightingThread;
  Mutex m_lightMapPrepMutex;
  Mutex m_lightMapMutex;
  Lightmap m_pendingLightMap;
  Lightmap m_lightMap;
  List<LightSource> m_pendingLights;
  struct PendingParticleLight {
    Vec2F position;
    Vec3F light;
  };
  List<PendingParticleLight> m_pendingParticleLights;
  RectI m_pendingLightRange;
  atomic<bool> m_pendingLightReady;
  Vec2I m_lightMinPosition;
  bool m_interactiveHighlightMode = false;
};

}
