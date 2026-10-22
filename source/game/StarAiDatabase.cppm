module;
import std;
import star.string;
import star.map;
import star.json;
import star.format;

import star.ordered_set;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;
import star.game_types;
import star.strong_typedef;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;

import star.animation;

namespace Star {

class Root;

class AiDatabase {
public:
  [[nodiscard]] static std::expected<std::shared_ptr<AiDatabase>, StarException> create(Root& root);

  [[nodiscard]] std::expected<AiMission, StarException> mission(String const& missionName) const;

  [[nodiscard]] std::expected<AiSpeech, StarException> shipStatus(unsigned shipLevel) const;
  AiSpeech noMissionsSpeech() const;
  AiSpeech noCrewSpeech() const;

  [[nodiscard]] std::expected<String, StarException> portraitImage(String const& species, String const& frame = "idle.0") const;
  [[nodiscard]] std::expected<Animation, StarException> animation(String const& species, String const& animationName) const;
  [[nodiscard]] std::expected<Animation, StarException> staticAnimation(String const& species) const;
  Animation scanlineAnimation() const;

  float charactersPerSecond() const;
  String defaultAnimation() const;

private:
  struct AiAnimationConfig {
    AiAnimationConfig(Animation staticAnimation, Animation scanlineAnimation)
      : staticAnimation(std::move(staticAnimation)), scanlineAnimation(std::move(scanlineAnimation)) {}

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

  AiDatabase(Animation staticAnimation, Animation scanlineAnimation);
  [[nodiscard]] static std::expected<AiSpeech, StarException> parseSpeech(Json const& v);
  [[nodiscard]] static std::expected<AiSpeciesParameters, StarException> parseSpeciesParameters(Json const& vm);

  [[nodiscard]] static std::expected<AiSpeciesMissionText, StarException> parseSpeciesMissionText(Json const& vm);
  [[nodiscard]] static std::expected<AiMission, StarException> parseMission(Json const& vm);

  StringMap<AiMission> m_missions;
  StringMap<AiSpeciesParameters> m_speciesParameters;
  Map<unsigned, AiSpeech> m_shipStatus;
  AiSpeech m_noMissionsSpeech;
  AiSpeech m_noCrewSpeech;

  AiAnimationConfig m_animationConfig;
};

}

export module star.ai_database;

export namespace Star {
  using ::Star::AiExceptionTag;
  using ::Star::AiException;
  using ::Star::AiSpeech;
  using ::Star::AiState;
  using ::Star::AiSpeciesMissionText;
  using ::Star::AiMission;
  using ::Star::AiDatabase;
}
