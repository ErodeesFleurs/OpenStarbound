module;
import std;
import star.json;
import star.id_map;
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
import star.game_types;
import star.set;
import star.audio;
import star.map;
#include "StarStrongTypedef.hpp"
import star.rpc_promise;

#include "StarLuaComponents.hpp"

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
import star.container_interactor;
import star.widget_parsing;
import star.gui_reader;

namespace Star {

class ContainerEntity;
using ContainerEntityPtr = std::shared_ptr<ContainerEntity>;
using ContainerEntityConstPtr = std::shared_ptr<ContainerEntity const>;
using ContainerEntityWeakPtr = std::weak_ptr<ContainerEntity>;
using ContainerEntityConstWeakPtr = std::weak_ptr<ContainerEntity const>;
using ContainerEntityUPtr = std::unique_ptr<ContainerEntity>;
using ContainerEntityConstUPtr = std::unique_ptr<ContainerEntity const>;
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
class WorldClient;
using WorldClientPtr = std::shared_ptr<WorldClient>;
using WorldClientConstPtr = std::shared_ptr<WorldClient const>;
using WorldClientWeakPtr = std::weak_ptr<WorldClient>;
using WorldClientConstWeakPtr = std::weak_ptr<WorldClient const>;
using WorldClientUPtr = std::unique_ptr<WorldClient>;
using WorldClientConstUPtr = std::unique_ptr<WorldClient const>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<Item const>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<Item const>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<Item const>;
class ItemGridWidget;
using ItemGridWidgetPtr = std::shared_ptr<ItemGridWidget>;
using ItemGridWidgetConstPtr = std::shared_ptr<ItemGridWidget const>;
using ItemGridWidgetWeakPtr = std::weak_ptr<ItemGridWidget>;
using ItemGridWidgetConstWeakPtr = std::weak_ptr<ItemGridWidget const>;
using ItemGridWidgetUPtr = std::unique_ptr<ItemGridWidget>;
using ItemGridWidgetConstUPtr = std::unique_ptr<ItemGridWidget const>;
class ItemBag;
using ItemBagPtr = std::shared_ptr<ItemBag>;
using ItemBagConstPtr = std::shared_ptr<ItemBag const>;
using ItemBagWeakPtr = std::weak_ptr<ItemBag>;
using ItemBagConstWeakPtr = std::weak_ptr<ItemBag const>;
using ItemBagUPtr = std::unique_ptr<ItemBag>;
using ItemBagConstUPtr = std::unique_ptr<ItemBag const>;
class ContainerPane;
using ContainerPanePtr = std::shared_ptr<ContainerPane>;
using ContainerPaneConstPtr = std::shared_ptr<ContainerPane const>;
using ContainerPaneWeakPtr = std::weak_ptr<ContainerPane>;
using ContainerPaneConstWeakPtr = std::weak_ptr<ContainerPane const>;
using ContainerPaneUPtr = std::unique_ptr<ContainerPane>;
using ContainerPaneConstUPtr = std::unique_ptr<ContainerPane const>;

class ContainerPane : public Pane {
public:
  ContainerPane(WorldClientPtr worldClient, PlayerPtr player, ContainerInteractorPtr containerInteractor);

  void displayed() override;
  void dismissed() override;
  PanePtr createTooltip(Vec2I const& screenPosition) override;

  bool giveContainerResult(ContainerResult result);

protected:
  void update(float dt) override;

private:
  enum class ExpectingSwap {
    None,
    Inventory,
    SwapSlot,
    SwapSlotStack
  };

  void swapSlot(ItemGridWidget* grid);
  void startCrafting();
  void stopCrafting();
  void toggleCrafting();
  void clear();
  void burn();

  WorldClientPtr m_worldClient;
  PlayerPtr m_player;
  ContainerInteractorPtr m_containerInteractor;
  ItemBagPtr m_itemBag;

  ExpectingSwap m_expectingSwap;

  GuiReader m_reader;

  Maybe<LuaWorldComponent<LuaUpdatableComponent<LuaBaseComponent>>> m_script;
};

}

export module star.container_interface;

export namespace Star {
  using ::Star::ContainerEntity;
  using ::Star::ContainerEntityPtr;
  using ::Star::ContainerEntityConstPtr;
  using ::Star::ContainerEntityWeakPtr;
  using ::Star::ContainerEntityConstWeakPtr;
  using ::Star::ContainerEntityUPtr;
  using ::Star::ContainerEntityConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::WorldClient;
  using ::Star::WorldClientPtr;
  using ::Star::WorldClientConstPtr;
  using ::Star::WorldClientWeakPtr;
  using ::Star::WorldClientConstWeakPtr;
  using ::Star::WorldClientUPtr;
  using ::Star::WorldClientConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::ItemGridWidget;
  using ::Star::ItemGridWidgetPtr;
  using ::Star::ItemGridWidgetConstPtr;
  using ::Star::ItemGridWidgetWeakPtr;
  using ::Star::ItemGridWidgetConstWeakPtr;
  using ::Star::ItemGridWidgetUPtr;
  using ::Star::ItemGridWidgetConstUPtr;
  using ::Star::ItemBag;
  using ::Star::ItemBagPtr;
  using ::Star::ItemBagConstPtr;
  using ::Star::ItemBagWeakPtr;
  using ::Star::ItemBagConstWeakPtr;
  using ::Star::ItemBagUPtr;
  using ::Star::ItemBagConstUPtr;
  using ::Star::ContainerPane;
  using ::Star::ContainerPanePtr;
  using ::Star::ContainerPaneConstPtr;
  using ::Star::ContainerPaneWeakPtr;
  using ::Star::ContainerPaneConstWeakPtr;
  using ::Star::ContainerPaneUPtr;
  using ::Star::ContainerPaneConstUPtr;
}
