#pragma once

#include "StarAmbient.hpp"
#include "StarVector.hpp"

namespace Star {

class WorldClient;

class StarWorldClientAudio {
public:
  friend class WorldClient;

  explicit StarWorldClientAudio(WorldClient& worldClient);

  Vec2I environmentBiomeTrackPosition() const;
  AmbientNoisesDescriptionPtr currentAmbientNoises() const;
  AmbientNoisesDescriptionPtr currentMusicTrack() const;
  AmbientNoisesDescriptionPtr currentAltMusicTrack() const;

  void playAltMusic(StringList const& newTracks, float fadeTime, int loops = -1);
  void stopAltMusic(float fadeTime);

  List<AudioInstancePtr> pullPendingAudio();
  List<AudioInstancePtr> pullPendingMusic();

private:
  WorldClient& m_worldClient;

  AmbientManager m_ambientSounds;
  AmbientManager m_musicTrack;
  AmbientManager m_altMusicTrack;
  AudioInstancePtr m_spaceSound;
  String m_activeSpaceSound;
  AmbientNoisesDescriptionPtr m_altMusicTrackDescription;
  bool m_altMusicActive = false;
  List<AudioInstancePtr> m_samples;
  List<AudioInstancePtr> m_music;
};

}
