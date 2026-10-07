module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
#include "StarPoly.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarMathCommon.hpp"
#include "StarRandom.hpp"
import star.periodic;
#include "StarInterpolation.hpp"
import star.periodic_function;
#include "StarSet.hpp"
#include "StarColor.hpp"
#include "StarLua.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"


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
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;

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
