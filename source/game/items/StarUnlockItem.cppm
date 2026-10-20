module;
import star.json;
import star.id_map;
import star.variant;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
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
import star.string;
import star.set;
import star.ordered_set;
import star.audio;
import star.map;
import star.either;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;


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
import star.lua_root;
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
import star.lua_components;
import star.fireable_item;
import star.swingable_item;
import star.drawable;
import star.previewable_item;

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
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

export module star.unlock_item;

export namespace Star {

class UnlockItem : public Item, public SwingableItem, public PreviewableItem {
public:
  UnlockItem(Root& root, Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  List<Drawable> drawables() const override;
  List<Drawable> preview(TileDrawer*, std::shared_ptr<Player> const& viewer = {}) const override;

protected:
  void fireTriggered() override;

private:
  Maybe<String> m_sectorUnlock;
  Maybe<String> m_tierRecipesUnlock;
  Maybe<unsigned> m_shipUpgrade;
  String m_unlockMessage;
  List<Drawable> m_drawables;
};

}

namespace Star {

UnlockItem::UnlockItem(Root& root, Json const& config, String const& directory, Json const& itemParameters)
  : Item(root, config, directory, itemParameters), SwingableItem(config) {
  m_tierRecipesUnlock = instanceValue("tierRecipesUnlock").optString();
  m_shipUpgrade = instanceValue("shipUpgrade").optUInt();
  m_unlockMessage = instanceValue("unlockMessage").optString().value();
  auto image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_drawables = {Drawable::makeImage(*root.imageMetadataDatabase(), image, 1.0f / TilePixels, true, Vec2F())};
}

std::shared_ptr<Item> UnlockItem::clone() const {
  return std::make_shared<UnlockItem>(*this);
}

List<Drawable> UnlockItem::drawables() const {
  return m_drawables;
}

List<Drawable> UnlockItem::preview(TileDrawer*, std::shared_ptr<Player> const&) const {
  return iconDrawables();
}

void UnlockItem::fireTriggered() {
  if (!initialized())
    throw ItemException("Item not init'd properly, or user not recognized as Tool User.");

  // Only the player can use an unlock item, for any other entity it should do
  // nothing.
  if (auto player = as<Player>(owner())) {
    if (instanceValue("consume", true).toBool() && !consume(1))
      return;

    if (auto clientContext = player->clientContext()) {
      if (m_shipUpgrade)
        player->applyShipUpgrades(JsonObject{ {"shipLevel", *m_shipUpgrade} });
    }

    if (!m_unlockMessage.empty()) {
      JsonObject message;
      message["message"] = m_unlockMessage;
      owner()->interact(InteractAction(InteractActionType::ShowPopup, owner()->entityId(), message));
    }

    if (m_tierRecipesUnlock) {
      auto playerConfig = root().assets()->json("/player.config");

      List<ItemDescriptor> blueprints;
      for (Json v : playerConfig.get("defaultBlueprints", JsonObject()).getArray(*m_tierRecipesUnlock, JsonArray()))
        blueprints.append(ItemDescriptor(v));

      auto speciesConfig = root().assets()->json(strf("/species/{}.species", player->species()));
      for (Json v : speciesConfig.get("defaultBlueprints", JsonObject()).getArray(*m_tierRecipesUnlock, JsonArray()))
        blueprints.append(ItemDescriptor(v));

      for (auto b : blueprints)
        player->addBlueprint(b);
    }
  }
}

}
