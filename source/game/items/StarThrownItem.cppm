module;
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarAssetPath.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarSet.hpp"
#include "StarOrderedMap.hpp"
#include "StarLua.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"


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
#include "StarLuaRoot.hpp"
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
  ThrownItem(Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  ItemPtr clone() const override;

  List<Drawable> drawables() const override;
  List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

protected:
  void fireTriggered() override;

private:
  String m_projectileType;
  Json m_projectileConfig;
  size_t m_ammoUsage;
  List<Drawable> m_drawables;
};

}

namespace Star {

ThrownItem::ThrownItem(Json const& config, String const& directory, Json const& itemParameters)
  : Item(config, directory, itemParameters), SwingableItem(config) {
  m_projectileType = instanceValue("projectileType").toString();
  m_projectileConfig = instanceValue("projectileConfig", {});
  m_ammoUsage = instanceValue("ammoUsage", 1).toUInt();

  auto image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_drawables = {Drawable::makeImage(image, 1.0f / TilePixels, true, Vec2F())};
}

ItemPtr ThrownItem::clone() const {
  return make_shared<ThrownItem>(*this);
}

List<Drawable> ThrownItem::drawables() const {
  return m_drawables;
}

List<Drawable> ThrownItem::preview(PlayerPtr const&) const {
  return iconDrawables();
}

void ThrownItem::fireTriggered() {
  auto& root = Root::singleton();

  if (initialized()) {
    Vec2F direction = world()->geometry().diff(owner()->aimPosition(), owner()->position()).normalized();
    Vec2F firePosition = owner()->position() + ownerFirePosition();
    if (world()->lineTileCollision(owner()->position(), firePosition))
      return;

    if (consume(m_ammoUsage)) {
      auto projectile = root.projectileDatabase()->createProjectile(m_projectileType, m_projectileConfig);
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
