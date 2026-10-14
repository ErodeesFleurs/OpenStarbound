#include "StarConfig.hpp"
import star.string;
import star.list;
import star.json;
import star.id_map;
import star.game_types;
#include "StarCasting.hpp"
import star.vector;
import star.data_stream;
#include "StarStrongTypedef.hpp"
import star.bi_map;
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

import star.rect;
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
