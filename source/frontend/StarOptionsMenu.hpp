#pragma once

#include "StarPane.hpp"
#include "StarConfiguration.hpp"
#include "StarMainInterfaceTypes.hpp"
#include "StarUniverseClient.hpp"

namespace Star {

class SliderBarWidget;
using SliderBarWidgetPtr = SharedPtr<SliderBarWidget>;
class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;
class LabelWidget;
using LabelWidgetPtr = SharedPtr<LabelWidget>;
class VoiceSettingsMenu;
using VoiceSettingsMenuPtr = SharedPtr<VoiceSettingsMenu>;
class KeybindingsMenu;
using KeybindingsMenuPtr = SharedPtr<KeybindingsMenu>;
class GraphicsMenu;
using GraphicsMenuPtr = SharedPtr<GraphicsMenu>;
class BindingsMenu;
using BindingsMenuPtr = SharedPtr<BindingsMenu>;
class OptionsMenu;
using OptionsMenuPtr = SharedPtr<OptionsMenu>;

class OptionsMenu : public Pane {
public:
  OptionsMenu(PaneManager* manager, UniverseClientPtr client);

  virtual void show() override;

  void toggleFullscreen();

private:
  static StringList const ConfigKeys;

  void initConfig();

  void updateInstrumentVol();
  void updateSFXVol();
  void updateMusicVol();
  void updateTutorialMessages();
  void updateClientIPJoinable();
  void updateClientP2PJoinable();
  void updateAllowAssetsMismatch();
  void updateHeadRotation();

  void syncGuiToConf();

  void displayControls();
  void displayVoiceSettings();
  void displayModBindings();
  void displayGraphics();

  SliderBarWidgetPtr m_instrumentSlider;
  SliderBarWidgetPtr m_sfxSlider;
  SliderBarWidgetPtr m_musicSlider;
  ButtonWidgetPtr m_tutorialMessagesButton;
  ButtonWidgetPtr m_interactiveHighlightButton;
  ButtonWidgetPtr m_clientIPJoinableButton;
  ButtonWidgetPtr m_clientP2PJoinableButton;
  ButtonWidgetPtr m_allowAssetsMismatchButton;
  ButtonWidgetPtr m_headRotationButton;

  LabelWidgetPtr m_instrumentLabel;
  LabelWidgetPtr m_sfxLabel;
  LabelWidgetPtr m_musicLabel;
  LabelWidgetPtr m_p2pJoinableLabel;

  //TODO: add instrument range (or just use one range for all 3, it's kinda silly.)
  Vec2I m_sfxRange;
  Vec2I m_musicRange;

  JsonObject m_origConfig;
  JsonObject m_localChanges;

  VoiceSettingsMenuPtr m_voiceSettingsMenu;
  BindingsMenuPtr m_modBindingsMenu;
  KeybindingsMenuPtr m_keybindingsMenu;
  GraphicsMenuPtr m_graphicsMenu;
  PaneManager* m_paneManager;
};

}
