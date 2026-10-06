module;

#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"


import star.mixer;

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

export module star.main_mixer;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::WorldPainter;
  using ::Star::WorldPainterPtr;
  using ::Star::WorldPainterConstPtr;
  using ::Star::WorldPainterWeakPtr;
  using ::Star::WorldPainterConstWeakPtr;
  using ::Star::WorldPainterUPtr;
  using ::Star::WorldPainterConstUPtr;
  using ::Star::MainMixer;
  using ::Star::MainMixerPtr;
  using ::Star::MainMixerConstPtr;
  using ::Star::MainMixerWeakPtr;
  using ::Star::MainMixerConstWeakPtr;
  using ::Star::MainMixerUPtr;
  using ::Star::MainMixerConstUPtr;
}
