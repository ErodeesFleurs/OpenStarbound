#pragma once

#include "StarString.hpp"
#include "StarStatusTypes.hpp"

namespace Star {

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
  PanePtr buildItemTooltip(ItemPtr const& item, PlayerPtr const& viewer = {});

  void buildItemDescription(WidgetPtr const& container, ItemPtr const& item);
  void buildItemDescriptionInner(
      WidgetPtr const& container, ItemPtr const& item, String const& tooltipKind, String& title, String& subtitle, PlayerPtr const& viewer = {});

  void describePersistentEffect(ListWidgetPtr const& container, PersistentStatusEffect const& effect);
};

}
