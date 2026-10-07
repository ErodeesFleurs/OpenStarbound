module;
#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarJson.hpp"
#include "StarParametricFunction.hpp"
#include "StarMultiTable.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarLogging.hpp"
#include "StarJsonExtra.hpp"

import star.status_types;
#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
#include "StarRoot.hpp"
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.widget_parsing;
import star.gui_reader;
import star.pane;
import star.list_widget;
import star.label_widget;
#include "StarRoot.hpp"
import star.image_widget;
import star.progress_widget;
import star.animation;
import star.item_slot_widget;
import star.previewable_item;
#include "StarArmors.hpp"
import star.damage_types;
import star.world_geometry;
import star.damage;
import star.light_source;
import star.entity;
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
#include "StarObject.hpp"
#include "StarAssets.hpp"

export module star.item_tooltip;
import star.stored_functions;

import star.object_item;
import star.status_effect_database;
import star.object_database;


export namespace Star {
namespace ItemTooltipBuilder {
  PanePtr buildItemTooltip(ItemPtr const& item, PlayerPtr const& viewer = {});

  void buildItemDescription(WidgetPtr const& container, ItemPtr const& item);
  void buildItemDescriptionInner(
      WidgetPtr const& container, ItemPtr const& item, String const& tooltipKind, String& title, String& subtitle, PlayerPtr const& viewer = {});

  void describePersistentEffect(ListWidgetPtr const& container, PersistentStatusEffect const& effect);
};
}

