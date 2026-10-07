module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarJson.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarConfig.hpp"
import star.version;

#include "StarOrderedMap.hpp"




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
STAR_CLASS(UniverseClient);
}

namespace Star {

STAR_CLASS(SliderBarWidget);
STAR_CLASS(ButtonWidget);
STAR_CLASS(LabelWidget);
STAR_CLASS(VoiceSettingsMenu);
STAR_CLASS(KeybindingsMenu);
STAR_CLASS(GraphicsMenu);
STAR_CLASS(BindingsMenu);
STAR_CLASS(OptionsMenu);

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
