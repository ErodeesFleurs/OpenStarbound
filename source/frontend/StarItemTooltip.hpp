#pragma once

#include "StarIAssets.hpp"
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

namespace ItemTooltipBuilder {
  struct Services {
    Services(IAssetsConstPtr assets, ObjectDatabaseConstPtr objectDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase)
      : assets(std::move(assets)), objectDatabase(std::move(objectDatabase)), statusEffectDatabase(std::move(statusEffectDatabase)) {}

    IAssetsConstPtr assets;
    ObjectDatabaseConstPtr objectDatabase;
    StatusEffectDatabaseConstPtr statusEffectDatabase;
  };

  PanePtr buildItemTooltip(ItemPtr const& item, PlayerPtr const& viewer, Services services);

  void buildItemDescription(WidgetPtr const& container, ItemPtr const& item, Services services);
  void buildItemDescriptionInner(
      WidgetPtr const& container, ItemPtr const& item, String const& tooltipKind, String& title, String& subtitle, PlayerPtr const& viewer, Services services);

  void describePersistentEffect(ListWidgetPtr const& container, PersistentStatusEffect const& effect, Services services);
};

}
