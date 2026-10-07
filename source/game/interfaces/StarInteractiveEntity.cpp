#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
import star.interaction_types;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;

namespace Star {

RectF InteractiveEntity::interactiveBoundBox() const {
  return metaBoundBox();
}

bool InteractiveEntity::isInteractive() const {
  return true;
}

List<QuestArcDescriptor> InteractiveEntity::offeredQuests() const {
  return {};
}

StringSet InteractiveEntity::turnInQuests() const {
  return {};
}

Vec2F InteractiveEntity::questIndicatorPosition() const {
  return position();
}

}
