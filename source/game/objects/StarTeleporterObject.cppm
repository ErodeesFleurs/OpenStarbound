module;
#include "StarIdMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
#include "StarPoly.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarJsonExtra.hpp"

import star.uuid;
import star.celestial_coordinate;
import star.warping;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.warp_target_entity;
#include "StarObject.hpp"

export module star.teleporter_object;

export namespace Star {

class TeleporterObject : public Object, public WarpTargetEntity {
public:
  TeleporterObject(ObjectConfigConstPtr config, Json const& parameters = JsonObject());

  Vec2F footPosition() const override;
};

}

namespace Star {

TeleporterObject::TeleporterObject(ObjectConfigConstPtr config, Json const& parameters) : Object(config, parameters) {
  setUniqueId(configValue("uniqueId", Uuid().hex()).optString());
}

Vec2F TeleporterObject::footPosition() const {
  if (auto footPos = configValue("teleporterFootPosition"))
    return jsonToVec2F(footPos);
  return Vec2F();
}

}
