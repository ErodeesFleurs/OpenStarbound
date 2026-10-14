module;
import std;
import star.json;

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
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
}

namespace Star {

class SliderBarWidget;
using SliderBarWidgetPtr = std::shared_ptr<SliderBarWidget>;
using SliderBarWidgetConstPtr = std::shared_ptr<SliderBarWidget const>;
using SliderBarWidgetWeakPtr = std::weak_ptr<SliderBarWidget>;
using SliderBarWidgetConstWeakPtr = std::weak_ptr<SliderBarWidget const>;
using SliderBarWidgetUPtr = std::unique_ptr<SliderBarWidget>;
using SliderBarWidgetConstUPtr = std::unique_ptr<SliderBarWidget const>;
class ButtonWidget;
using ButtonWidgetPtr = std::shared_ptr<ButtonWidget>;
using ButtonWidgetConstPtr = std::shared_ptr<ButtonWidget const>;
using ButtonWidgetWeakPtr = std::weak_ptr<ButtonWidget>;
using ButtonWidgetConstWeakPtr = std::weak_ptr<ButtonWidget const>;
using ButtonWidgetUPtr = std::unique_ptr<ButtonWidget>;
using ButtonWidgetConstUPtr = std::unique_ptr<ButtonWidget const>;
class LabelWidget;
using LabelWidgetPtr = std::shared_ptr<LabelWidget>;
using LabelWidgetConstPtr = std::shared_ptr<LabelWidget const>;
using LabelWidgetWeakPtr = std::weak_ptr<LabelWidget>;
using LabelWidgetConstWeakPtr = std::weak_ptr<LabelWidget const>;
using LabelWidgetUPtr = std::unique_ptr<LabelWidget>;
using LabelWidgetConstUPtr = std::unique_ptr<LabelWidget const>;
class VoiceSettingsMenu;
using VoiceSettingsMenuPtr = std::shared_ptr<VoiceSettingsMenu>;
using VoiceSettingsMenuConstPtr = std::shared_ptr<VoiceSettingsMenu const>;
using VoiceSettingsMenuWeakPtr = std::weak_ptr<VoiceSettingsMenu>;
using VoiceSettingsMenuConstWeakPtr = std::weak_ptr<VoiceSettingsMenu const>;
using VoiceSettingsMenuUPtr = std::unique_ptr<VoiceSettingsMenu>;
using VoiceSettingsMenuConstUPtr = std::unique_ptr<VoiceSettingsMenu const>;
class KeybindingsMenu;
using KeybindingsMenuPtr = std::shared_ptr<KeybindingsMenu>;
using KeybindingsMenuConstPtr = std::shared_ptr<KeybindingsMenu const>;
using KeybindingsMenuWeakPtr = std::weak_ptr<KeybindingsMenu>;
using KeybindingsMenuConstWeakPtr = std::weak_ptr<KeybindingsMenu const>;
using KeybindingsMenuUPtr = std::unique_ptr<KeybindingsMenu>;
using KeybindingsMenuConstUPtr = std::unique_ptr<KeybindingsMenu const>;
class GraphicsMenu;
using GraphicsMenuPtr = std::shared_ptr<GraphicsMenu>;
using GraphicsMenuConstPtr = std::shared_ptr<GraphicsMenu const>;
using GraphicsMenuWeakPtr = std::weak_ptr<GraphicsMenu>;
using GraphicsMenuConstWeakPtr = std::weak_ptr<GraphicsMenu const>;
using GraphicsMenuUPtr = std::unique_ptr<GraphicsMenu>;
using GraphicsMenuConstUPtr = std::unique_ptr<GraphicsMenu const>;
class BindingsMenu;
using BindingsMenuPtr = std::shared_ptr<BindingsMenu>;
using BindingsMenuConstPtr = std::shared_ptr<BindingsMenu const>;
using BindingsMenuWeakPtr = std::weak_ptr<BindingsMenu>;
using BindingsMenuConstWeakPtr = std::weak_ptr<BindingsMenu const>;
using BindingsMenuUPtr = std::unique_ptr<BindingsMenu>;
using BindingsMenuConstUPtr = std::unique_ptr<BindingsMenu const>;
class OptionsMenu;
using OptionsMenuPtr = std::shared_ptr<OptionsMenu>;
using OptionsMenuConstPtr = std::shared_ptr<OptionsMenu const>;
using OptionsMenuWeakPtr = std::weak_ptr<OptionsMenu>;
using OptionsMenuConstWeakPtr = std::weak_ptr<OptionsMenu const>;
using OptionsMenuUPtr = std::unique_ptr<OptionsMenu>;
using OptionsMenuConstUPtr = std::unique_ptr<OptionsMenu const>;

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

export module star.options_menu;

export namespace Star {
  using ::Star::SliderBarWidget;
  using ::Star::SliderBarWidgetPtr;
  using ::Star::SliderBarWidgetConstPtr;
  using ::Star::SliderBarWidgetWeakPtr;
  using ::Star::SliderBarWidgetConstWeakPtr;
  using ::Star::SliderBarWidgetUPtr;
  using ::Star::SliderBarWidgetConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::VoiceSettingsMenu;
  using ::Star::VoiceSettingsMenuPtr;
  using ::Star::VoiceSettingsMenuConstPtr;
  using ::Star::VoiceSettingsMenuWeakPtr;
  using ::Star::VoiceSettingsMenuConstWeakPtr;
  using ::Star::VoiceSettingsMenuUPtr;
  using ::Star::VoiceSettingsMenuConstUPtr;
  using ::Star::KeybindingsMenu;
  using ::Star::KeybindingsMenuPtr;
  using ::Star::KeybindingsMenuConstPtr;
  using ::Star::KeybindingsMenuWeakPtr;
  using ::Star::KeybindingsMenuConstWeakPtr;
  using ::Star::KeybindingsMenuUPtr;
  using ::Star::KeybindingsMenuConstUPtr;
  using ::Star::GraphicsMenu;
  using ::Star::GraphicsMenuPtr;
  using ::Star::GraphicsMenuConstPtr;
  using ::Star::GraphicsMenuWeakPtr;
  using ::Star::GraphicsMenuConstWeakPtr;
  using ::Star::GraphicsMenuUPtr;
  using ::Star::GraphicsMenuConstUPtr;
  using ::Star::BindingsMenu;
  using ::Star::BindingsMenuPtr;
  using ::Star::BindingsMenuConstPtr;
  using ::Star::BindingsMenuWeakPtr;
  using ::Star::BindingsMenuConstWeakPtr;
  using ::Star::BindingsMenuUPtr;
  using ::Star::BindingsMenuConstUPtr;
  using ::Star::OptionsMenu;
  using ::Star::OptionsMenuPtr;
  using ::Star::OptionsMenuConstPtr;
  using ::Star::OptionsMenuWeakPtr;
  using ::Star::OptionsMenuConstWeakPtr;
  using ::Star::OptionsMenuUPtr;
  using ::Star::OptionsMenuConstUPtr;
}
