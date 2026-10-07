#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarThread.hpp"
import star.time;
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;
#include "StarRandom.hpp"
#include "StarGameTypes.hpp"

import star.mixer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.ambient;

namespace Star {

AmbientTrackGroup::AmbientTrackGroup() {
  tracks = {};
}

AmbientTrackGroup::AmbientTrackGroup(StringList tracks) : tracks(std::move(tracks)) {}

AmbientTrackGroup::AmbientTrackGroup(Json const& config, String const& directory) {
  for (auto track : jsonToStringList(config.get("tracks", JsonArray())))
    tracks.append(AssetPath::relativeTo(directory, track));
}

Json AmbientTrackGroup::toJson() const {
  return JsonObject{{"tracks", jsonFromStringList(tracks)}};
}

AmbientNoisesDescription::AmbientNoisesDescription() : trackLoops(-1) {}

AmbientNoisesDescription::AmbientNoisesDescription(AmbientTrackGroup day, AmbientTrackGroup night, int loops)
  : daySounds(std::move(day)), nightSounds(std::move(night)), trackLoops(loops) {}

AmbientNoisesDescription::AmbientNoisesDescription(Json const& config, String const& directory) {
  if (auto day = config.opt("day"))
    daySounds = AmbientTrackGroup(*day, directory);
  if (auto night = config.opt("night"))
    nightSounds = AmbientTrackGroup(*night, directory);
  if (auto loops = config.optInt("loops"))
    trackLoops = *loops;
}

Json AmbientNoisesDescription::toJson() const {
  return JsonObject{{"day", daySounds.toJson()}, {"night", nightSounds.toJson()}, {"loops", trackLoops}};
}

AmbientManager::~AmbientManager() {
  cancelAll();
}

void AmbientManager::setTrackSwitchGrace(float grace) {
  m_trackSwitchGrace = grace;
}

void AmbientManager::setTrackFadeInTime(float fadeInTime) {
  m_trackFadeInTime = fadeInTime;
}

AudioInstancePtr AmbientManager::updateAmbient(AmbientNoisesDescriptionPtr current, bool dayTime) {
  auto assets = Root::singleton().assets();

  if (m_currentTrack) {
    if (m_currentTrack->finished())
      m_currentTrack = {};
  }
  StringList tracks;
  if (current)
    tracks = dayTime ? current->daySounds.tracks : current->nightSounds.tracks;

  if (m_currentTrack) {
    if (m_currentTrack->finished() || !tracks.contains(m_currentTrackName)) {
      if (m_trackSwitchGrace <= Time::monotonicTime() - m_trackGraceTimestamp) {
        m_currentTrack->stop(m_trackFadeInTime);
        m_currentTrack = {};
      }
    } else {
      m_trackGraceTimestamp = Time::monotonicTime();
    }
  }
  if (!m_currentTrack) {
    m_currentTrackName = "";
    if (tracks.size() > 0) {
      while ((m_recentTracks.size() / 2) >= tracks.size())
        m_recentTracks.removeFirst();
      while (true) {
        m_currentTrackName = Random::randValueFrom(tracks);
        if (m_currentTrackName.empty() || !m_recentTracks.contains(m_currentTrackName))
          break;
        m_recentTracks.removeFirst(); // reduce chance of collisions on collision
      }
    }
    if (!m_currentTrackName.empty()) {
      if (auto audio = assets->tryAudio(m_currentTrackName)) {
        m_recentTracks.append(m_currentTrackName);
        m_currentTrack = make_shared<AudioInstance>(*audio);
        m_currentTrack->setLoops(current ? current->trackLoops : -1);
        // Slowly fade the music track in
        m_currentTrack->setVolume(0.0f);
        m_currentTrack->setVolume(m_volume, m_trackFadeInTime);
        m_delay = 0;
        m_duration = 0;
        m_volumeChanged = false;
        return m_currentTrack;
      }
    }
  }
  if (m_volumeChanged) {
    if (m_delay > 0)
      m_delay -= GlobalTimestep;
    else {
      m_volumeChanged = false;
      if (m_currentTrack) {
        m_currentTrack->setVolume(m_volume, m_duration);
      }
    }
  }

  return {};
}

AudioInstancePtr AmbientManager::updateWeather(WeatherNoisesDescriptionPtr current) {
  auto assets = Root::singleton().assets();

  if (m_weatherTrack) {
    if (m_weatherTrack->finished())
      m_weatherTrack = {};
  }

  StringList tracks;
  if (current)
    tracks = current->tracks;

  if (m_weatherTrack) {
    if (!tracks.contains(m_weatherTrackName)) {
      m_weatherTrack->stop(10.0f);
      m_weatherTrack = {};
    }
  }

  if (!m_weatherTrack) {
    m_weatherTrackName = Random::randValueFrom(tracks);
    if (!m_weatherTrackName.empty()) {
      if (auto audio = assets->tryAudio(m_weatherTrackName)) {
        m_weatherTrack = make_shared<AudioInstance>(*audio);
        m_weatherTrack->setLoops(-1);
        m_weatherTrack->setVolume(0.0f);
        m_weatherTrack->setVolume(1, m_trackFadeInTime);
        return m_weatherTrack;
      }
    }
  }

  return {};
}

void AmbientManager::cancelAll() {
  if (m_weatherTrack) {
    m_weatherTrack->stop();
    m_weatherTrack = {};
  }
  if (m_currentTrack) {
    m_currentTrack->stop();
    m_currentTrack = {};
  }
}

void AmbientManager::setVolume(float volume, float delay, float duration) {
  if (m_volume == volume)
    return;
  m_volume = volume;
  m_delay = delay;
  m_duration = duration;
  m_volumeChanged = true;
}

}
