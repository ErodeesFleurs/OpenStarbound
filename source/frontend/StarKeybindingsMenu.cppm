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
class ListWidget;

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
  std::shared_ptr<TabSetWidget> m_tabSet;
  std::shared_ptr<ListWidget> m_playerList;
  std::shared_ptr<ListWidget> m_toolBarList;
  std::shared_ptr<ListWidget> m_gameList;

  Json m_origConfiguration;

  std::size_t m_maxBindings;
  KeyMod m_currentMods;
};

}

export module star.keybindings_menu;

export namespace Star {
  using ::Star::TabSetWidget;
  using ::Star::ListWidget;
  using ::Star::KeybindingsMenu;
}
