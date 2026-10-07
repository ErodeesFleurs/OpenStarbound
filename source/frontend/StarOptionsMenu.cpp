#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarJsonExtra.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarVersion.hpp"
#include "StarOrderedMap.hpp"
#include "StarEither.hpp"
#include "StarVariant.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarOrderedSet.hpp"
#include "StarZSTDCompression.hpp"
#include "StarNetCompatibility.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
#include "StarIdMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarRpcPromise.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarLexicalCast.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"


#include "StarLuaRoot.hpp"
#include "StarLuaComponents.hpp"
#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
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
import star.host_address;
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
import star.uuid;
import star.warping;
import star.item_descriptor;
import star.quest_descriptor;
import star.ai_types;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.world_geometry;
import star.wiring;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
#include "StarLuaRoot.hpp"
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.entity_rendering;
import star.world;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;


import star.options_menu;
import star.widget_parsing;
import star.gui_reader;
import star.button_group;
import star.button_widget;
import star.image_widget;
import star.slider_bar;
import star.label_widget;



import star.keybindings_menu;




import star.base_script_pane;


import star.voice_settings_menu;






import star.bindings_menu;






import star.graphics_menu;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;

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
