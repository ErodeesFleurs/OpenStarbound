#include "StarConfig.hpp"
import star.json;
import star.id_map;
import star.game_types;
#include "StarCasting.hpp"
import star.data_stream;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.list;
import star.rpc_promise;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.container_entity;

import star.item_bag;

namespace Star {

size_t ContainerEntity::containerSize() const {
  return itemBag()->size();
}

List<ItemPtr> ContainerEntity::containerItems() const {
  return itemBag()->items();
}

}

