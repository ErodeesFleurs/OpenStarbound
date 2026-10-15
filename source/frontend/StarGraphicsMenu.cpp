import std;
import star.json;
import star.xx_hash;
#include "StarJsonExtra.hpp"
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.ordered_map;
import star.either;
import star.variant;
import star.random;
import star.weighted_pool;
#include "StarStrongTypedef.hpp"
import star.byte_array;
#include "StarDataStreamDevices.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.format;
import star.id_map;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
#include "StarBTree.hpp"
import star.lru_cache;
import star.btree_database;
import star.rpc_promise;
import star.image;
import star.version;
import star.ordered_set;


#include "StarLuaRoot.hpp"
#include "StarLuaComponents.hpp"
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.application_controller;
import star.renderer;
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
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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


import star.graphics_menu;
import star.configuration;
import star.widget_parsing;
import star.gui_reader;
import star.list_widget;
import star.label_widget;
import star.button_group;
import star.button_widget;
import star.image_widget;
import star.slider_bar;




import star.base_script_pane;


import star.shaders_menu;

namespace Star {

GraphicsMenu::GraphicsMenu(PaneManager* manager,std::shared_ptr<UniverseClient> client)
  : m_paneManager(manager) {
  GuiReader reader;
  reader.registerCallback("cancel",
      [&](Widget*) {
        dismiss();
      });
  reader.registerCallback("accept",
      [&](Widget*) {
        apply();
        applyWindowSettings();
      });
  reader.registerCallback("resSlider", [this](Widget*) {
      Vec2U res = m_resList[fetchChild<SliderBarWidget>("resSlider")->val()];
      m_localChanges.set("fullscreenResolution", jsonFromVec2U(res));
      syncGui();
    });
  reader.registerCallback("interfaceScaleSlider", [this](Widget*) {
      auto interfaceScaleSlider = fetchChild<SliderBarWidget>("interfaceScaleSlider");
      m_localChanges.set("interfaceScale", m_interfaceScaleList[interfaceScaleSlider->val()]);
      syncGui();
    });
  reader.registerCallback("zoomSlider", [this](Widget*) {
      auto zoomSlider = fetchChild<SliderBarWidget>("zoomSlider");
      m_localChanges.set("zoomLevel", m_zoomList[zoomSlider->val()]);
      Root::singleton().configuration()->set("zoomLevel", m_zoomList[zoomSlider->val()]);
      syncGui();
    });
  reader.registerCallback("cameraSpeedSlider", [this](Widget*) {
      auto cameraSpeedSlider = fetchChild<SliderBarWidget>("cameraSpeedSlider");
      m_localChanges.set("cameraSpeedFactor", m_cameraSpeedList[cameraSpeedSlider->val()]);
      Root::singleton().configuration()->set("cameraSpeedFactor", m_cameraSpeedList[cameraSpeedSlider->val()]);
      syncGui();
    });
  reader.registerCallback("speechBubbleCheckbox", [this](Widget*) {
      auto button = fetchChild<ButtonWidget>("speechBubbleCheckbox");
      m_localChanges.set("speechBubbles", button->isChecked());
      Root::singleton().configuration()->set("speechBubbles", button->isChecked());
      syncGui();
    });
  reader.registerCallback("interactiveHighlightCheckbox", [this](Widget*) {
      auto button = fetchChild<ButtonWidget>("interactiveHighlightCheckbox");
      m_localChanges.set("interactiveHighlight", button->isChecked());
      Root::singleton().configuration()->set("interactiveHighlight", button->isChecked());
      syncGui();
    });
  reader.registerCallback("fullscreenCheckbox", [this](Widget*) {
      bool checked = fetchChild<ButtonWidget>("fullscreenCheckbox")->isChecked();
      m_localChanges.set("fullscreen", checked);
      if (checked)
        m_localChanges.set("borderless", !checked);
      syncGui();
    });
  reader.registerCallback("borderlessCheckbox", [this](Widget*) {
      bool checked = fetchChild<ButtonWidget>("borderlessCheckbox")->isChecked();
      m_localChanges.set("borderless", checked);
      if (checked)
        m_localChanges.set("fullscreen", !checked);
      syncGui();
    });
  reader.registerCallback("textureLimitCheckbox", [this](Widget*) {
      m_localChanges.set("limitTextureAtlasSize", fetchChild<ButtonWidget>("textureLimitCheckbox")->isChecked());
      syncGui();
    });
  reader.registerCallback("multiTextureCheckbox", [this](Widget*) {
      m_localChanges.set("useMultiTexturing", fetchChild<ButtonWidget>("multiTextureCheckbox")->isChecked());
      syncGui();
    });
  reader.registerCallback("antiAliasingCheckbox", [this](Widget*) {
    bool checked = fetchChild<ButtonWidget>("antiAliasingCheckbox")->isChecked();
    m_localChanges.set("antiAliasing", checked);
    Root::singleton().configuration()->set("antiAliasing", checked);
    syncGui();
  });
  reader.registerCallback("hardwareCursorCheckbox", [this](Widget*) {
    bool checked = fetchChild<ButtonWidget>("hardwareCursorCheckbox")->isChecked();
    m_localChanges.set("hardwareCursor", checked);
    Root::singleton().configuration()->set("hardwareCursor", checked);
    GuiContext::singleton().applicationController()->setCursorHardware(checked);
  });
  reader.registerCallback("monochromeCheckbox", [this](Widget*) {
      bool checked = fetchChild<ButtonWidget>("monochromeCheckbox")->isChecked();
      m_localChanges.set("monochromeLighting", checked);
      Root::singleton().configuration()->set("monochromeLighting", checked);
      syncGui();
    });
  reader.registerCallback("newLightingCheckbox", [this](Widget*) {
    bool checked = fetchChild<ButtonWidget>("newLightingCheckbox")->isChecked();
    m_localChanges.set("newLighting", checked);
    Root::singleton().configuration()->set("newLighting", checked);
    syncGui();
  });
  reader.registerCallback("hdrCheckbox", [this](Widget*) {
    bool checked = fetchChild<ButtonWidget>("hdrCheckbox")->isChecked();
    m_localChanges.set("hdr", checked);
    Root::singleton().configuration()->set("hdr", checked);
    syncGui();
  });
  reader.registerCallback("showShadersMenu", [this](Widget*) {
      displayShaders();
    });

  auto assets = Root::singleton().assets();

  auto config = assets->json("/interface/windowconfig/graphicsmenu.config");
  Json paneLayout = config.get("paneLayout");

  m_interfaceScaleList = jsonToFloatList(assets->json("/interface/windowconfig/graphicsmenu.config:interfaceScaleList"));
  m_resList = jsonToVec2UList(assets->json("/interface/windowconfig/graphicsmenu.config:resolutionList"));
  m_zoomList = jsonToFloatList(assets->json("/interface/windowconfig/graphicsmenu.config:zoomList"));
  m_cameraSpeedList = jsonToFloatList(assets->json("/interface/windowconfig/graphicsmenu.config:cameraSpeedList"));

  reader.construct(paneLayout, this);

  fetchChild<SliderBarWidget>("interfaceScaleSlider")->setRange(0, m_interfaceScaleList.size() - 1, 1);
  fetchChild<SliderBarWidget>("resSlider")->setRange(0, m_resList.size() - 1, 1);
  fetchChild<SliderBarWidget>("zoomSlider")->setRange(0, m_zoomList.size() - 1, 1);
  fetchChild<SliderBarWidget>("cameraSpeedSlider")->setRange(0, m_cameraSpeedList.size() - 1, 1);

  initConfig();
  syncGui();
  
  m_shadersMenu = std::make_shared<ShadersMenu>(assets->json(config.getString("shadersPanePath", "/interface/opensb/shaders/shaders.config")), client);
}

void GraphicsMenu::show() {
  Pane::show();
  initConfig();
  syncGui();
}

void GraphicsMenu::dismissed() {
  Pane::dismissed();
}

void GraphicsMenu::toggleFullscreen() {  
  bool fullscreen = m_localChanges.get("fullscreen").toBool();
  bool borderless = m_localChanges.get("borderless").toBool();

  m_localChanges.set("fullscreen", !(fullscreen || borderless));
  Root::singleton().configuration()->set("fullscreen", !(fullscreen || borderless));

  m_localChanges.set("borderless", false);
  Root::singleton().configuration()->set("borderless", false);

  applyWindowSettings();
  syncGui();
}

StringList const GraphicsMenu::ConfigKeys = {
  "fullscreenResolution",
  "interfaceScale",
  "zoomLevel",
  "cameraSpeedFactor",
  "speechBubbles",
  "interactiveHighlight",
  "fullscreen",
  "borderless",
  "limitTextureAtlasSize",
  "useMultiTexturing",
  "antiAliasing",
  "hardwareCursor",
  "monochromeLighting",
  "newLighting",
  "hdr"
};

void GraphicsMenu::initConfig() {
  auto configuration = Root::singleton().configuration();

  for (auto key : ConfigKeys) {
    m_localChanges.set(key, configuration->get(key));
  }
}

void GraphicsMenu::syncGui() {
  Vec2U res = jsonToVec2U(m_localChanges.get("fullscreenResolution"));
  auto resSlider = fetchChild<SliderBarWidget>("resSlider");
  auto resIt = std::lower_bound(m_resList.begin(), m_resList.end(), res, [&](Vec2U const& a, Vec2U const& b) {
      return a[0] * a[1] < b[0] * b[1]; // sort by number of pixels
    });
  if (resIt != m_resList.end()) {
    std::size_t resIndex = resIt - m_resList.begin();
    resIndex = std::min(resIndex, m_resList.size() - 1);
    resSlider->setVal(resIndex, false);
  } else {
    resSlider->setVal(m_resList.size() - 1);
  }
  fetchChild<LabelWidget>("resValueLabel")->setText(strf("{}x{}", res[0], res[1]));

  auto interfaceScaleSlider = fetchChild<SliderBarWidget>("interfaceScaleSlider");
  auto interfaceScale = m_localChanges.get("interfaceScale").optFloat().value();
  auto interfaceScaleIt = std::lower_bound(m_interfaceScaleList.begin(), m_interfaceScaleList.end(), interfaceScale);
  if (interfaceScaleIt != m_interfaceScaleList.end()) {
    std::size_t scaleIndex = interfaceScaleIt - m_interfaceScaleList.begin();
    interfaceScaleSlider->setVal(std::min(scaleIndex, m_interfaceScaleList.size() - 1), false);
  } else {
    interfaceScaleSlider->setVal(m_interfaceScaleList.size() - 1);
  }
  fetchChild<LabelWidget>("interfaceScaleValueLabel")->setText(interfaceScale != 0 ? toString(interfaceScale) : "AUTO");

  auto zoomSlider = fetchChild<SliderBarWidget>("zoomSlider");
  auto zoomLevel = m_localChanges.get("zoomLevel").toFloat();
  auto zoomIt = std::lower_bound(m_zoomList.begin(), m_zoomList.end(), zoomLevel);
  if (zoomIt != m_zoomList.end()) {
    std::size_t zoomIndex = zoomIt - m_zoomList.begin();
    zoomSlider->setVal(std::min(zoomIndex, m_zoomList.size() - 1), false);
  } else {
    zoomSlider->setVal(m_zoomList.size() - 1);
  }
  fetchChild<LabelWidget>("zoomValueLabel")->setText(strf("{}x", zoomLevel));

  auto cameraSpeedSlider = fetchChild<SliderBarWidget>("cameraSpeedSlider");
  auto cameraSpeedFactor = m_localChanges.get("cameraSpeedFactor").toFloat();
  auto speedIt = std::lower_bound(m_cameraSpeedList.begin(), m_cameraSpeedList.end(), cameraSpeedFactor);
  if (speedIt != m_cameraSpeedList.end()) {
    std::size_t speedIndex = speedIt - m_cameraSpeedList.begin();
    cameraSpeedSlider->setVal(std::min(speedIndex, m_cameraSpeedList.size() - 1), false);
  } else {
    cameraSpeedSlider->setVal(m_cameraSpeedList.size() - 1);
  }
  fetchChild<LabelWidget>("cameraSpeedValueLabel")->setText(strf("{}x", cameraSpeedFactor));

  fetchChild<ButtonWidget>("speechBubbleCheckbox")->setChecked(m_localChanges.get("speechBubbles").toBool());
  fetchChild<ButtonWidget>("interactiveHighlightCheckbox")->setChecked(m_localChanges.get("interactiveHighlight").toBool());
  fetchChild<ButtonWidget>("fullscreenCheckbox")->setChecked(m_localChanges.get("fullscreen").toBool());
  fetchChild<ButtonWidget>("borderlessCheckbox")->setChecked(m_localChanges.get("borderless").toBool());
  fetchChild<ButtonWidget>("textureLimitCheckbox")->setChecked(m_localChanges.get("limitTextureAtlasSize").toBool());
  fetchChild<ButtonWidget>("multiTextureCheckbox")->setChecked(m_localChanges.get("useMultiTexturing").optBool().value(true));
  fetchChild<ButtonWidget>("antiAliasingCheckbox")->setChecked(m_localChanges.get("antiAliasing").toBool());
  fetchChild<ButtonWidget>("monochromeCheckbox")->setChecked(m_localChanges.get("monochromeLighting").toBool());
  fetchChild<ButtonWidget>("newLightingCheckbox")->setChecked(m_localChanges.get("newLighting").optBool().value(true));
  fetchChild<ButtonWidget>("hardwareCursorCheckbox")->setChecked(m_localChanges.get("hardwareCursor").toBool());
  fetchChild<ButtonWidget>("hdrCheckbox")->setChecked(m_localChanges.get("hdr").optBool().value(true));
}

void GraphicsMenu::apply() {
  auto configuration = Root::singleton().configuration();
  for (auto p : m_localChanges) {
    configuration->set(p.first, p.second);
  }
}

void GraphicsMenu::displayShaders() {
  m_paneManager->displayPane(PaneLayer::ModalWindow, m_shadersMenu);
}

void GraphicsMenu::applyWindowSettings() {
  auto configuration = Root::singleton().configuration();
  auto appController = GuiContext::singleton().applicationController();
  if (configuration->get("fullscreen").toBool())
    appController->setFullscreenWindow(jsonToVec2U(configuration->get("fullscreenResolution")));
  else if (configuration->get("borderless").toBool())
    appController->setBorderlessWindow();
  else if (configuration->get("maximized").toBool())
    appController->setMaximizedWindow();
  else
    appController->setNormalWindow(jsonToVec2U(configuration->get("windowedResolution")));
}

}
