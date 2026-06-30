#pragma once

#include "StarGameTypes.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarJson.hpp"

namespace Star {

struct DungeonWorldData {
  HashMap<DungeonId, float> dungeonIdGravity;
  HashMap<DungeonId, bool> dungeonIdBreathable;
  StableHashSet<DungeonId> protectedDungeonIds;
};

}