namespace Star {

PanePtr ItemTooltipBuilder::buildItemTooltip(ItemPtr const& item, PlayerPtr const& viewer) {
  if (!item) {
    return {};
  } else {
    PanePtr tooltip = make_shared<Pane>();
    tooltip->removeAllChildren();

    String title;
    String subTitle;

    String tooltipKind = item->tooltipKind();

    if (tooltipKind.empty())
      tooltipKind = "base";
    if (!tooltipKind.endsWith(".tooltip"))
      tooltipKind = "/interface/tooltips/" + tooltipKind + ".tooltip";

    buildItemDescriptionInner(tooltip, item, tooltipKind, title, subTitle, viewer);

    auto titleIcon = make_shared<ItemSlotWidget>(item, "/interface/inventory/portrait.png");
    titleIcon->setBackingImageAffinity(true, true);
    titleIcon->showRarity(false);
    tooltip->setTitle(titleIcon, title, subTitle);

    return tooltip;
  }
}

void ItemTooltipBuilder::buildItemDescription(WidgetPtr const& container, ItemPtr const& item) {
  String tooltipKind = item->tooltipKind();

  if (tooltipKind.empty())
    tooltipKind = "base";
  if (!tooltipKind.endsWith(".itemdescription"))
    tooltipKind = "/interface/itemdescriptions/" + tooltipKind + ".itemdescription";

  String title;
  String subTitle;
  buildItemDescriptionInner(container, item, tooltipKind, title, subTitle);
}

String categoryDisplayName(String const& category) {
  Json categories = Root::singleton().assets()->json("/items/categories.config:labels");
  return categories.getString(category, category);
}

void ItemTooltipBuilder::buildItemDescriptionInner(
    WidgetPtr const& container, ItemPtr const& item, String const& tooltipKind, String& title, String& subTitle, PlayerPtr const& viewer) {
  GuiReader reader;
  auto& root = Root::singleton();
  auto assets = root.assets();
  title = item->friendlyName();
  subTitle = categoryDisplayName(item->category());
  String description = item->description();

  reader.construct(assets->json(tooltipKind), container.get());

  if (container->containsChild("icon"))
    container->fetchChild<ItemSlotWidget>("icon")->setItem(item);

  container->setLabel("nameLabel", item->name());
  container->setLabel("countLabel", toString(item->count()));

  container->setLabel("rarityLabel", RarityNames.getRight(item->rarity()).titleCase());

  if (item->twoHanded())
    container->setLabel("handednessLabel", "2-Handed");
  else
    container->setLabel("handednessLabel", "1-Handed");

  container->setLabel("countLabel", toString(item->instanceValue("fuelAmount", 0).toUInt() * item->count()));
  container->setLabel("priceLabel", toString((int)item->price()));

  if (auto objectItem = as<ObjectItem>(item)) {
    try {
      auto object = Root::singleton().objectDatabase()->createObject(objectItem->objectName(), objectItem->objectParameters());

      if (container->containsChild("objectImage")) {
        auto drawables = object->cursorHintDrawables();
        container->fetchChild<ImageWidget>("objectImage")->setDrawables(drawables);
      }

      if (objectItem->tooltipKind() == "container")
        container->setLabel("slotCountLabel", strf("Holds {} Items", objectItem->instanceValue("slotCount")));

      title = object->shortDescription();
      subTitle = categoryDisplayName(object->category());
      description = object->description();
    } catch (StarException const& e) {
      Logger::error("Failed to instantiate object for object item tooltip. {}", outputException(e, false));
    }
  } else {
    if (container->containsChild("objectImage")) {
      auto objectImage = container->fetchChild<ImageWidget>("objectImage");
      if (auto previewable = as<PreviewableItem>(item)) {
        if (is<ArmorItem>(previewable)) {
          objectImage->disableScissoring();
          PlayerPtr armorViewer;
          if (assets->json("/interface.config:tooltip.previewArmorWith").toString() == "player")
            armorViewer = viewer;
          objectImage->setDrawables(previewable->preview(armorViewer));
        } else {
          objectImage->setDrawables(previewable->preview(viewer));
        }
      } else {
        auto drawables = item->iconDrawables();
        objectImage->setDrawables(drawables);
      }
    }
  }

  auto tooltipFields = item->instanceValue("tooltipFields", JsonObject());
  for (auto const& pair : tooltipFields.iterateObject()) {
    if (pair.first.equalsIgnoreCase("subtitle"))
      subTitle = pair.second.toString();
    if (pair.first.endsWith("Label"))
      container->setLabel(pair.first, pair.second.type() == Json::Type::String ? pair.second.toString() : toString(pair.second));
    if (pair.first.endsWith("Image") && container->containsChild(pair.first)) {
      if (pair.second.isType(Json::Type::String))
        container->fetchChild<ImageWidget>(pair.first)->setImage(pair.second.toString());
      else
        container->fetchChild<ImageWidget>(pair.first)->setDrawables(pair.second.toArray().transformed(construct<Drawable>()));
    }
  }

  if (auto fireable = as<FireableItem>(item)) {
    container->setLabel("cooldownTimeLabel", strf("{:.2f}", fireable->cooldownTime()));
    container->setLabel("windupTimeLabel", strf("{:.2f}", fireable->windupTime()));
    container->setLabel("speedLabel", strf("{:.2f}", 1.0f / (fireable->cooldownTime() + fireable->windupTime())));
  }

  if (container->containsChild("largeImage")) {
    container->fetchChild<ImageWidget>("largeImage")->setImage(item->largeImage());
  }

  container->setLabel("descriptionLabel", description);
  container->setLabel("friendlyNameLabel", title);

  if (container->containsChild("statusList")) {
    auto statusList = container->fetchChild<ListWidget>("statusList");
    if (auto statusEffects = as<StatusEffectItem>(item)) {
      for (auto effect : statusEffects->statusEffects())
        describePersistentEffect(statusList, effect);
    }
  }

  if (item->instanceValue("acceptsAugmentType", false)) {
    if (auto augmentLabel = container->fetchChild<LabelWidget>("augmentNameLabel")) {
      if (auto currentAugment = item->instanceValue("currentAugment")) {
        container->setLabel("augmentNameLabel", currentAugment.getString("displayName", "???"));
        if (auto augmentIcon = container->fetchChild<ImageWidget>("augmentIconImage"))
          augmentIcon->setImage(currentAugment.getString("displayIcon", ""));
        augmentLabel->setColor(Color::White);
      } else {
        container->setLabel("augmentNameLabel", "NO AUGMENT INSERTED");
        if (auto augmentIcon = container->fetchChild<ImageWidget>("augmentIconImage"))
          augmentIcon->setImage("");
        augmentLabel->setColor(Color::Gray);
      }
    }
  }

  container->setLabel("title", title);
  container->setLabel("subTitle", subTitle);
  if (container->containsChild("titleIcon")) {
    auto titleIcon = container->fetchChild<ItemSlotWidget>("titleIcon");
    titleIcon->setItem(item);
  }
}

void ItemTooltipBuilder::describePersistentEffect(
    ListWidgetPtr const& container, PersistentStatusEffect const& effect) {
  if (auto uniqueStatusEffect = effect.ptr<UniqueStatusEffect>()) {
    auto statusEffectDatabase = Root::singleton().statusEffectDatabase();
    auto effectConfig = statusEffectDatabase->uniqueEffectConfig(*uniqueStatusEffect);
    if (effectConfig.icon) {
      auto listItem = container->addItem();
      listItem->setLabel("statusLabel", effectConfig.label);
      listItem->fetchChild<ImageWidget>("statusImage")->setImage(*effectConfig.icon);
    }
  } else if (auto modifierEffect = effect.ptr<StatModifier>()) {
    auto statsConfig = Root::singleton().assets()->json("/interface/stats/stats.config");
    if (auto baseMultiplier = modifierEffect->ptr<StatBaseMultiplier>()) {
      if (statsConfig.contains(baseMultiplier->statName)) {
        auto listItem = container->addItem();
        listItem->fetchChild<ImageWidget>("statusImage")
            ->setImage(statsConfig.get(baseMultiplier->statName).getString("icon"));
        listItem->setLabel("statusLabel", strf("{:.1f}%", (baseMultiplier->baseMultiplier - 1) * 100));
      }
    } else if (auto valueModifier = modifierEffect->ptr<StatValueModifier>()) {
      if (statsConfig.contains(valueModifier->statName)) {
        auto listItem = container->addItem();
        listItem->fetchChild<ImageWidget>("statusImage")
            ->setImage(statsConfig.get(valueModifier->statName).getString("icon"));
        listItem->setLabel("statusLabel", strf("{}{:.2f}", valueModifier->value < 0 ? "-" : "", valueModifier->value));
      }
    } else if (auto effectiveMultiplier = modifierEffect->ptr<StatEffectiveMultiplier>()) {
      if (statsConfig.contains(effectiveMultiplier->statName)) {
        auto listItem = container->addItem();
        listItem->fetchChild<ImageWidget>("statusImage")
            ->setImage(statsConfig.get(effectiveMultiplier->statName).getString("icon"));
        listItem->setLabel("statusLabel", strf("{:.1f}%", (effectiveMultiplier->effectiveMultiplier - 1) * 100));
      }
    }
  }
}

}
