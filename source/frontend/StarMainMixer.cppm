module;

import std;
import star.format;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.bi_map;
import star.game_types;


import star.mixer;

namespace Star {

class UniverseClient;
class WorldPainter;
class Root;
class Voice;

class MainMixer {
public:
  MainMixer(Root& root, unsigned sampleRate, unsigned channels);

  void setUniverseClient(std::shared_ptr<UniverseClient> universeClient);
  void setWorldPainter(std::shared_ptr<WorldPainter> worldPainter);
  // Borrows the client voice service; bind after construction and clear before it is destroyed.
  void setVoice(Voice* voice);

  [[nodiscard]] std::expected<void, StarException> update(float dt, bool muteSfx = false, bool muteMusic = false);

  std::shared_ptr<Mixer> mixer() const;

  void setSpeed(float speed);
  void setVolume(float volume, float rampTime = 0.0f);
  [[nodiscard]] std::expected<void, StarException> read(std::int16_t* sampleData, std::size_t frameCount, Mixer::ExtraMixFunction = {});

private:
  Root* m_root;
  Voice* m_voice = nullptr;
  std::shared_ptr<UniverseClient> m_universeClient;
  std::shared_ptr<WorldPainter> m_worldPainter;
  std::shared_ptr<Mixer> m_mixer;
  Set<MixerGroup> m_mutedGroups;
  Map<MixerGroup, float> m_groupVolumes;
};

}

export module star.main_mixer;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::WorldPainter;
  using ::Star::MainMixer;
}
