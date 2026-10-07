module;
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRpcPromise.hpp"



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

STAR_CLASS(MainInterface);
STAR_CLASS(UniverseClient);
STAR_CLASS(Player);
STAR_CLASS(Item);
STAR_CLASS(ItemSlotWidget);
STAR_CLASS(ItemGridWidget);
STAR_CLASS(ImageWidget);
STAR_CLASS(Widget);
STAR_CLASS(InventoryPane);

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
