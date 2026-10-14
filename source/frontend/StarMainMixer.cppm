module;

import std;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.maybe;
import star.bi_map;
import star.game_types;


import star.mixer;

namespace Star {

class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class WorldPainter;
using WorldPainterPtr = std::shared_ptr<WorldPainter>;
using WorldPainterConstPtr = std::shared_ptr<WorldPainter const>;
using WorldPainterWeakPtr = std::weak_ptr<WorldPainter>;
using WorldPainterConstWeakPtr = std::weak_ptr<WorldPainter const>;
using WorldPainterUPtr = std::unique_ptr<WorldPainter>;
using WorldPainterConstUPtr = std::unique_ptr<WorldPainter const>;
class MainMixer;
using MainMixerPtr = std::shared_ptr<MainMixer>;
using MainMixerConstPtr = std::shared_ptr<MainMixer const>;
using MainMixerWeakPtr = std::weak_ptr<MainMixer>;
using MainMixerConstWeakPtr = std::weak_ptr<MainMixer const>;
using MainMixerUPtr = std::unique_ptr<MainMixer>;
using MainMixerConstUPtr = std::unique_ptr<MainMixer const>;

class MainMixer {
public:
  MainMixer(unsigned sampleRate, unsigned channels);

  void setUniverseClient(UniverseClientPtr universeClient);
  void setWorldPainter(WorldPainterPtr worldPainter);

  void update(float dt, bool muteSfx = false, bool muteMusic = false);

  MixerPtr mixer() const;

  void setSpeed(float speed);
  void setVolume(float volume, float rampTime = 0.0f);
  void read(std::int16_t* sampleData, std::size_t frameCount, Mixer::ExtraMixFunction = {});

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
