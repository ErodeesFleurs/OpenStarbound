module;
#include "StarConfig.hpp"
import star.json;

import star.bi_map;
#include "StarStrongTypedef.hpp"

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
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.ordered_map;




import star.inventory_types;
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
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;

namespace Star {

STAR_CLASS(MainInterface);
STAR_CLASS(UniverseClient);
STAR_CLASS(Player);
STAR_CLASS(Item);
STAR_CLASS(ItemSlotWidget);
STAR_CLASS(ImageWidget);

STAR_CLASS(ActionBar);

class ActionBar : public Pane {
public:
  ActionBar(MainInterfacePaneManager* paneManager, PlayerPtr player);

  PanePtr createTooltip(Vec2I const& screenPosition) override;
  bool sendEvent(InputEvent const& event) override;

  void update(float dt) override;

  Maybe<String> cursorOverride(Vec2I const& screenPosition) override;

private:
  struct CustomBarEntry {
    ItemSlotWidgetPtr left;
    ItemSlotWidgetPtr right;
    ImageWidgetPtr leftOverlay;
    ImageWidgetPtr rightOverlay;
  };

  void customBarClick(uint8_t index, bool primary);
  void customBarClickRight(uint8_t index, bool primary);
  void essentialBarClick(uint8_t index);
  void swapCustomBar();

  MainInterfacePaneManager* m_paneManager;
  PlayerPtr m_player;
  Json m_config;

  Vec2I m_actionBarSelectOffset;
  StringList m_switchSounds;

  List<CustomBarEntry> m_customBarWidgets;
  ImageWidgetPtr m_customSelectedWidget;

  List<ItemSlotWidgetPtr> m_essentialBarWidgets;
  ImageWidgetPtr m_essentialSelectedWidget;

  SelectedActionBarLocation m_emptyHandsPreviousActionBarLocation;
  Maybe<pair<CustomBarIndex, bool>> m_customBarHover;
};

}

export module star.action_bar;

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
  using ::Star::ImageWidget;
  using ::Star::ImageWidgetPtr;
  using ::Star::ImageWidgetConstPtr;
  using ::Star::ImageWidgetWeakPtr;
  using ::Star::ImageWidgetConstWeakPtr;
  using ::Star::ImageWidgetUPtr;
  using ::Star::ImageWidgetConstUPtr;
  using ::Star::ActionBar;
  using ::Star::ActionBarPtr;
  using ::Star::ActionBarConstPtr;
  using ::Star::ActionBarWeakPtr;
  using ::Star::ActionBarConstWeakPtr;
  using ::Star::ActionBarUPtr;
  using ::Star::ActionBarConstUPtr;
}
