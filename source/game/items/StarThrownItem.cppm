module;
import std;
import star.type_identity;
import star.json;
import star.id_map;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.casting;
import star.vector;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.variant;
import star.rpc_promise;
import star.a_star;
import star.set;
import star.ordered_map;
import star.lua;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;


import star.item;
import star.drawable;
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
import star.lua_root;
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
import star.previewable_item;

import star.scripted_entity;
import star.status_effect_entity;
import star.effect_emitter;
import star.projectile;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

export module star.thrown_item;
import star.projectile_database;

export namespace Star {

class ThrownItem : public Item, public SwingableItem, public PreviewableItem {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<ThrownItem, Item, SwingableItem, PreviewableItem>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<ThrownItem>();
  }

  ThrownItem(Root& root, Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  List<Drawable> drawables() const override;
  List<Drawable> preview(TileDrawer*, std::shared_ptr<Player> const& viewer = {}) const override;

protected:
  void fireTriggered() override;

private:
  String m_projectileType;
  Json m_projectileConfig;
  std::size_t m_ammoUsage;
  List<Drawable> m_drawables;
};

}

namespace Star {

ThrownItem::ThrownItem(Root& root, Json const& config, String const& directory, Json const& itemParameters)
  : Item(root, config, directory, itemParameters), SwingableItem(config) {
  m_projectileType = instanceValue("projectileType").toString();
  m_projectileConfig = instanceValue("projectileConfig", {});
  m_ammoUsage = instanceValue("ammoUsage", 1).toUInt();

  auto image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_drawables = {Drawable::makeImage(*root.imageMetadataDatabase(), image, 1.0f / TilePixels, true, Vec2F())};
}

std::shared_ptr<Item> ThrownItem::clone() const {
  return std::make_shared<ThrownItem>(*this);
}

List<Drawable> ThrownItem::drawables() const {
  return m_drawables;
}

List<Drawable> ThrownItem::preview(TileDrawer*, std::shared_ptr<Player> const&) const {
  return iconDrawables();
}

void ThrownItem::fireTriggered() {
  if (initialized()) {
    Vec2F direction = world()->geometry().diff(owner()->aimPosition(), owner()->position()).normalized();
    Vec2F firePosition = owner()->position() + ownerFirePosition();
    if (world()->lineTileCollision(owner()->position(), firePosition))
      return;

    if (consume(m_ammoUsage)) {
      auto projectile = root().projectileDatabase()->createProjectile(m_projectileType, m_projectileConfig);
      projectile->setInitialPosition(firePosition);
      projectile->setInitialDirection(direction);
      projectile->setSourceEntity(owner()->entityId(), false);
      projectile->setPowerMultiplier(owner()->powerMultiplier());
      world()->addEntity(projectile);
    }

    FireableItem::fireTriggered();
  } else {
    throw ItemException("Thrown item not init'd properly, or user not recognized as Tool User.");
  }
}

}
