import std;
import star.string;
import star.json;
import star.id_map;
import star.casting;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.variant;
import star.rpc_promise;
import star.a_star;
import star.lua_root;
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
import star.lua_components;
import star.fireable_item;
import star.swingable_item;

import star.math_common;
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
  m_swingStart = params.getFloat("swingStart", 60) * std::numbers::pi / 180;
  m_swingFinish = params.getFloat("swingFinish", -40) * std::numbers::pi / 180;
  m_swingAimFactor = params.getFloat("swingAimFactor", 1);
  m_coolingDownAngle = params.optFloat("coolingDownAngle").transform([](float angle) { return angle * std::numbers::pi / 180; });
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
        return -std::numbers::pi / 2;
    }

    if (m_timeFiring < windupTime())
      return m_swingStart + (m_swingFinish - m_swingStart) * m_timeFiring / windupTime() + aimAngle * m_swingAimFactor;

    return m_swingFinish + (m_swingStart - m_swingFinish) * fireTimer() / (cooldownTime() + windupTime()) + aimAngle * m_swingAimFactor;
  }

  return -std::numbers::pi / 2;
}

float SwingableItem::getItemAngle(float aimAngle) {
  return getAngle(aimAngle);
}

String SwingableItem::getArmFrame() {
  return "rotation";
}

}
