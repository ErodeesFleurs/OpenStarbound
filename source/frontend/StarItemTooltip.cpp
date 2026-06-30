#include "StarItemTooltip.hpp"
#include "StarAlgorithm.hpp"
#include "StarGuiReader.hpp"
#include "StarPane.hpp"
#include "StarListWidget.hpp"
#include "StarLabelWidget.hpp"
#include "StarException.hpp"
#include "StarStoredFunctions.hpp"
#include "StarObjectItem.hpp"
#include "StarImageWidget.hpp"
#include "StarItemSlotWidget.hpp"
#include "StarPreviewableItem.hpp"
#include "StarArmors.hpp"
#include "StarFireableItem.hpp"
#include "StarStatusEffectItem.hpp"
#include "StarObject.hpp"
#include "StarLogging.hpp"
#include "StarAssets.hpp"
#include "StarObjectDatabase.hpp"
#include "StarStatusEffectDatabase.hpp"
#include "StarJsonExtra.hpp"

namespace Star {

namespace {
AssetsConstPtr tooltipAssets(ItemTooltipBuilder::Services const& services) {
  return requireServiceValueAs<StarException>(services.assets, "ItemTooltipBuilder", "assets");
}

ObjectDatabaseConstPtr tooltipObjectDatabase(ItemTooltipBuilder::Services const& services) {
  return requireServiceValueAs<StarException>(services.objectDatabase, "ItemTooltipBuilder", "object database");
}

StatusEffectDatabaseConstPtr tooltipStatusEffectDatabase(ItemTooltipBuilder::Services const& services) {
  return requireServiceValueAs<StarException>(services.statusEffectDatabase, "ItemTooltipBuilder", "status effect database");
}

String categoryDisplayName(String const& category, ItemTooltipBuilder::Services const& services) {
  Json categories = tooltipAssets(services)->json("/items/categories.config:labels");
  return categories.getString(category, category);
}
}

PanePtr ItemTooltipBuilder::buildItemTooltip(ItemPtr const& item, PlayerPtr const& viewer, Services services) {
  if (!item) {
    return {};
  } else {
    PanePtr tooltip = make_shared<Pane>(services.guiContext);
    tooltip->removeAllChildren();

    String title;
    String subTitle;

    String tooltipKind = item->tooltipKind();

    if (tooltipKind.empty())
      tooltipKind = "base";
    if (!tooltipKind.endsWith(".tooltip"))
      tooltipKind = "/interface/tooltips/" + tooltipKind + ".tooltip";

    buildItemDescriptionInner(tooltip.get(), item, tooltipKind, title, subTitle, viewer, services);

    auto titleIcon = make_shared<ItemSlotWidget>(services.guiContext, item, "/interface/inventory/portrait.png");
    titleIcon->setBackingImageAffinity(true, true);
    titleIcon->showRarity(false);
    tooltip->setTitle(titleIcon, title, subTitle);

    return tooltip;
  }
}

void ItemTooltipBuilder::buildItemDescription(Widget* container, ItemPtr const& item, Services services) {
  String tooltipKind = item->tooltipKind();

  if (tooltipKind.empty())
    tooltipKind = "base";
  if (!tooltipKind.endsWith(".itemdescription"))
    tooltipKind = "/interface/itemdescriptions/" + tooltipKind + ".itemdescription";

  String title;
  String subTitle;
  buildItemDescriptionInner(container, item, tooltipKind, title, subTitle, {}, services);
}

void ItemTooltipBuilder::buildItemDescriptionInner(
    Widget* container, ItemPtr const& item, String const& tooltipKind, String& title, String& subTitle, PlayerPtr const& viewer, Services services) {
  GuiReader reader(services.guiContext);
  auto assets = tooltipAssets(services);
  title = item->friendlyName();
  subTitle = categoryDisplayName(item->category(), services);
  String description = item->description();

  reader.construct(assets->json(tooltipKind), container);

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
  container->setLabel("priceLabel", toString(static_cast<int>(item->price())));

  if (auto objectItem = as<ObjectItem>(item)) {
    try {
      auto object = tooltipObjectDatabase(services)->createObject(objectItem->objectName(), objectItem->objectParameters());

      if (container->containsChild("objectImage")) {
        auto drawables = object->cursorHintDrawables();
        container->fetchChild<ImageWidget>("objectImage")->setDrawables(drawables);
      }

      if (objectItem->tooltipKind() == "container")
        container->setLabel("slotCountLabel", strf("Holds {} Items", objectItem->instanceValue("slotCount")));

      title = object->shortDescription();
      subTitle = categoryDisplayName(object->category(), services);
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
  for (auto const& [fieldName, fieldValue] : tooltipFields.iterateObject()) {
    if (fieldName.equalsIgnoreCase("subtitle"))
      subTitle = fieldValue.toString();
    if (fieldName.endsWith("Label"))
      container->setLabel(fieldName, fieldValue.type() == Json::Type::String ? fieldValue.toString() : toString(fieldValue));
    if (fieldName.endsWith("Image") && container->containsChild(fieldName)) {
      if (fieldValue.isType(Json::Type::String))
        container->fetchChild<ImageWidget>(fieldName)->setImage(fieldValue.toString());
      else
        container->fetchChild<ImageWidget>(fieldName)->setDrawables(fieldValue.toArray().transformed(construct<Drawable>()));
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
        describePersistentEffect(statusList, effect, services);
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
    WidgetRef<ListWidget> container, PersistentStatusEffect const& effect, Services services) {
  if (auto uniqueStatusEffect = effect.ptr<UniqueStatusEffect>()) {
    auto statusEffectDatabase = tooltipStatusEffectDatabase(services);
    auto effectConfig = statusEffectDatabase->uniqueEffectConfig(*uniqueStatusEffect);
    if (effectConfig.icon) {
      auto listItem = container->addItem();
      listItem->setLabel("statusLabel", effectConfig.label);
      listItem->fetchChild<ImageWidget>("statusImage")->setImage(*effectConfig.icon);
    }
  } else if (auto modifierEffect = effect.ptr<StatModifier>()) {
    auto statsConfig = tooltipAssets(services)->json("/interface/stats/stats.config");
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
