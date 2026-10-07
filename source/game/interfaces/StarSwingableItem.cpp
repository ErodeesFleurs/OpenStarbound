#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarLuaRoot.hpp"
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.animation;
import star.particle;
import star.interaction_types;
import star.tile_damage;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.actor_movement_controller;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.status_effect_item;
#include "StarLuaComponents.hpp"
import star.fireable_item;
import star.swingable_item;

namespace Star {

SwingableItem::SwingableItem() {
  m_swingAimFactor = 0;
  m_swingStart = 0;
  m_swingFinish = 0;
}

SwingableItem::SwingableItem(Json const& params) : FireableItem(params) {
  setParams(params);
}

void SwingableItem::setParams(Json const& params) {
  m_swingStart = params.getFloat("swingStart", 60) * Constants::pi / 180;
  m_swingFinish = params.getFloat("swingFinish", -40) * Constants::pi / 180;
  m_swingAimFactor = params.getFloat("swingAimFactor", 1);
  m_coolingDownAngle = params.optFloat("coolingDownAngle").apply([](float angle) { return angle * Constants::pi / 180; });
  FireableItem::setParams(params);
}

float SwingableItem::getAngleDir(float angle, Direction) {
  return getAngle(angle);
}

float SwingableItem::getAngle(float aimAngle) {
  if (!ready()) {
    if (coolingDown()) {
      if (m_coolingDownAngle)
        return *m_coolingDownAngle + aimAngle * m_swingAimFactor;
      else
        return -Constants::pi / 2;
    }

    if (m_timeFiring < windupTime())
      return m_swingStart + (m_swingFinish - m_swingStart) * m_timeFiring / windupTime() + aimAngle * m_swingAimFactor;

    return m_swingFinish + (m_swingStart - m_swingFinish) * fireTimer() / (cooldownTime() + windupTime()) + aimAngle * m_swingAimFactor;
  }

  return -Constants::pi / 2;
}

float SwingableItem::getItemAngle(float aimAngle) {
  return getAngle(aimAngle);
}

String SwingableItem::getArmFrame() {
  return "rotation";
}

}
