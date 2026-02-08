#pragma once

#include "StarAiTypes.hpp"
#include "StarAnimation.hpp"

namespace Star {

class AiDatabase {
public:
  AiDatabase();

  [[nodiscard]] auto mission(String const& missionName) const -> AiMission;

  [[nodiscard]] auto shipStatus(unsigned shipLevel) const -> AiSpeech;
  [[nodiscard]] auto noMissionsSpeech() const -> AiSpeech;
  [[nodiscard]] auto noCrewSpeech() const -> AiSpeech;

  [[nodiscard]] auto portraitImage(String const& species, String const& frame = "idle.0") const -> String;
  [[nodiscard]] auto animation(String const& species, String const& animationName) const -> Animation;
  [[nodiscard]] auto staticAnimation(String const& species) const -> Animation;
  [[nodiscard]] auto scanlineAnimation() const -> Animation;

  [[nodiscard]] auto charactersPerSecond() const -> float;
  [[nodiscard]] auto defaultAnimation() const -> String;

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

  static auto parseSpeech(Json const& v) -> AiSpeech;
  static auto parseSpeciesParameters(Json const& vm) -> AiSpeciesParameters;

  static auto parseSpeciesMissionText(Json const& vm) -> AiSpeciesMissionText;
  static auto parseMission(Json const& vm) -> AiMission;

  StringMap<AiMission> m_missions;
  StringMap<AiSpeciesParameters> m_speciesParameters;
  Map<unsigned, AiSpeech> m_shipStatus;
  AiSpeech m_noMissionsSpeech;
  AiSpeech m_noCrewSpeech;

  AiAnimationConfig m_animationConfig;
};

}// namespace Star
