module;
import std;
import star.json;

import star.bi_map;
import star.strong_typedef;

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

class MainInterface;
class UniverseClient;
class Player;
class Item;
class ItemSlotWidget;
class ImageWidget;

class ActionBar : public Pane {
public:
  ActionBar(MainInterfacePaneManager* paneManager, std::shared_ptr<Player> player);

  std::shared_ptr<Pane> createTooltip(Vec2I const& screenPosition) override;
  bool sendEvent(InputEvent const& event) override;

  void update(float dt) override;

  Maybe<String> cursorOverride(Vec2I const& screenPosition) override;

private:
  struct CustomBarEntry {
    std::shared_ptr<ItemSlotWidget> left;
    std::shared_ptr<ItemSlotWidget> right;
    std::shared_ptr<ImageWidget> leftOverlay;
    std::shared_ptr<ImageWidget> rightOverlay;
  };

  void customBarClick(std::uint8_t index, bool primary);
  void customBarClickRight(std::uint8_t index, bool primary);
  void essentialBarClick(std::uint8_t index);
  void swapCustomBar();

  MainInterfacePaneManager* m_paneManager;
  std::shared_ptr<Player> m_player;
  Json m_config;

  Vec2I m_actionBarSelectOffset;
  StringList m_switchSounds;

  List<CustomBarEntry> m_customBarWidgets;
  std::shared_ptr<ImageWidget> m_customSelectedWidget;

  List<std::shared_ptr<ItemSlotWidget>> m_essentialBarWidgets;
  std::shared_ptr<ImageWidget> m_essentialSelectedWidget;

  SelectedActionBarLocation m_emptyHandsPreviousActionBarLocation;
  Maybe<std::pair<CustomBarIndex, bool>> m_customBarHover;
};

}

export module star.action_bar;

export namespace Star {
  using ::Star::MainInterface;
  using ::Star::UniverseClient;
  using ::Star::Player;
  using ::Star::Item;
  using ::Star::ItemSlotWidget;
  using ::Star::ImageWidget;
  using ::Star::ActionBar;
}
