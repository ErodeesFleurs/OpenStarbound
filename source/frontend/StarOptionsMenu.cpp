
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
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
#include "StarVersion.hpp"
import star.configuration;

#include "StarOrderedMap.hpp"
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;


import star.main_interface_types;
#include "StarUniverseClient.hpp"


import star.options_menu;
import star.widget_parsing;
import star.gui_reader;
#include "StarLexicalCast.hpp"
#include "StarJsonExtra.hpp"
import star.button_group;
import star.button_widget;
import star.image_widget;
import star.slider_bar;
import star.label_widget;
#include "StarAssets.hpp"



import star.keybindings_menu;


#include "StarLuaComponents.hpp"


import star.base_script_pane;


import star.voice_settings_menu;






import star.bindings_menu;






import star.graphics_menu;
#include "StarHumanoid.hpp"

namespace Star {

OptionsMenu::OptionsMenu(PaneManager* manager, UniverseClientPtr client)
  : m_sfxRange(0, 100), m_musicRange(0, 100), m_paneManager(manager) {
  auto root = Root::singletonPtr();
  auto assets = root->assets();

  GuiReader reader;

  reader.registerCallback("instrumentSlider", [this](Widget*) {
    updateInstrumentVol();
    });
  reader.registerCallback("sfxSlider", [this](Widget*) {
      updateSFXVol();
    });
  reader.registerCallback("musicSlider", [this](Widget*) {
      updateMusicVol();
    });
  reader.registerCallback("acceptButton", [=, this](Widget*) {
      for (auto k : ConfigKeys)
        root->configuration()->set(k, m_localChanges.get(k));

      dismiss();
    });
  reader.registerCallback("tutorialMessagesCheckbox", [this](Widget*) {
      updateTutorialMessages();
    });
  reader.registerCallback("clientIPJoinableCheckbox", [this](Widget*) {
      updateClientIPJoinable();
    });
  reader.registerCallback("clientP2PJoinableCheckbox", [this](Widget*) {
      updateClientP2PJoinable();
    });
  reader.registerCallback("allowAssetsMismatchCheckbox", [this](Widget*) {
      updateAllowAssetsMismatch();
    });
  reader.registerCallback("headRotationCheckbox", [this](Widget*) {
      updateHeadRotation();
    });
  reader.registerCallback("backButton", [this](Widget*) {
      dismiss();
    });
  reader.registerCallback("showKeybindings", [this](Widget*) {
      displayControls();
    });
  reader.registerCallback("showVoiceSettings", [this](Widget*) {
      displayVoiceSettings();
    });
  reader.registerCallback("showVoicePlayers", [=](Widget*) {

    });
  reader.registerCallback("showModBindings", [this](Widget*) {
      displayModBindings();
    });
  reader.registerCallback("showGraphics", [this](Widget*) {
      displayGraphics();
    });

  Json config = assets->json("/interface/optionsmenu/optionsmenu.config");

  reader.construct(config.get("paneLayout"), this);

  m_instrumentSlider = fetchChild<SliderBarWidget>("instrumentSlider");
  m_sfxSlider = fetchChild<SliderBarWidget>("sfxSlider");
  m_musicSlider = fetchChild<SliderBarWidget>("musicSlider");
  m_tutorialMessagesButton = fetchChild<ButtonWidget>("tutorialMessagesCheckbox");
  m_clientIPJoinableButton = fetchChild<ButtonWidget>("clientIPJoinableCheckbox");
  m_clientP2PJoinableButton = fetchChild<ButtonWidget>("clientP2PJoinableCheckbox");
  m_allowAssetsMismatchButton = fetchChild<ButtonWidget>("allowAssetsMismatchCheckbox");
  m_headRotationButton = fetchChild<ButtonWidget>("headRotationCheckbox");

  m_instrumentLabel = fetchChild<LabelWidget>("instrumentValueLabel");
  m_sfxLabel = fetchChild<LabelWidget>("sfxValueLabel");
  m_musicLabel = fetchChild<LabelWidget>("musicValueLabel");
  m_p2pJoinableLabel = fetchChild<LabelWidget>("clientP2PJoinableLabel");

  m_instrumentSlider->setRange(m_sfxRange, assets->json("/interface/optionsmenu/optionsmenu.config:sfxDelta").toInt());
  m_sfxSlider->setRange(m_sfxRange, assets->json("/interface/optionsmenu/optionsmenu.config:sfxDelta").toInt());
  m_musicSlider->setRange(m_musicRange, assets->json("/interface/optionsmenu/optionsmenu.config:musicDelta").toInt());

  m_voiceSettingsMenu = make_shared<VoiceSettingsMenu>(assets->json(config.getString("voiceSettingsPanePath", "/interface/opensb/voicechat/voicechat.config")));
  m_modBindingsMenu = make_shared<BindingsMenu>(assets->json(config.getString("bindingsPanePath", "/interface/opensb/bindings/bindings.config")));
  m_keybindingsMenu = make_shared<KeybindingsMenu>();
  m_graphicsMenu = make_shared<GraphicsMenu>(manager,client);

  initConfig();
}

void OptionsMenu::show() {
  initConfig();
  syncGuiToConf();

  Pane::show();
}

void OptionsMenu::toggleFullscreen() {
  m_graphicsMenu->toggleFullscreen();

  syncGuiToConf();
}

StringList const OptionsMenu::ConfigKeys = {
  "instrumentVol",
  "sfxVol",
  "musicVol",
  "tutorialMessages",
  "clientIPJoinable",
  "clientP2PJoinable",
  "allowAssetsMismatch",
  "humanoidHeadRotation"
};

void OptionsMenu::initConfig() {
  auto configuration = Root::singleton().configuration();

  for (auto k : ConfigKeys) {
    m_origConfig[k] = configuration->get(k);
    m_localChanges[k] = configuration->get(k);
  }
}

void OptionsMenu::updateInstrumentVol() {
  m_localChanges.set("instrumentVol", m_instrumentSlider->val());
  Root::singleton().configuration()->set("instrumentVol", m_instrumentSlider->val());
  m_instrumentLabel->setText(toString(m_instrumentSlider->val()));
}

void OptionsMenu::updateSFXVol() {
  m_localChanges.set("sfxVol", m_sfxSlider->val());
  Root::singleton().configuration()->set("sfxVol", m_sfxSlider->val());
  m_sfxLabel->setText(toString(m_sfxSlider->val()));
}

void OptionsMenu::updateMusicVol() {
  m_localChanges.set("musicVol", {m_musicSlider->val()});
  Root::singleton().configuration()->set("musicVol", m_musicSlider->val());
  m_musicLabel->setText(toString(m_musicSlider->val()));
}


void OptionsMenu::updateTutorialMessages() {
  m_localChanges.set("tutorialMessages", m_tutorialMessagesButton->isChecked());
  Root::singleton().configuration()->set("tutorialMessages", m_tutorialMessagesButton->isChecked());
}

void OptionsMenu::updateClientIPJoinable() {
  m_localChanges.set("clientIPJoinable", m_clientIPJoinableButton->isChecked());
  Root::singleton().configuration()->set("clientIPJoinable", m_clientIPJoinableButton->isChecked());
}

void OptionsMenu::updateClientP2PJoinable() {
  m_localChanges.set("clientP2PJoinable", m_clientP2PJoinableButton->isChecked());
  Root::singleton().configuration()->set("clientP2PJoinable", m_clientP2PJoinableButton->isChecked());
}

void OptionsMenu::updateAllowAssetsMismatch() {
  m_localChanges.set("allowAssetsMismatch", m_allowAssetsMismatchButton->isChecked());
  Root::singleton().configuration()->set("allowAssetsMismatch", m_allowAssetsMismatchButton->isChecked());
}

void OptionsMenu::updateHeadRotation() {
  m_localChanges.set("humanoidHeadRotation", m_headRotationButton->isChecked());
  Root::singleton().configuration()->set("humanoidHeadRotation", m_headRotationButton->isChecked());
  Humanoid::globalHeadRotation() = m_headRotationButton->isChecked();
}

void OptionsMenu::syncGuiToConf() {
  m_instrumentSlider->setVal(m_localChanges.get("instrumentVol").toInt(), false);
  m_instrumentLabel->setText(toString(m_instrumentSlider->val()));

  m_sfxSlider->setVal(m_localChanges.get("sfxVol").toInt(), false);
  m_sfxLabel->setText(toString(m_sfxSlider->val()));

  m_musicSlider->setVal(m_localChanges.get("musicVol").toInt(), false);
  m_musicLabel->setText(toString(m_musicSlider->val()));

  m_tutorialMessagesButton->setChecked(m_localChanges.get("tutorialMessages").toBool());
  m_clientIPJoinableButton->setChecked(m_localChanges.get("clientIPJoinable").toBool());
  m_clientP2PJoinableButton->setChecked(m_localChanges.get("clientP2PJoinable").toBool());
  m_allowAssetsMismatchButton->setChecked(m_localChanges.get("allowAssetsMismatch").toBool());
  m_headRotationButton->setChecked(m_localChanges.get("humanoidHeadRotation").optBool().value(true));

  auto appController = GuiContext::singleton().applicationController();
  if (!appController->p2pNetworkingService()) {
    m_p2pJoinableLabel->setColor(Color::DarkGray);
    m_clientP2PJoinableButton->setEnabled(false);
    m_clientP2PJoinableButton->setChecked(false);
  }
}

void OptionsMenu::displayControls() {
  m_paneManager->displayPane(PaneLayer::ModalWindow, m_keybindingsMenu);
}

void OptionsMenu::displayVoiceSettings() {
  m_paneManager->displayPane(PaneLayer::ModalWindow, m_voiceSettingsMenu);
}

void OptionsMenu::displayModBindings() {
  m_paneManager->displayPane(PaneLayer::ModalWindow, m_modBindingsMenu);
}

void OptionsMenu::displayGraphics() {
  m_paneManager->displayPane(PaneLayer::ModalWindow, m_graphicsMenu);
}

}
