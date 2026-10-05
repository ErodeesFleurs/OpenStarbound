module;

#include "StarJson.hpp"

namespace Star {

STAR_CLASS(AudioInstance);
STAR_STRUCT(AmbientTrackGroup);
STAR_STRUCT(AmbientNoisesDescription);
STAR_CLASS(AmbientManager);

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
typedef shared_ptr<WeatherNoisesDescription> WeatherNoisesDescriptionPtr;

// manages the running ambient sounds
class AmbientManager {
public:
  // Automatically calls cancelAll();
  ~AmbientManager();

  void setTrackSwitchGrace(float grace);
  void setTrackFadeInTime(float fadeInTime);

  // Returns a new AudioInstance if a new ambient sound is to be started.
  AudioInstancePtr updateAmbient(AmbientNoisesDescriptionPtr current, bool dayTime = false);
  AudioInstancePtr updateWeather(WeatherNoisesDescriptionPtr current);
  void cancelAll();

  void setVolume(float volume, float delay, float duration);

private:
  AudioInstancePtr m_currentTrack;
  AudioInstancePtr m_weatherTrack;
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
  using ::Star::AudioInstancePtr;
  using ::Star::AudioInstanceConstPtr;
  using ::Star::AudioInstanceWeakPtr;
  using ::Star::AudioInstanceConstWeakPtr;
  using ::Star::AudioInstanceUPtr;
  using ::Star::AudioInstanceConstUPtr;
  using ::Star::AmbientTrackGroup;
  using ::Star::AmbientTrackGroupPtr;
  using ::Star::AmbientTrackGroupConstPtr;
  using ::Star::AmbientTrackGroupWeakPtr;
  using ::Star::AmbientTrackGroupConstWeakPtr;
  using ::Star::AmbientTrackGroupUPtr;
  using ::Star::AmbientTrackGroupConstUPtr;
  using ::Star::AmbientNoisesDescription;
  using ::Star::AmbientNoisesDescriptionPtr;
  using ::Star::AmbientNoisesDescriptionConstPtr;
  using ::Star::AmbientNoisesDescriptionWeakPtr;
  using ::Star::AmbientNoisesDescriptionConstWeakPtr;
  using ::Star::AmbientNoisesDescriptionUPtr;
  using ::Star::AmbientNoisesDescriptionConstUPtr;
  using ::Star::AmbientManager;
  using ::Star::AmbientManagerPtr;
  using ::Star::AmbientManagerConstPtr;
  using ::Star::AmbientManagerWeakPtr;
  using ::Star::AmbientManagerConstWeakPtr;
  using ::Star::AmbientManagerUPtr;
  using ::Star::AmbientManagerConstUPtr;
  using ::Star::WeatherNoisesDescription;
  using ::Star::WeatherNoisesDescriptionPtr;
}
