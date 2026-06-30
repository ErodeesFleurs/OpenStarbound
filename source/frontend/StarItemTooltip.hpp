#pragma once

#include "StarAssets.hpp"
#include "StarString.hpp"
#include "StarStatusTypes.hpp"

namespace Star {

class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;
class Item;
using ItemPtr = SharedPtr<Item>;
class Widget;
using WidgetPtr = SharedPtr<Widget>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;
class Augment;
class Pane;
using PanePtr = SharedPtr<Pane>;
class Player;
using PlayerPtr = SharedPtr<Player>;
class GuiContext;

namespace ItemTooltipBuilder {
  struct Services {
    Services(AssetsConstPtr assets, ObjectDatabaseConstPtr objectDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, GuiContext& guiContext)
      : assets(std::move(assets)), objectDatabase(std::move(objectDatabase)), statusEffectDatabase(std::move(statusEffectDatabase)), guiContext(guiContext) {}

    AssetsConstPtr assets;
    ObjectDatabaseConstPtr objectDatabase;
    StatusEffectDatabaseConstPtr statusEffectDatabase;
    GuiContext& guiContext;
  };

  PanePtr buildItemTooltip(ItemPtr const& item, PlayerPtr const& viewer, Services services);

  void buildItemDescription(WidgetPtr const& container, ItemPtr const& item, Services services);
  void buildItemDescriptionInner(
      WidgetPtr const& container, ItemPtr const& item, String const& tooltipKind, String& title, String& subtitle, PlayerPtr const& viewer, Services services);

  void describePersistentEffect(ListWidgetPtr const& container, PersistentStatusEffect const& effect, Services services);
};

}
