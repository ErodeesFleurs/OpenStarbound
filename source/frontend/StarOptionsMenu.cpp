#include "StarOptionsMenu.hpp"
#include "StarAlgorithm.hpp"
#include "StarGuiReader.hpp"
#include "StarLexicalCast.hpp"
#include "StarJsonExtra.hpp"
#include "StarSliderBar.hpp"
#include "StarLabelWidget.hpp"
#include "StarAssets.hpp"
#include "StarKeybindingsMenu.hpp"
#include "StarVoiceSettingsMenu.hpp"
#include "StarBindingsMenu.hpp"
#include "StarGraphicsMenu.hpp"

namespace Star {

OptionsMenu::OptionsMenu(PaneManager& manager, UniverseClientPtr client, OptionsMenuServices services)
  : Pane(services.guiContext),
    m_sfxRange(0, 100),
    m_musicRange(0, 100),
    m_paneManager(manager),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "OptionsMenu", "assets")),
    m_configuration(requireServiceValueAs<StarException>(std::move(services.configuration), "OptionsMenu", "configuration")),
    m_luaRootServices(requireLuaRootServices(std::move(services.luaRootServices), "OptionsMenu")),
    m_voice(services.voice),
    m_input(services.input) {
  GuiReader reader(context());

  reader.registerCallback("instrumentSlider", [=, this](Widget*) {
    updateInstrumentVol();
    });
  reader.registerCallback("sfxSlider", [=, this](Widget*) {
      updateSFXVol();
    });
  reader.registerCallback("musicSlider", [=, this](Widget*) {
      updateMusicVol();
    });
  reader.registerCallback("acceptButton", [=, this](Widget*) {
      for (auto k : ConfigKeys)
        m_configuration->set(k, m_localChanges.get(k));

      dismiss();
    });
  reader.registerCallback("tutorialMessagesCheckbox", [=, this](Widget*) {
      updateTutorialMessages();
    });
  reader.registerCallback("clientIPJoinableCheckbox", [=, this](Widget*) {
      updateClientIPJoinable();
    });
  reader.registerCallback("clientP2PJoinableCheckbox", [=, this](Widget*) {
      updateClientP2PJoinable();
    });
  reader.registerCallback("allowAssetsMismatchCheckbox", [=, this](Widget*) {
      updateAllowAssetsMismatch();
    });
  reader.registerCallback("headRotationCheckbox", [=, this](Widget*) {
      updateHeadRotation();
    });
  reader.registerCallback("backButton", [=, this](Widget*) {
      dismiss();
    });
  reader.registerCallback("showKeybindings", [=, this](Widget*) {
      displayControls();
    });
  reader.registerCallback("showVoiceSettings", [=, this](Widget*) {
      displayVoiceSettings();
    });
  reader.registerCallback("showVoicePlayers", [=](Widget*) {

    });
  reader.registerCallback("showModBindings", [=, this](Widget*) {
      displayModBindings();
    });
  reader.registerCallback("showGraphics", [=, this](Widget*) {
      displayGraphics();
    });

  Json config = m_assets->json("/interface/optionsmenu/optionsmenu.config");

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

  m_instrumentSlider->setRange(m_sfxRange, m_assets->json("/interface/optionsmenu/optionsmenu.config:sfxDelta").toInt());
  m_sfxSlider->setRange(m_sfxRange, m_assets->json("/interface/optionsmenu/optionsmenu.config:sfxDelta").toInt());
  m_musicSlider->setRange(m_musicRange, m_assets->json("/interface/optionsmenu/optionsmenu.config:musicDelta").toInt());

  m_voiceSettingsMenu = make_shared<VoiceSettingsMenu>(m_assets->json(config.getString("voiceSettingsPanePath", "/interface/opensb/voicechat/voicechat.config")), BaseScriptPaneServices{m_assets, {}, {}, {}, m_luaRootServices, context()}, m_voice);
  m_modBindingsMenu = make_shared<BindingsMenu>(m_assets->json(config.getString("bindingsPanePath", "/interface/opensb/bindings/bindings.config")), BaseScriptPaneServices{m_assets, {}, {}, {}, m_luaRootServices, context()}, m_input);
  m_keybindingsMenu = make_shared<KeybindingsMenu>(KeybindingsMenuServices{m_assets, m_configuration, context()});
  m_graphicsMenu = make_shared<GraphicsMenu>(manager, client, GraphicsMenuServices{m_assets, m_configuration, context()});

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
  for (auto k : ConfigKeys) {
    m_origConfig[k] = m_configuration->get(k);
    m_localChanges[k] = m_configuration->get(k);
  }
}

void OptionsMenu::updateInstrumentVol() {
  m_localChanges.set("instrumentVol", m_instrumentSlider->val());
  m_configuration->set("instrumentVol", m_instrumentSlider->val());
  m_instrumentLabel->setText(toString(m_instrumentSlider->val()));
}

void OptionsMenu::updateSFXVol() {
  m_localChanges.set("sfxVol", m_sfxSlider->val());
  m_configuration->set("sfxVol", m_sfxSlider->val());
  m_sfxLabel->setText(toString(m_sfxSlider->val()));
}

void OptionsMenu::updateMusicVol() {
  m_localChanges.set("musicVol", {m_musicSlider->val()});
  m_configuration->set("musicVol", m_musicSlider->val());
  m_musicLabel->setText(toString(m_musicSlider->val()));
}


void OptionsMenu::updateTutorialMessages() {
  m_localChanges.set("tutorialMessages", m_tutorialMessagesButton->isChecked());
  m_configuration->set("tutorialMessages", m_tutorialMessagesButton->isChecked());
}

void OptionsMenu::updateClientIPJoinable() {
  m_localChanges.set("clientIPJoinable", m_clientIPJoinableButton->isChecked());
  m_configuration->set("clientIPJoinable", m_clientIPJoinableButton->isChecked());
}

void OptionsMenu::updateClientP2PJoinable() {
  m_localChanges.set("clientP2PJoinable", m_clientP2PJoinableButton->isChecked());
  m_configuration->set("clientP2PJoinable", m_clientP2PJoinableButton->isChecked());
}

void OptionsMenu::updateAllowAssetsMismatch() {
  m_localChanges.set("allowAssetsMismatch", m_allowAssetsMismatchButton->isChecked());
  m_configuration->set("allowAssetsMismatch", m_allowAssetsMismatchButton->isChecked());
}

void OptionsMenu::updateHeadRotation() {
  m_localChanges.set("humanoidHeadRotation", m_headRotationButton->isChecked());
  m_configuration->set("humanoidHeadRotation", m_headRotationButton->isChecked());
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

  auto appController = context().applicationController();
  if (!appController->p2pNetworkingService()) {
    m_p2pJoinableLabel->setColor(Color::DarkGray);
    m_clientP2PJoinableButton->setEnabled(false);
    m_clientP2PJoinableButton->setChecked(false);
  }
}

void OptionsMenu::displayControls() {
  m_paneManager.displayPane(PaneLayer::ModalWindow, m_keybindingsMenu);
}

void OptionsMenu::displayVoiceSettings() {
  m_paneManager.displayPane(PaneLayer::ModalWindow, m_voiceSettingsMenu);
}

void OptionsMenu::displayModBindings() {
  m_paneManager.displayPane(PaneLayer::ModalWindow, m_modBindingsMenu);
}

void OptionsMenu::displayGraphics() {
  m_paneManager.displayPane(PaneLayer::ModalWindow, m_graphicsMenu);
}

}
