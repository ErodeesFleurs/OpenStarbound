module;
import star.json;
#include "StarJsonExtra.hpp"
import star.id_map;
import star.variant;
#include "StarCasting.hpp"
import star.vector;
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.list;
import star.rpc_promise;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.maybe;
import star.a_star;
import star.set;
import star.string;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;
import star.ordered_set;
import star.audio;
import star.map;
import star.either;
import star.random;
import star.periodic_function;


import star.drawable;
import star.item;
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
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;
import star.animation;
import star.particle;
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

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.uuid;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;

export module star.blueprint_item;

export namespace Star {

class BlueprintItem;
using BlueprintItemPtr = std::shared_ptr<BlueprintItem>;
using BlueprintItemConstPtr = std::shared_ptr<const BlueprintItem>;
using BlueprintItemWeakPtr = std::weak_ptr<BlueprintItem>;
using BlueprintItemConstWeakPtr = std::weak_ptr<const BlueprintItem>;
using BlueprintItemUPtr = std::unique_ptr<BlueprintItem>;
using BlueprintItemConstUPtr = std::unique_ptr<const BlueprintItem>;

class BlueprintItem : public Item, public SwingableItem {
public:
  BlueprintItem(Json const& config, String const& directory, Json const& data);
  virtual ItemPtr clone() const override;

  virtual List<Drawable> drawables() const override;

  virtual void fireTriggered() override;

  virtual List<Drawable> iconDrawables() const override;
  virtual List<Drawable> dropDrawables() const override;

private:
  ItemDescriptor m_recipe;
  Drawable m_recipeIconUnderlay;
  List<Drawable> m_inHandDrawable;
};

}

namespace Star {

BlueprintItem::BlueprintItem(Json const& config, String const& directory, Json const& data)
  : Item(config, directory, data), SwingableItem(config) {
  setWindupTime(0.2f);
  setCooldownTime(0.1f);
  m_requireEdgeTrigger = true;
  m_recipe = ItemDescriptor(instanceValue("recipe"));

  m_recipeIconUnderlay = Drawable(Root::singleton().assets()->json("/blueprint.config:iconUnderlay"));
  m_inHandDrawable = {Drawable::makeImage(
      Root::singleton().assets()->json("/blueprint.config:inHandImage").toString(),
      1.0f / TilePixels,
      true,
      Vec2F())};

  setPrice(int(price() * Root::singleton().assets()->json("/items/defaultParameters.config:blueprintPriceFactor").toFloat()));
}

ItemPtr BlueprintItem::clone() const {
  return std::make_shared<BlueprintItem>(*this);
}

List<Drawable> BlueprintItem::drawables() const {
  return m_inHandDrawable;
}

void BlueprintItem::fireTriggered() {
  if (count())
    if (auto player = as<Player>(owner()))
      if (player->addBlueprint(m_recipe, true))
        setCount(count() - 1);
}

List<Drawable> BlueprintItem::iconDrawables() const {
  List<Drawable> result;
  result.append(m_recipeIconUnderlay);
  result.appendAll(Item::iconDrawables());
  return result;
}

List<Drawable> BlueprintItem::dropDrawables() const {
  return m_inHandDrawable;
}

}
