#pragma once

#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarPane.hpp"

namespace Star {

class TabSetWidget;
using TabSetWidgetPtr = SharedPtr<TabSetWidget>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;
class KeybindingsMenu;
using KeybindingsMenuPtr = SharedPtr<KeybindingsMenu>;

struct KeybindingsMenuServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  GuiContext& guiContext;
};

class KeybindingsMenu : public Pane {
public:
  explicit KeybindingsMenu(KeybindingsMenuServices services);

  // We need to handle our own Esc dismissal
  [[nodiscard]] KeyboardCaptureMode keyboardCaptureMode() const override;
  [[nodiscard]] bool sendEvent(InputEvent const& event) override;

  void show() override;
  void dismissed() override;

private:
  void buildListsFromConfig();
  [[nodiscard]] bool activateBinding(Widget* widget);
  void setKeybinding(KeyChord desc);
  void clearActive();
  void exitActiveMode();
  void apply();
  void revert();
  void resetDefaults();

  Widget* m_activeKeybinding = nullptr;

  Map<Widget*, InterfaceAction> m_childToAction;
  TabSetWidgetPtr m_tabSet;
  ListWidgetPtr m_playerList;
  ListWidgetPtr m_toolBarList;
  ListWidgetPtr m_gameList;

  Json m_origConfiguration;

  size_t m_maxBindings;
  KeyMod m_currentMods;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
};

}
