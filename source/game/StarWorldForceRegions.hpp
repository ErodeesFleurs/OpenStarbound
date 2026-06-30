#pragma once

#include "StarForceRegions.hpp"
#include "StarJson.hpp"

namespace Star {

class Assets;
class WorldTemplate;

namespace WorldForceRegions {
  List<PhysicsForceRegion> setupForceRegions(Json const& config, WorldTemplate* worldTemplate);
}

}
