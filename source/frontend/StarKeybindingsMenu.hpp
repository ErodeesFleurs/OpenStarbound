#pragma once

#include "StarIAssets.hpp"
#include "StarIConfiguration.hpp"
#include "StarPane.hpp"

namespace Star {

class TabSetWidget;
using TabSetWidgetPtr = SharedPtr<TabSetWidget>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;
class KeybindingsMenu;
using KeybindingsMenuPtr = SharedPtr<KeybindingsMenu>;

struct KeybindingsMenuServices {
  IAssetsConstPtr assets;
  IConfigurationPtr configuration;
};

class KeybindingsMenu : public Pane {
public:
  explicit KeybindingsMenu(KeybindingsMenuServices services);

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

  size_t m_maxBindings;
  KeyMod m_currentMods;
  IAssetsConstPtr m_assets;
  IConfigurationPtr m_configuration;
};

}
