module;

import std;
import star.string;
import star.list;
import star.json;

namespace Star {

class AudioInstance;
using AudioInstancePtr = std::shared_ptr<AudioInstance>;
using AudioInstanceConstPtr = std::shared_ptr<const AudioInstance>;
using AudioInstanceWeakPtr = std::weak_ptr<AudioInstance>;
using AudioInstanceConstWeakPtr = std::weak_ptr<const AudioInstance>;
using AudioInstanceUPtr = std::unique_ptr<AudioInstance>;
using AudioInstanceConstUPtr = std::unique_ptr<const AudioInstance>;
struct AmbientTrackGroup;
using AmbientTrackGroupPtr = std::shared_ptr<AmbientTrackGroup>;
using AmbientTrackGroupConstPtr = std::shared_ptr<const AmbientTrackGroup>;
using AmbientTrackGroupWeakPtr = std::weak_ptr<AmbientTrackGroup>;
using AmbientTrackGroupConstWeakPtr = std::weak_ptr<const AmbientTrackGroup>;
using AmbientTrackGroupUPtr = std::unique_ptr<AmbientTrackGroup>;
using AmbientTrackGroupConstUPtr = std::unique_ptr<const AmbientTrackGroup>;
struct AmbientNoisesDescription;
using AmbientNoisesDescriptionPtr = std::shared_ptr<AmbientNoisesDescription>;
using AmbientNoisesDescriptionConstPtr = std::shared_ptr<const AmbientNoisesDescription>;
using AmbientNoisesDescriptionWeakPtr = std::weak_ptr<AmbientNoisesDescription>;
using AmbientNoisesDescriptionConstWeakPtr = std::weak_ptr<const AmbientNoisesDescription>;
using AmbientNoisesDescriptionUPtr = std::unique_ptr<AmbientNoisesDescription>;
using AmbientNoisesDescriptionConstUPtr = std::unique_ptr<const AmbientNoisesDescription>;
class AmbientManager;
using AmbientManagerPtr = std::shared_ptr<AmbientManager>;
using AmbientManagerConstPtr = std::shared_ptr<const AmbientManager>;
using AmbientManagerWeakPtr = std::weak_ptr<AmbientManager>;
using AmbientManagerConstWeakPtr = std::weak_ptr<const AmbientManager>;
using AmbientManagerUPtr = std::unique_ptr<AmbientManager>;
using AmbientManagerConstUPtr = std::unique_ptr<const AmbientManager>;

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
typedef std::shared_ptr<WeatherNoisesDescription> WeatherNoisesDescriptionPtr;

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
