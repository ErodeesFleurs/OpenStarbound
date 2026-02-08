#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarString.hpp"

import std;

namespace Star {

using AiException = ExceptionDerived<"AiException">;

struct AiSpeech {
  String animation;
  String text;
  float speedModifier;
};

struct AiState {
  AiState();
  AiState(Json const& v);

  [[nodiscard]] auto toJson() const -> Json;

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

}// namespace Star
