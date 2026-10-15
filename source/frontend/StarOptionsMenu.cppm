module;
import std;
import star.json;

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
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.version;

import star.ordered_map;




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
import star.configuration;
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;

namespace Star {
class UniverseClient;
}

namespace Star {

class SliderBarWidget;
class ButtonWidget;
class LabelWidget;
class VoiceSettingsMenu;
class KeybindingsMenu;
class GraphicsMenu;
class BindingsMenu;

class OptionsMenu : public Pane {
public:
  OptionsMenu(PaneManager* manager, std::shared_ptr<UniverseClient> client);

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

  std::shared_ptr<SliderBarWidget> m_instrumentSlider;
  std::shared_ptr<SliderBarWidget> m_sfxSlider;
  std::shared_ptr<SliderBarWidget> m_musicSlider;
  std::shared_ptr<ButtonWidget> m_tutorialMessagesButton;
  std::shared_ptr<ButtonWidget> m_interactiveHighlightButton;
  std::shared_ptr<ButtonWidget> m_clientIPJoinableButton;
  std::shared_ptr<ButtonWidget> m_clientP2PJoinableButton;
  std::shared_ptr<ButtonWidget> m_allowAssetsMismatchButton;
  std::shared_ptr<ButtonWidget> m_headRotationButton;

  std::shared_ptr<LabelWidget> m_instrumentLabel;
  std::shared_ptr<LabelWidget> m_sfxLabel;
  std::shared_ptr<LabelWidget> m_musicLabel;
  std::shared_ptr<LabelWidget> m_p2pJoinableLabel;

  //TODO: add instrument range (or just use one range for all 3, it's kinda silly.)
  Vec2I m_sfxRange;
  Vec2I m_musicRange;

  JsonObject m_origConfig;
  JsonObject m_localChanges;

  std::shared_ptr<VoiceSettingsMenu> m_voiceSettingsMenu;
  std::shared_ptr<BindingsMenu> m_modBindingsMenu;
  std::shared_ptr<KeybindingsMenu> m_keybindingsMenu;
  std::shared_ptr<GraphicsMenu> m_graphicsMenu;
  PaneManager* m_paneManager;
};

}

export module star.options_menu;

export namespace Star {
  using ::Star::SliderBarWidget;
  using ::Star::ButtonWidget;
  using ::Star::LabelWidget;
  using ::Star::VoiceSettingsMenu;
  using ::Star::KeybindingsMenu;
  using ::Star::GraphicsMenu;
  using ::Star::BindingsMenu;
  using ::Star::OptionsMenu;
}
