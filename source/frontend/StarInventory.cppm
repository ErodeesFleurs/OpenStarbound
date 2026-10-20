module;
import std;
import star.type_identity;
import star.json;
import star.id_map;
import star.game_types;
import star.vector;
import star.casting;
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
import star.strong_typedef;
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
class UniverseClient;
class Player;
class Item;
class ItemSlotWidget;
class ItemGridWidget;
class ImageWidget;
class Widget;

class InventoryPane : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<InventoryPane, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<InventoryPane>();
  }

  InventoryPane(GuiContext& guiContext, MainInterface* parent, std::shared_ptr<Player> player, std::shared_ptr<ContainerInteractor> containerInteractor);

  void displayed() override;
  std::shared_ptr<Pane> createTooltip(Vec2I const& screenPosition) override;
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
  std::shared_ptr<Player> m_player;
  std::shared_ptr<ContainerInteractor> m_containerInteractor;

  bool m_alwaysDisplayCosmetics;
  bool m_displayingCosmetics;
  bool m_expectingSwap;
  InventorySlot m_containerSource;

  GameTimer m_trashBurn;
  std::shared_ptr<ItemSlotWidget> m_trashSlot;

  Map<String, std::shared_ptr<ItemGridWidget>> m_itemGrids;
  Map<String, String> m_tabButtonData;

  Map<String, std::shared_ptr<Widget>> m_newItemMarkers;
  String m_selectedTab;

  StringList m_pickUpSounds;
  StringList m_putDownSounds;
  StringList m_someUpSounds;
  StringList m_someDownSounds;
  Maybe<ItemDescriptor> m_currentSwapSlotItem;

  List<std::shared_ptr<ImageWidget>> m_disabledTechOverlays;
};

}

export module star.inventory;

export namespace Star {
  using ::Star::MainInterface;
  using ::Star::UniverseClient;
  using ::Star::Player;
  using ::Star::Item;
  using ::Star::ItemSlotWidget;
  using ::Star::ItemGridWidget;
  using ::Star::ImageWidget;
  using ::Star::Widget;
  using ::Star::InventoryPane;
}
