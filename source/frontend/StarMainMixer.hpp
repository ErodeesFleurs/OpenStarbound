#pragma once

#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
import star.mixer;
#include "StarGameTypes.hpp"

namespace Star {

STAR_CLASS(UniverseClient);
STAR_CLASS(WorldPainter);
STAR_CLASS(MainMixer);

class MainMixer {
public:
  MainMixer(unsigned sampleRate, unsigned channels);

  void setUniverseClient(UniverseClientPtr universeClient);
  void setWorldPainter(WorldPainterPtr worldPainter);

  void update(float dt, bool muteSfx = false, bool muteMusic = false);

  MixerPtr mixer() const;

  void setSpeed(float speed);
  void setVolume(float volume, float rampTime = 0.0f);
  void read(int16_t* sampleData, size_t frameCount, Mixer::ExtraMixFunction = {});

private:
  UniverseClientPtr m_universeClient;
  WorldPainterPtr m_worldPainter;
  MixerPtr m_mixer;
  Set<MixerGroup> m_mutedGroups;
  Map<MixerGroup, float> m_groupVolumes;
};

}
