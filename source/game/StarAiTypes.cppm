module;
import std;
import star.string;
import star.format;
import star.json;

import star.ordered_set;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;
import star.game_types;
import star.strong_typedef;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;

namespace Star {

struct AiExceptionTag {
  static constexpr char const* name() { return "AiException"; }
};
using AiException = StarError<AiExceptionTag, StarException>;

struct AiSpeech {
  String animation;
  String text;
  float speedModifier;
};

struct AiState {
  AiState();
  [[nodiscard]] static std::expected<AiState, StarException> create(Json const& v);

  Json toJson() const;

  OrderedHashSet<String> availableMissions;
  OrderedHashSet<String> completedMissions;
};

struct AiSpeciesMissionText {
  String buttonText;
  String repeatButtonText;
  AiSpeech selectSpeech;
};

struct AiMission {
  String missionName;
  String missionUniqueWorld;
  std::optional<String> warpAnimation;
  std::optional<bool> warpDeploy;
  String icon;
  StringMap<AiSpeciesMissionText> speciesText;
};

}

export module star.ai_types;

export namespace Star {
  using ::Star::AiExceptionTag;
  using ::Star::AiException;
  using ::Star::AiSpeech;
  using ::Star::AiState;
  using ::Star::AiSpeciesMissionText;
  using ::Star::AiMission;
}
