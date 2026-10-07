module;
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"
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
#include "StarString.hpp"

#include "StarItem.hpp"
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
import star.drawable;
import star.previewable_item;

#include "StarPlayer.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"

export module star.unlock_item;

export namespace Star {

STAR_CLASS(UnlockItem);

class UnlockItem : public Item, public SwingableItem, public PreviewableItem {
public:
  UnlockItem(Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  ItemPtr clone() const override;

  List<Drawable> drawables() const override;
  List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

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

UnlockItem::UnlockItem(Json const& config, String const& directory, Json const& itemParameters)
  : Item(config, directory, itemParameters), SwingableItem(config) {
  m_tierRecipesUnlock = instanceValue("tierRecipesUnlock").optString();
  m_shipUpgrade = instanceValue("shipUpgrade").optUInt();
  m_unlockMessage = instanceValue("unlockMessage").optString().value();
  auto image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_drawables = {Drawable::makeImage(image, 1.0f / TilePixels, true, Vec2F())};
}

ItemPtr UnlockItem::clone() const {
  return make_shared<UnlockItem>(*this);
}

List<Drawable> UnlockItem::drawables() const {
  return m_drawables;
}

List<Drawable> UnlockItem::preview(PlayerPtr const&) const {
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
      auto playerConfig = Root::singleton().assets()->json("/player.config");

      List<ItemDescriptor> blueprints;
      for (Json v : playerConfig.get("defaultBlueprints", JsonObject()).getArray(*m_tierRecipesUnlock, JsonArray()))
        blueprints.append(ItemDescriptor(v));

      auto speciesConfig = Root::singleton().assets()->json(strf("/species/{}.species", player->species()));
      for (Json v : speciesConfig.get("defaultBlueprints", JsonObject()).getArray(*m_tierRecipesUnlock, JsonArray()))
        blueprints.append(ItemDescriptor(v));

      for (auto b : blueprints)
        player->addBlueprint(b);
    }
  }
}

}
