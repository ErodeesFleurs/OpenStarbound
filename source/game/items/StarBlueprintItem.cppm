module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarOrderedMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"
#include "StarArray.hpp"
#include "StarOrderedSet.hpp"
#include "StarAudio.hpp"
#include "StarMap.hpp"
#include "StarEither.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"


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

STAR_CLASS(BlueprintItem);

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
  return make_shared<BlueprintItem>(*this);
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
