#pragma once

#include "StarAssets.hpp"
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
class Input;
class Voice;
class GuiContext;

struct OptionsMenuServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  LuaRootServices luaRootServices;
  Voice& voice;
  Input& input;
  GuiContext& guiContext;
};

class OptionsMenu : public Pane {
public:
  OptionsMenu(PaneManager& manager, UniverseClientPtr client, OptionsMenuServices services);

  void show() override;

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

  WidgetRef<SliderBarWidget> m_instrumentSlider;
  WidgetRef<SliderBarWidget> m_sfxSlider;
  WidgetRef<SliderBarWidget> m_musicSlider;
  WidgetRef<ButtonWidget> m_tutorialMessagesButton;
  ButtonWidgetPtr m_interactiveHighlightButton;
  WidgetRef<ButtonWidget> m_clientIPJoinableButton;
  WidgetRef<ButtonWidget> m_clientP2PJoinableButton;
  WidgetRef<ButtonWidget> m_allowAssetsMismatchButton;
  WidgetRef<ButtonWidget> m_headRotationButton;

  WidgetRef<LabelWidget> m_instrumentLabel;
  WidgetRef<LabelWidget> m_sfxLabel;
  WidgetRef<LabelWidget> m_musicLabel;
  WidgetRef<LabelWidget> m_p2pJoinableLabel;

  //TODO: add instrument range (or just use one range for all 3, it's kinda silly.)
  Vec2I m_sfxRange;
  Vec2I m_musicRange;

  JsonObject m_origConfig;
  JsonObject m_localChanges;

  VoiceSettingsMenuPtr m_voiceSettingsMenu;
  BindingsMenuPtr m_modBindingsMenu;
  KeybindingsMenuPtr m_keybindingsMenu;
  GraphicsMenuPtr m_graphicsMenu;
  PaneManager& m_paneManager;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  LuaRootServices m_luaRootServices;
  Voice& m_voice;
  Input& m_input;
};

}
