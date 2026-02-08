#pragma once

#include "StarConfig.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

class AudioInstance;

struct AmbientTrackGroup {
  AmbientTrackGroup();
  AmbientTrackGroup(StringList tracks);
  AmbientTrackGroup(Json const& config, String const& directory = "");

  [[nodiscard]] auto toJson() const -> Json;

  StringList tracks;
};

// represents the ambient sounds data for a biome
struct AmbientNoisesDescription {
  AmbientNoisesDescription();
  AmbientNoisesDescription(AmbientTrackGroup day, AmbientTrackGroup night, int loops = -1);
  AmbientNoisesDescription(Json const& config, String const& directory = "");

  [[nodiscard]] auto toJson() const -> Json;

  AmbientTrackGroup daySounds;
  AmbientTrackGroup nightSounds;
  int trackLoops = -1;
};

using WeatherNoisesDescription = AmbientTrackGroup;

// manages the running ambient sounds
class AmbientManager {
public:
  // Automatically calls cancelAll();
  ~AmbientManager();

  void setTrackSwitchGrace(float grace);
  void setTrackFadeInTime(float fadeInTime);

  // Returns a new AudioInstance if a new ambient sound is to be started.
  auto updateAmbient(Ptr<AmbientNoisesDescription> current, bool dayTime = false) -> Ptr<AudioInstance>;
  auto updateWeather(Ptr<WeatherNoisesDescription> current) -> Ptr<AudioInstance>;
  void cancelAll();

  void setVolume(float volume, float delay, float duration);

private:
  Ptr<AudioInstance> m_currentTrack;
  Ptr<AudioInstance> m_weatherTrack;
  String m_currentTrackName;
  String m_weatherTrackName;
  float m_trackFadeInTime = 0.0f;
  float m_trackSwitchGrace = 0.0f;
  double m_trackGraceTimestamp = 0;
  Deque<String> m_recentTracks;
  float m_volume = 1.0f;
  float m_delay = 0.0f;
  float m_duration = 0.0f;
  bool m_volumeChanged = false;
};

}// namespace Star
