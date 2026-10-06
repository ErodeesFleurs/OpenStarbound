#include "StarJsonExtra.hpp"
#include "StarOrderedSet.hpp"
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
import star.item_descriptor;
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
import star.item_descriptor;
#include "StarVector.hpp"
import star.celestial_coordinate;
import star.quest_descriptor;
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
