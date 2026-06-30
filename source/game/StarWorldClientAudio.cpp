#include "StarWorldClientAudio.hpp"
#include "StarWorldClient.hpp"
#include "StarWorldTemplate.hpp"
#include "StarSky.hpp"
#include "StarRoot.hpp"

namespace Star {

StarWorldClientAudio::StarWorldClientAudio(WorldClient* worldClient)
  : m_worldClient(worldClient) {}

Vec2I StarWorldClientAudio::environmentBiomeTrackPosition() const {
  if (!m_worldClient->inWorld())
    return {};

  auto pos = Vec2I::floor(m_worldClient->m_clientState.windowCenter());
  return {m_worldClient->m_geometry.xwrap(pos[0]), pos[1]};
}

AmbientNoisesDescriptionPtr StarWorldClientAudio::currentAmbientNoises() const {
  if (!m_worldClient->inWorld())
    return {};

  Vec2I pos = environmentBiomeTrackPosition();
  return m_worldClient->m_worldTemplate->ambientNoises(pos[0], pos[1]);
}

AmbientNoisesDescriptionPtr StarWorldClientAudio::currentMusicTrack() const {
  if (!m_worldClient->inWorld())
    return {};

  Vec2I pos = environmentBiomeTrackPosition();
  return m_worldClient->m_worldTemplate->musicTrack(pos[0], pos[1]);
}

void StarWorldClientAudio::playAltMusic(StringList const& newTracks, float fadeTime, int loops) {
  auto newTrackGroup = AmbientTrackGroup(newTracks);
  m_altMusicTrackDescription = make_shared<AmbientNoisesDescription>(AmbientTrackGroup(newTracks), AmbientTrackGroup(), loops);
  if (!m_altMusicActive) {
    m_musicTrack.setVolume(0.0, 0.0, fadeTime);
    m_altMusicTrack.setVolume(1.0, 0.0, fadeTime);
    m_altMusicActive = true;
  }
}

void StarWorldClientAudio::stopAltMusic(float fadeTime) {
  if (m_altMusicActive) {
    m_musicTrack.setVolume(1.0, 0.0, fadeTime);
    m_altMusicTrack.setVolume(0.0, 0.0, fadeTime);
    m_altMusicActive = false;
  }
}

List<AudioInstancePtr> StarWorldClientAudio::pullPendingAudio() {
  return take(m_samples);
}

List<AudioInstancePtr> StarWorldClientAudio::pullPendingMusic() {
  return take(m_music);
}

}
