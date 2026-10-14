module;
import std;
import star.json;

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

namespace Star {

class TabSetWidget;
using TabSetWidgetPtr = std::shared_ptr<TabSetWidget>;
using TabSetWidgetConstPtr = std::shared_ptr<TabSetWidget const>;
using TabSetWidgetWeakPtr = std::weak_ptr<TabSetWidget>;
using TabSetWidgetConstWeakPtr = std::weak_ptr<TabSetWidget const>;
using TabSetWidgetUPtr = std::unique_ptr<TabSetWidget>;
using TabSetWidgetConstUPtr = std::unique_ptr<TabSetWidget const>;
class ListWidget;
using ListWidgetPtr = std::shared_ptr<ListWidget>;
using ListWidgetConstPtr = std::shared_ptr<ListWidget const>;
using ListWidgetWeakPtr = std::weak_ptr<ListWidget>;
using ListWidgetConstWeakPtr = std::weak_ptr<ListWidget const>;
using ListWidgetUPtr = std::unique_ptr<ListWidget>;
using ListWidgetConstUPtr = std::unique_ptr<ListWidget const>;
class KeybindingsMenu;
using KeybindingsMenuPtr = std::shared_ptr<KeybindingsMenu>;
using KeybindingsMenuConstPtr = std::shared_ptr<KeybindingsMenu const>;
using KeybindingsMenuWeakPtr = std::weak_ptr<KeybindingsMenu>;
using KeybindingsMenuConstWeakPtr = std::weak_ptr<KeybindingsMenu const>;
using KeybindingsMenuUPtr = std::unique_ptr<KeybindingsMenu>;
using KeybindingsMenuConstUPtr = std::unique_ptr<KeybindingsMenu const>;

class KeybindingsMenu : public Pane {
public:
  KeybindingsMenu();

  // We need to handle our own Esc dismissal
  KeyboardCaptureMode keyboardCaptureMode() const override;
  bool sendEvent(InputEvent const& event) override;

  void show() override;
  void dismissed() override;

private:
  void buildListsFromConfig();
  bool activateBinding(Widget* widget);
  void setKeybinding(KeyChord desc);
  void clearActive();
  void exitActiveMode();
  void apply();
  void revert();
  void resetDefaults();

  Widget* m_activeKeybinding;

  Map<Widget*, InterfaceAction> m_childToAction;
  TabSetWidgetPtr m_tabSet;
  ListWidgetPtr m_playerList;
  ListWidgetPtr m_toolBarList;
  ListWidgetPtr m_gameList;

  Json m_origConfiguration;

  std::size_t m_maxBindings;
  KeyMod m_currentMods;
};

}

export module star.keybindings_menu;

export namespace Star {
  using ::Star::TabSetWidget;
  using ::Star::TabSetWidgetPtr;
  using ::Star::TabSetWidgetConstPtr;
  using ::Star::TabSetWidgetWeakPtr;
  using ::Star::TabSetWidgetConstWeakPtr;
  using ::Star::TabSetWidgetUPtr;
  using ::Star::TabSetWidgetConstUPtr;
  using ::Star::ListWidget;
  using ::Star::ListWidgetPtr;
  using ::Star::ListWidgetConstPtr;
  using ::Star::ListWidgetWeakPtr;
  using ::Star::ListWidgetConstWeakPtr;
  using ::Star::ListWidgetUPtr;
  using ::Star::ListWidgetConstUPtr;
  using ::Star::KeybindingsMenu;
  using ::Star::KeybindingsMenuPtr;
  using ::Star::KeybindingsMenuConstPtr;
  using ::Star::KeybindingsMenuWeakPtr;
  using ::Star::KeybindingsMenuConstWeakPtr;
  using ::Star::KeybindingsMenuUPtr;
  using ::Star::KeybindingsMenuConstUPtr;
}
