#pragma once

#include "StarAiTypes.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

class AiDatabase {
public:
  AiDatabase(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] AiMission mission(String const& missionName) const;

  [[nodiscard]] AiSpeech shipStatus(unsigned shipLevel) const;
  [[nodiscard]] AiSpeech noMissionsSpeech() const;
  [[nodiscard]] AiSpeech noCrewSpeech() const;

  [[nodiscard]] String portraitImage(String const& species, String const& frame = "idle.0") const;
  [[nodiscard]] Animation animation(String const& species, String const& animationName) const;
  [[nodiscard]] Animation staticAnimation(String const& species) const;
  [[nodiscard]] Animation scanlineAnimation() const;

  [[nodiscard]] float charactersPerSecond() const;
  [[nodiscard]] String defaultAnimation() const;

private:
  struct AiAnimationConfig {
    StringMap<Animation> aiAnimations;
    String defaultAnimation;
    float charactersPerSecond;

    Animation staticAnimation;
    float staticOpacity;

    Animation scanlineAnimation;
    float scanlineOpacity;
  };

  struct AiSpeciesParameters {
    String aiFrames;
    String portraitFrames;
    String staticFrames;
  };

  [[nodiscard]] static AiSpeech parseSpeech(Json const& v);
  [[nodiscard]] static AiSpeciesParameters parseSpeciesParameters(Json const& vm);

  [[nodiscard]] static AiSpeciesMissionText parseSpeciesMissionText(Json const& vm);
  [[nodiscard]] static AiMission parseMission(Json const& vm);

  StringMap<AiMission> m_missions;
  StringMap<AiSpeciesParameters> m_speciesParameters;
  Map<unsigned, AiSpeech> m_shipStatus;
  AiSpeech m_noMissionsSpeech;
  AiSpeech m_noCrewSpeech;

  AiAnimationConfig m_animationConfig;
};

}
