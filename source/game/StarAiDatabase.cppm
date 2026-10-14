module;
import std;
import star.string;
import star.map;
import star.json;

import star.ordered_set;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;

import star.animation;

namespace Star {

class AiDatabase;
using AiDatabasePtr = std::shared_ptr<AiDatabase>;
using AiDatabaseConstPtr = std::shared_ptr<const AiDatabase>;
using AiDatabaseWeakPtr = std::weak_ptr<AiDatabase>;
using AiDatabaseConstWeakPtr = std::weak_ptr<const AiDatabase>;
using AiDatabaseUPtr = std::unique_ptr<AiDatabase>;
using AiDatabaseConstUPtr = std::unique_ptr<const AiDatabase>;

class AiDatabase {
public:
  AiDatabase();

  AiMission mission(String const& missionName) const;

  AiSpeech shipStatus(unsigned shipLevel) const;
  AiSpeech noMissionsSpeech() const;
  AiSpeech noCrewSpeech() const;

  String portraitImage(String const& species, String const& frame = "idle.0") const;
  Animation animation(String const& species, String const& animationName) const;
  Animation staticAnimation(String const& species) const;
  Animation scanlineAnimation() const;

  float charactersPerSecond() const;
  String defaultAnimation() const;

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

  static AiSpeech parseSpeech(Json const& v);
  static AiSpeciesParameters parseSpeciesParameters(Json const& vm);

  static AiSpeciesMissionText parseSpeciesMissionText(Json const& vm);
  static AiMission parseMission(Json const& vm);

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
  using ::Star::AiDatabasePtr;
  using ::Star::AiDatabaseConstPtr;
  using ::Star::AiDatabaseWeakPtr;
  using ::Star::AiDatabaseConstWeakPtr;
  using ::Star::AiDatabaseUPtr;
  using ::Star::AiDatabaseConstUPtr;
}
