#include "StarJsonExtra.hpp"
#include "StarOrderedSet.hpp"
#include "StarItemDescriptor.hpp"
#include "StarQuestDescriptor.hpp"
import star.ai_types;

namespace Star {

AiState::AiState() {}

AiState::AiState(Json const& v) {
  availableMissions.addAll(jsonToStringList(v.get("availableMissions", JsonArray())));
  completedMissions.addAll(jsonToStringList(v.get("completedMissions", JsonArray())));
}

Json AiState::toJson() const {
  return JsonObject{{"availableMissions", jsonFromStringList(availableMissions.values())},
      {"completedMissions", jsonFromStringList(completedMissions.values())}};
}

}
