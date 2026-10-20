import std;
import star.set;
import star.json;
import star.id_map;
import star.casting;
import star.data_stream;
import star.game_types;
import star.poly;
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

namespace Star {

ToolUserItem::ToolUserItem() : m_owner(nullptr) {}

void ToolUserItem::init(ToolUserEntity* owner, ToolHand hand) {
  m_owner = owner;
  m_hand = hand;
}

void ToolUserItem::uninit() {
  m_owner = nullptr;
  m_hand = {};
}

void ToolUserItem::update(float, FireMode, bool, HashSet<MoveControlType> const&) {}

bool ToolUserItem::initialized() const {
  return (bool)m_owner;
}

ToolUserEntity* ToolUserItem::owner() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::owner");
  return m_owner;
}

EntityMode ToolUserItem::entityMode() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::entityMode");
  return m_owner->entityMode().value();
}

ToolHand ToolUserItem::hand() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::hand");
  return m_hand.value();
}

World* ToolUserItem::world() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::world");
  return m_owner->world();
}

List<DamageSource> ToolUserItem::damageSources() const {
  return {};
}

List<PolyF> ToolUserItem::shieldPolys() const {
  return {};
}

List<PhysicsForceRegion> ToolUserItem::forceRegions() const {
  return {};
}

}
