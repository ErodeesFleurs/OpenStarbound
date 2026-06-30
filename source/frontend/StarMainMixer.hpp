#pragma once

#include "StarMixer.hpp"
#include "StarGameTypes.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"

namespace Star {

class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class WorldPainter;
using WorldPainterPtr = SharedPtr<WorldPainter>;
class MainMixer;
using MainMixerPtr = SharedPtr<MainMixer>;
class Voice;

class MainMixer {
public:
  struct Services {
    AssetsConstPtr assets;
    ConfigurationPtr configuration;
  };

  MainMixer(unsigned sampleRate, unsigned channels, Voice& voice, Services services);

  void setUniverseClient(UniverseClientPtr universeClient);
  void setWorldPainter(WorldPainterPtr worldPainter);

  void update(float dt, bool muteSfx = false, bool muteMusic = false);

  [[nodiscard]] MixerPtr mixer() const;

  void setSpeed(float speed);
  void setVolume(float volume, float rampTime = 0.0f);
  void read(int16_t* sampleData, size_t frameCount, Mixer::ExtraMixFunction = {});

private:
  UniverseClientPtr m_universeClient;
  WorldPainterPtr m_worldPainter;
  Voice& m_voice;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  MixerPtr m_mixer;
  Set<MixerGroup> m_mutedGroups;
  Map<MixerGroup, float> m_groupVolumes;
};

}
