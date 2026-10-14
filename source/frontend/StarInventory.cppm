module;
import std;
import star.json;
import star.id_map;
import star.game_types;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;
#include "StarStrongTypedef.hpp"
import star.rpc_promise;



import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.container_entity;





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
import star.pane;
import star.inventory_types;
import star.item_descriptor;
import star.container_interactor;
import star.game_timers;

namespace Star {

class MainInterface;
using MainInterfacePtr = std::shared_ptr<MainInterface>;
using MainInterfaceConstPtr = std::shared_ptr<MainInterface const>;
using MainInterfaceWeakPtr = std::weak_ptr<MainInterface>;
using MainInterfaceConstWeakPtr = std::weak_ptr<MainInterface const>;
using MainInterfaceUPtr = std::unique_ptr<MainInterface>;
using MainInterfaceConstUPtr = std::unique_ptr<MainInterface const>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<Item const>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<Item const>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<Item const>;
class ItemSlotWidget;
using ItemSlotWidgetPtr = std::shared_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstPtr = std::shared_ptr<ItemSlotWidget const>;
using ItemSlotWidgetWeakPtr = std::weak_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstWeakPtr = std::weak_ptr<ItemSlotWidget const>;
using ItemSlotWidgetUPtr = std::unique_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstUPtr = std::unique_ptr<ItemSlotWidget const>;
class ItemGridWidget;
using ItemGridWidgetPtr = std::shared_ptr<ItemGridWidget>;
using ItemGridWidgetConstPtr = std::shared_ptr<ItemGridWidget const>;
using ItemGridWidgetWeakPtr = std::weak_ptr<ItemGridWidget>;
using ItemGridWidgetConstWeakPtr = std::weak_ptr<ItemGridWidget const>;
using ItemGridWidgetUPtr = std::unique_ptr<ItemGridWidget>;
using ItemGridWidgetConstUPtr = std::unique_ptr<ItemGridWidget const>;
class ImageWidget;
using ImageWidgetPtr = std::shared_ptr<ImageWidget>;
using ImageWidgetConstPtr = std::shared_ptr<ImageWidget const>;
using ImageWidgetWeakPtr = std::weak_ptr<ImageWidget>;
using ImageWidgetConstWeakPtr = std::weak_ptr<ImageWidget const>;
using ImageWidgetUPtr = std::unique_ptr<ImageWidget>;
using ImageWidgetConstUPtr = std::unique_ptr<ImageWidget const>;
class Widget;
using WidgetPtr = std::shared_ptr<Widget>;
using WidgetConstPtr = std::shared_ptr<Widget const>;
using WidgetWeakPtr = std::weak_ptr<Widget>;
using WidgetConstWeakPtr = std::weak_ptr<Widget const>;
using WidgetUPtr = std::unique_ptr<Widget>;
using WidgetConstUPtr = std::unique_ptr<Widget const>;
class InventoryPane;
using InventoryPanePtr = std::shared_ptr<InventoryPane>;
using InventoryPaneConstPtr = std::shared_ptr<InventoryPane const>;
using InventoryPaneWeakPtr = std::weak_ptr<InventoryPane>;
using InventoryPaneConstWeakPtr = std::weak_ptr<InventoryPane const>;
using InventoryPaneUPtr = std::unique_ptr<InventoryPane>;
using InventoryPaneConstUPtr = std::unique_ptr<InventoryPane const>;

class InventoryPane : public Pane {
public:
  InventoryPane(MainInterface* parent, PlayerPtr player, ContainerInteractorPtr containerInteractor);

  void displayed() override;
  PanePtr createTooltip(Vec2I const& screenPosition) override;
  bool sendEvent(InputEvent const& event) override;

  bool giveContainerResult(ContainerResult result);

  // update only item grids, to see if they have had their slots changed
  // this is a little hacky and should probably be checked in the player inventory instead
  void updateItems();
  bool containsNewItems() const;
  void clearChangedSlots();

protected:
  virtual void update(float dt) override;

  void selectTab(String const& selected);

private:
  MainInterface* m_parent;
  PlayerPtr m_player;
  ContainerInteractorPtr m_containerInteractor;

  bool m_alwaysDisplayCosmetics;
  bool m_displayingCosmetics;
  bool m_expectingSwap;
  InventorySlot m_containerSource;

  GameTimer m_trashBurn;
  ItemSlotWidgetPtr m_trashSlot;

  Map<String, ItemGridWidgetPtr> m_itemGrids;
  Map<String, String> m_tabButtonData;

  Map<String, WidgetPtr> m_newItemMarkers;
  String m_selectedTab;

  StringList m_pickUpSounds;
  StringList m_putDownSounds;
  StringList m_someUpSounds;
  StringList m_someDownSounds;
  Maybe<ItemDescriptor> m_currentSwapSlotItem;

  List<ImageWidgetPtr> m_disabledTechOverlays;
};

}

export module star.inventory;

export namespace Star {
  using ::Star::MainInterface;
  using ::Star::MainInterfacePtr;
  using ::Star::MainInterfaceConstPtr;
  using ::Star::MainInterfaceWeakPtr;
  using ::Star::MainInterfaceConstWeakPtr;
  using ::Star::MainInterfaceUPtr;
  using ::Star::MainInterfaceConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::ItemSlotWidget;
  using ::Star::ItemSlotWidgetPtr;
  using ::Star::ItemSlotWidgetConstPtr;
  using ::Star::ItemSlotWidgetWeakPtr;
  using ::Star::ItemSlotWidgetConstWeakPtr;
  using ::Star::ItemSlotWidgetUPtr;
  using ::Star::ItemSlotWidgetConstUPtr;
  using ::Star::ItemGridWidget;
  using ::Star::ItemGridWidgetPtr;
  using ::Star::ItemGridWidgetConstPtr;
  using ::Star::ItemGridWidgetWeakPtr;
  using ::Star::ItemGridWidgetConstWeakPtr;
  using ::Star::ItemGridWidgetUPtr;
  using ::Star::ItemGridWidgetConstUPtr;
  using ::Star::ImageWidget;
  using ::Star::ImageWidgetPtr;
  using ::Star::ImageWidgetConstPtr;
  using ::Star::ImageWidgetWeakPtr;
  using ::Star::ImageWidgetConstWeakPtr;
  using ::Star::ImageWidgetUPtr;
  using ::Star::ImageWidgetConstUPtr;
  using ::Star::Widget;
  using ::Star::WidgetPtr;
  using ::Star::WidgetConstPtr;
  using ::Star::WidgetWeakPtr;
  using ::Star::WidgetConstWeakPtr;
  using ::Star::WidgetUPtr;
  using ::Star::WidgetConstUPtr;
  using ::Star::InventoryPane;
  using ::Star::InventoryPanePtr;
  using ::Star::InventoryPaneConstPtr;
  using ::Star::InventoryPaneWeakPtr;
  using ::Star::InventoryPaneConstWeakPtr;
  using ::Star::InventoryPaneUPtr;
  using ::Star::InventoryPaneConstUPtr;
}
