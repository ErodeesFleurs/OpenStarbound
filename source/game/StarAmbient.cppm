module;

import std;
import star.string;
import star.list;
import star.json;

namespace Star {

class AudioInstance;

struct AmbientTrackGroup {
  AmbientTrackGroup();
  AmbientTrackGroup(StringList tracks);
  AmbientTrackGroup(Json const& config, String const& directory = "");

  Json toJson() const;

  StringList tracks;
};

// represents the ambient sounds data for a biome
struct AmbientNoisesDescription {
  AmbientNoisesDescription();
  AmbientNoisesDescription(AmbientTrackGroup day, AmbientTrackGroup night, int loops = -1);
  AmbientNoisesDescription(Json const& config, String const& directory = "");

  Json toJson() const;

  AmbientTrackGroup daySounds;
  AmbientTrackGroup nightSounds;
  int trackLoops = -1;
};

typedef AmbientTrackGroup WeatherNoisesDescription;

// manages the running ambient sounds
class AmbientManager {
public:
  // Automatically calls cancelAll();
  ~AmbientManager();

  void setTrackSwitchGrace(float grace);
  void setTrackFadeInTime(float fadeInTime);

  // Returns a new AudioInstance if a new ambient sound is to be started.
  std::shared_ptr<AudioInstance> updateAmbient(std::shared_ptr<AmbientNoisesDescription> current, bool dayTime = false);
  std::shared_ptr<AudioInstance> updateWeather(std::shared_ptr<WeatherNoisesDescription> current);
  void cancelAll();

  void setVolume(float volume, float delay, float duration);

private:
  std::shared_ptr<AudioInstance> m_currentTrack;
  std::shared_ptr<AudioInstance> m_weatherTrack;
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

}

export module star.ambient;

export namespace Star {
  using ::Star::AudioInstance;
  using ::Star::AmbientTrackGroup;
  using ::Star::AmbientNoisesDescription;
  using ::Star::AmbientManager;
  using ::Star::WeatherNoisesDescription;
}
