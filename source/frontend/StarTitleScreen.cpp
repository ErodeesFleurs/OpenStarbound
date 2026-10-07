#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarEither.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarEncode.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarFont.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarArray.hpp"
#include "StarImageProcessing.hpp"
#include "StarLua.hpp"
#include "StarVersion.hpp"
#include "StarNetElementSystem.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarStrongTypedef.hpp"
#include "StarInterpolation.hpp"
#include "StarPerlin.hpp"
#include "StarRandomPoint.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarOrderedSet.hpp"
#include "StarZSTDCompression.hpp"
#include "StarNetCompatibility.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
#include "StarSectorArray2D.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarRpcPromise.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarAStar.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include <functional>

#include "StarLuaRoot.hpp"
#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
import star.host_address;
import star.chat_types;
import star.warping;
import star.item_descriptor;
import star.quest_descriptor;
import star.ai_types;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.worker_pool;
import star.tile_sector_array;
import star.world_layout;
import star.tile_modification;
import star.interaction_types;
import star.wiring;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.world_structure;
import star.chat_action;
import star.entity_rendering;
import star.world;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.weather;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.interface_cursor;
import star.ambient;
import star.title_screen;
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
import star.widget_parsing;
import star.gui_reader;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.actor_movement_controller;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.pane;
import star.game_timers;
import star.pane_manager;
import star.button_group;
import star.button_widget;
import star.list_widget;
import star.label_widget;

import star.uuid;


import star.char_selection;



import star.char_creation;
import star.text_box_widget;
import star.canvas_widget;
import star.widget_lua_bindings;

import star.configuration;

import star.registered_pane_manager;


import star.main_interface_types;


import star.options_menu;



import star.mods_menu;
import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.collision_block;
import star.liquid_types;
import star.collision_generator;
import star.world_tiles;
import star.entity_rendering_types;
import star.sky_render_data;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.environment_painter;
import star.sky_parameters;


import star.celestial_database;

import star.player_storage;
import star.sky;

namespace Star {

TitleScreen::TitleScreen(PlayerStoragePtr playerStorage, MixerPtr mixer, UniverseClientPtr client)
  : m_playerStorage(playerStorage), m_skipMultiPlayerConnection(false), m_mixer(mixer) {
  m_titleState = TitleState::Quit;

  auto assets = Root::singleton().assets();

  m_guiContext = GuiContext::singletonPtr();

  m_celestialDatabase = make_shared<CelestialMasterDatabase>();
  auto randomWorld = m_celestialDatabase->findRandomWorld(10, 50, [this](CelestialCoordinate const& coordinate) {
      return is<TerrestrialWorldParameters>(m_celestialDatabase->parameters(coordinate)->visitableParameters());
    }).take();

  if (auto name = m_celestialDatabase->name(randomWorld))
    Logger::info("Title world is {} @ CelestialWorld:{}", Text::stripEscapeCodes(*name), randomWorld);

  SkyParameters skyParameters(randomWorld, m_celestialDatabase);
  m_skyBackdrop = make_shared<Sky>(skyParameters, true);

  m_musicTrack = make_shared<AmbientNoisesDescription>(assets->json("/interface/windowconfig/title.config:music").toObject(), "/");

  initMainMenu();
  initCharSelectionMenu();
  initCharCreationMenu();
  initMultiPlayerMenu();
  initOptionsMenu(client);
  initModsMenu();

  resetState();
}

void TitleScreen::renderInit(RendererPtr renderer) {
  m_renderer = std::move(renderer);
  m_environmentPainter = make_shared<EnvironmentPainter>(m_renderer);
}

void TitleScreen::render() {
  auto assets = Root::singleton().assets();

  float pixelRatio = m_guiContext->interfaceScale();
  Vec2F screenSize = Vec2F(m_guiContext->windowSize());
  auto skyRenderData = m_skyBackdrop->renderData();

  float pixelRatioBasis = screenSize[1] / 1080.0f;
  float starAndDebrisRatio = lerp(0.0625f, pixelRatioBasis * 2.0f, pixelRatio);
  float orbiterAndPlanetRatio = lerp(0.125f, pixelRatioBasis * 3.0f, pixelRatio);

  m_environmentPainter->renderStars(starAndDebrisRatio, screenSize, skyRenderData);
  m_environmentPainter->renderDebrisFields(starAndDebrisRatio, screenSize, skyRenderData);
  m_environmentPainter->renderBackOrbiters(orbiterAndPlanetRatio, screenSize, skyRenderData);
  m_environmentPainter->renderPlanetHorizon(orbiterAndPlanetRatio, screenSize, skyRenderData);
  m_environmentPainter->renderSky(screenSize, skyRenderData);
  m_environmentPainter->renderFrontOrbiters(orbiterAndPlanetRatio, screenSize, skyRenderData);

  m_renderer->flush();

  auto skyBackdropDarken = jsonToColor(assets->json("/interface/windowconfig/title.config:skyBackdropDarken"));
  m_renderer->render(renderFlatRect(RectF(0, 0, windowWidth(), windowHeight()), skyBackdropDarken.toRgba(), 0.0f));

  m_renderer->flush();

  if (auto canvas = m_backgroundMenu->findChild("canvas")) {
    canvas->setPosition(Vec2I());
    canvas->setSize(Vec2I(m_guiContext->windowInterfaceSize()));
  }
  m_scriptComponent->invoke("render", JsonObject{{"interfaceScale", interfaceScale()}
  });

  m_renderer->flush();
  m_backgroundMenu->render(RectI(Vec2I(), Vec2I(m_guiContext->windowInterfaceSize())));
  m_paneManager.render();
  renderCursor();

  m_renderer->flush();
}

bool TitleScreen::handleInputEvent(InputEvent const& event) {
  if (auto mouseMove = event.ptr<MouseMoveEvent>())
    m_cursorScreenPos = Vec2I(mouseMove->mousePosition);

  if (event.is<KeyDownEvent>()) {
    if (GuiContext::singleton().actions(event).contains(InterfaceAction::TitleBack)) {
      back();
      return true;
    }
  }

  return m_paneManager.sendInputEvent(event);
}

void TitleScreen::update(float dt) {
  m_cursor.update(dt);

  for (auto p : m_rightAnchoredButtons)
    p.first->setPosition(Vec2I(m_guiContext->windowWidth() / m_guiContext->interfaceScale(), 0) + p.second);
  m_mainMenu->determineSizeFromChildren();
  m_backgroundMenu->determineSizeFromChildren();

  m_skyBackdrop->update(dt);
  m_environmentPainter->update(dt);

  m_backgroundMenu->update(dt);
  m_paneManager.update(dt);

  m_scriptComponent->update(dt);
  if (!finishedState()) {
    if (auto audioSample = m_musicTrackManager.updateAmbient(m_musicTrack, m_skyBackdrop->isDayTime())) {
      m_currentMusicTrack = audioSample;
      audioSample->setMixerGroup(MixerGroup::Music);
      audioSample->setLoops(0);
      m_mixer->play(audioSample);
    }
  }
}

bool TitleScreen::textInputActive() const {
  return m_paneManager.keyboardCapturedForTextInput();
}

TitleScreen::TitlePaneManager* TitleScreen::paneManager() {
  return &m_paneManager;
}

TitleState TitleScreen::currentState() const {
  return m_titleState;
}

bool TitleScreen::finishedState() const {
  switch (m_titleState) {
    case TitleState::StartSinglePlayer:
    case TitleState::StartMultiPlayer:
    case TitleState::Quit:
      return true;
    default:
      return false;
  }
}

void TitleScreen::resetState() {
  switchState(TitleState::Main);
  if (m_currentMusicTrack)
    m_currentMusicTrack->setVolume(1.0f, 4.0f);
}

void TitleScreen::goToMultiPlayerSelectCharacter(bool skipConnection) {
  m_skipMultiPlayerConnection = skipConnection;
  switchState(TitleState::MultiPlayerSelectCharacter);
}

void TitleScreen::stopMusic() {
  if (m_currentMusicTrack)
    m_currentMusicTrack->stop(8.0f);
}

PlayerPtr TitleScreen::currentlySelectedPlayer() const {
  return m_mainAppPlayer;
}

String TitleScreen::multiPlayerAddress() const {
  return m_connectionAddress;
}

void TitleScreen::setMultiPlayerAddress(String address) {
  m_multiPlayerMenu->fetchChild<TextBoxWidget>("address")->setText(address);
  m_connectionAddress = std::move(address);
}

String TitleScreen::multiPlayerPort() const {
  return m_connectionPort;
}

void TitleScreen::setMultiPlayerPort(String port) {
  m_multiPlayerMenu->fetchChild<TextBoxWidget>("port")->setText(port);
  m_connectionPort = std::move(port);
}

String TitleScreen::multiPlayerAccount() const {
  return m_account;
}

void TitleScreen::setMultiPlayerAccount(String account) {
  m_multiPlayerMenu->fetchChild<TextBoxWidget>("account")->setText(account);
  m_account = std::move(account);
}

String TitleScreen::multiPlayerPassword() const {
  return m_password;
}

void TitleScreen::setMultiPlayerPassword(String password) {
  m_multiPlayerMenu->fetchChild<TextBoxWidget>("password")->setText(password);
  m_password = std::move(password);
}

bool TitleScreen::multiPlayerForceLegacy() const {
  return m_forceLegacy;
}

void TitleScreen::setMultiPlayerForceLegacy(bool const& forceLegacy) {
  m_multiPlayerMenu->fetchChild<ButtonWidget>("legacyCheckbox")->setChecked(forceLegacy);
  m_forceLegacy = forceLegacy;
}

void TitleScreen::initMainMenu() {
  m_mainMenu = make_shared<Pane>();
  auto backMenu = make_shared<Pane>();

  auto assets = Root::singleton().assets();
  auto config = assets->json("/interface/windowconfig/title.config");

  StringMap<WidgetCallbackFunc> buttonCallbacks;
  buttonCallbacks["singleplayer"] = [this](Widget*) { switchState(TitleState::SinglePlayerSelectCharacter); };
  buttonCallbacks["multiplayer"] = [this](Widget*) { switchState(TitleState::MultiPlayerSelectCharacter); };
  buttonCallbacks["options"] = [this](Widget*) { switchState(TitleState::Options); };
  buttonCallbacks["quit"] = [this](Widget*) { switchState(TitleState::Quit); };
  buttonCallbacks["back"] = [this](Widget*) { back(); };
  buttonCallbacks["mods"] = [this](Widget*) { switchState(TitleState::Mods); };

  for (auto buttonConfig : config.getArray("mainMenuButtons")) {
    String key = buttonConfig.getString("key");
    String image = buttonConfig.getString("button");
    String imageHover = buttonConfig.getString("hover");
    Vec2I offset = jsonToVec2I(buttonConfig.get("offset"));
    WidgetCallbackFunc callback = buttonCallbacks.get(key);
    bool rightAnchored = buttonConfig.getBool("rightAnchored", false);

    auto button = make_shared<ButtonWidget>(callback, image, imageHover, "", "");
    button->setPosition(offset);

    if (rightAnchored)
      m_rightAnchoredButtons.append({button, offset});

    if (key == "back")
      backMenu->addChild(key, button);
    else
      m_mainMenu->addChild(key, button);
  }

  m_mainMenu->setAnchor(PaneAnchor::BottomLeft);
  m_mainMenu->lockPosition();

  backMenu->determineSizeFromChildren();
  backMenu->setAnchor(PaneAnchor::BottomLeft);
  backMenu->lockPosition();
  
  m_backgroundMenu = make_shared<Pane>();
  m_backgroundMenu->setAnchor(PaneAnchor::BottomLeft);
  m_backgroundMenu->lockPosition();
  m_backgroundMenu->addChild("canvas", make_shared<CanvasWidget>());
  m_backgroundMenu->show();

  m_paneManager.registerPane("mainMenu", PaneLayer::Hud, m_mainMenu);
  m_paneManager.registerPane("backMenu", PaneLayer::Hud, backMenu);

  m_scriptComponent = make_shared<ScriptComponent>();
  m_scriptComponent->setLuaRoot(make_shared<LuaRoot>());
  m_scriptComponent->addCallbacks("background", LuaBindings::makeWidgetCallbacks(m_backgroundMenu.get()));
  m_scriptComponent->addCallbacks("widget", LuaBindings::makeWidgetCallbacks(m_mainMenu.get()));
  m_scriptComponent->setScripts(jsonToStringList(config.getArray("scripts", JsonArray())));
  m_scriptComponent->init();
}

void TitleScreen::initCharSelectionMenu() {
  auto deleteDialog = make_shared<Pane>();

  GuiReader reader;

  reader.registerCallback("delete", [dialog = deleteDialog.get()](Widget*) { dialog->dismiss(); });
  reader.registerCallback("cancel", [dialog = deleteDialog.get()](Widget*) { dialog->dismiss(); });

  reader.construct(Root::singleton().assets()->json("/interface/windowconfig/deletedialog.config"), deleteDialog.get());

  auto charSelectionMenu = make_shared<CharSelectionPane>(m_playerStorage, [this]() {
      if (m_titleState == TitleState::SinglePlayerSelectCharacter)
        switchState(TitleState::SinglePlayerCreateCharacter);
      else if (m_titleState == TitleState::MultiPlayerSelectCharacter)
        switchState(TitleState::MultiPlayerCreateCharacter);
    }, [this](PlayerPtr mainPlayer) {
      m_mainAppPlayer = mainPlayer;
      m_playerStorage->moveToFront(m_mainAppPlayer->uuid());
      if (m_titleState == TitleState::SinglePlayerSelectCharacter) {
        switchState(TitleState::StartSinglePlayer);
        } else if (m_titleState == TitleState::MultiPlayerSelectCharacter) {
          if (m_skipMultiPlayerConnection)
            switchState(TitleState::StartMultiPlayer);
          else
            switchState(TitleState::MultiPlayerConnect);
        }
    }, [this](Uuid playerUuid) {
      auto deleteDialog = m_paneManager.registeredPane("deleteDialog");
      deleteDialog->fetchChild<ButtonWidget>("delete")->setCallback([this, playerUuid, dialog = deleteDialog.get()](Widget*) {
        m_playerStorage->deletePlayer(playerUuid);
        dialog->dismiss();
      });
      m_paneManager.displayRegisteredPane("deleteDialog");
    });
  charSelectionMenu->setAnchor(PaneAnchor::Center);
  charSelectionMenu->lockPosition();

  m_paneManager.registerPane("deleteDialog", PaneLayer::ModalWindow, deleteDialog, [=](PanePtr const&) {
      charSelectionMenu->updateCharacterPlates();
    });
  m_paneManager.registerPane("charSelectionMenu", PaneLayer::Hud, charSelectionMenu);
}

void TitleScreen::initCharCreationMenu() {
  auto charCreationMenu = make_shared<CharCreationPane>([this](PlayerPtr newPlayer) {
    if (newPlayer) {
      m_mainAppPlayer = newPlayer;
      m_playerStorage->savePlayer(m_mainAppPlayer);
      m_playerStorage->moveToFront(m_mainAppPlayer->uuid());
    }
    back();
  });
  charCreationMenu->setAnchor(PaneAnchor::Center);
  charCreationMenu->lockPosition();

  m_paneManager.registerPane("charCreationMenu", PaneLayer::Hud, charCreationMenu);
}


void TitleScreen::populateServerList(ListWidget* list){
  if (!m_serverList.isNull()) {
    list->clear();
    for (auto const& server : m_serverList.iterateArray()) {
      auto listItem = list->addItem();
      listItem->fetchChild<LabelWidget>("address")->setText(server.getString("address"));
      listItem->fetchChild<LabelWidget>("account")->setText(server.get("account", "").toString());
      listItem->setData(server);
    }
  }
};

void TitleScreen::initMultiPlayerMenu() {
  m_multiPlayerMenu = make_shared<Pane>();
  m_serverSelectPane = make_shared<Pane>();

  GuiReader readerConnect;
  GuiReader readerServer;

  m_serverList = Root::singleton().configuration()->get("serverList");
  if (!m_serverList.isType(Json::Type::Array))
    m_serverList = JsonArray();

  auto assets = Root::singleton().assets();

  readerServer.registerCallback("saveServer", [this](Widget*) {
    Json serverData = JsonObject{
      {"address", multiPlayerAddress()},
      {"account", multiPlayerAccount()},
      {"port", multiPlayerPort()},
      //{"password", multiPlayerPassword()},
      {"forceLegacy", multiPlayerForceLegacy()}
    };

    auto serverList = m_serverSelectPane->fetchChild<ListWidget>("serverSelectArea.serverList");
    if (auto const pos = serverList->selectedItem(); pos != NPos) { // Edit existing
      m_serverList = m_serverList.set(pos, serverData);
    } else { // Save new
      m_serverList = m_serverList.insert(0, serverData);
    }

    populateServerList(serverList.get());
    Root::singleton().configuration()->set("serverList", m_serverList);
  });

  readerServer.construct(assets->json("/interface/windowconfig/serverselect.config"), m_serverSelectPane.get());



  auto serverList = m_serverSelectPane->fetchChild<ListWidget>("serverSelectArea.serverList");
  
  serverList->registerMemberCallback("delete", [this, list = serverList.get()](Widget*) {
    if (auto const pos = list->selectedItem(); pos != NPos) {
      m_serverList = m_serverList.eraseIndex(pos);
    }
    populateServerList(list);
    Root::singleton().configuration()->set("serverList", m_serverList);
  });

  serverList->setCallback([this, list = serverList.get()](Widget*) {
    if (auto selectedItem = list->selectedWidget()) {
      if (selectedItem->findChild<ButtonWidget>("delete")->isHovered())
        return;
      auto& data = selectedItem->data();
      setMultiPlayerAddress(data.getString("address", ""));
      setMultiPlayerPort(data.getString("port", ""));
      setMultiPlayerAccount(data.getString("account", ""));
      setMultiPlayerPassword(data.getString("password", ""));
      setMultiPlayerForceLegacy(data.getBool("forceLegacy", false));

      if (auto passwordWidget = m_multiPlayerMenu->fetchChild("password"))
        passwordWidget->focus();
    }
  });

  readerConnect.registerCallback("address", [this](Widget* obj) {
      m_connectionAddress = convert<TextBoxWidget>(obj)->getText().trim();
      m_serverSelectPane->fetchChild<ButtonWidget>("save")->setVisibility(multiPlayerAddress().length() > 0);
    });

  readerConnect.registerCallback("port", [this](Widget* obj) {
      m_connectionPort = convert<TextBoxWidget>(obj)->getText().trim();
    });

  readerConnect.registerCallback("account", [this](Widget* obj) {
      m_account = convert<TextBoxWidget>(obj)->getText().trim();
    });

  readerConnect.registerCallback("password", [this](Widget* obj) {
      m_password = convert<TextBoxWidget>(obj)->getText().trim();
    });
  
  readerConnect.registerCallback("legacyCheckbox", [this](Widget* obj) {
      m_forceLegacy = convert<ButtonWidget>(obj)->isChecked();
    });

  readerConnect.registerCallback("connect", [this](Widget*) {
    switchState(TitleState::StartMultiPlayer);
    });


  readerConnect.construct(assets->json("/interface/windowconfig/multiplayer.config"), m_multiPlayerMenu.get());

  populateServerList(serverList.get());

  m_paneManager.registerPane("multiplayerMenu", PaneLayer::Hud, m_multiPlayerMenu);
  m_paneManager.registerPane("serverSelect", PaneLayer::Hud, m_serverSelectPane, [=](PanePtr const&) {
    serverList->clearSelected();
  });
}

void TitleScreen::initOptionsMenu(UniverseClientPtr client) {
  auto optionsMenu = make_shared<OptionsMenu>(&m_paneManager,client);
  optionsMenu->setAnchor(PaneAnchor::Center);
  optionsMenu->lockPosition();

  m_paneManager.registerPane("optionsMenu", PaneLayer::Hud, optionsMenu, [this](PanePtr const&) {
      back();
    });
}

void TitleScreen::initModsMenu() {
  auto modsMenu = make_shared<ModsMenu>();
  modsMenu->setAnchor(PaneAnchor::Center);
  modsMenu->lockPosition();

  m_paneManager.registerPane("modsMenu", PaneLayer::Hud, modsMenu, [this](PanePtr const&) {
      back();
    });
}

void TitleScreen::switchState(TitleState titleState) {
  if (m_titleState == titleState)
    return;

  m_paneManager.dismissAllPanes();
  m_titleState = titleState;

  // Clear the 'skip multi player connection' flag if we leave the multi player
  // menus
  if (m_titleState < TitleState::MultiPlayerSelectCharacter || m_titleState > TitleState::MultiPlayerConnect)
    m_skipMultiPlayerConnection = false;

  if (titleState == TitleState::Main) {
    m_paneManager.displayRegisteredPane("mainMenu");
  } else {
    m_paneManager.displayRegisteredPane("backMenu");

    if (titleState == TitleState::Options) {
      m_paneManager.displayRegisteredPane("optionsMenu");
    } if (titleState == TitleState::Mods) {
      m_paneManager.displayRegisteredPane("modsMenu");
    } else if (titleState == TitleState::SinglePlayerSelectCharacter) {
      m_paneManager.displayRegisteredPane("charSelectionMenu");
    } else if (titleState == TitleState::SinglePlayerCreateCharacter) {
      m_paneManager.displayRegisteredPane("charCreationMenu");
    } else if (titleState == TitleState::MultiPlayerSelectCharacter) {
      m_paneManager.displayRegisteredPane("charSelectionMenu");
    } else if (titleState == TitleState::MultiPlayerCreateCharacter) {
      m_paneManager.displayRegisteredPane("charCreationMenu");
    } else if (titleState == TitleState::MultiPlayerConnect) {
      m_paneManager.displayRegisteredPane("multiplayerMenu");
      m_paneManager.displayRegisteredPane("serverSelect");
      if (auto addressWidget = m_multiPlayerMenu->fetchChild("address"))
        addressWidget->focus();
    }
  }

  if (titleState == TitleState::Quit)
    m_musicTrackManager.cancelAll();
}

void TitleScreen::back() {
  if (m_titleState == TitleState::Options)
    switchState(TitleState::Main);
  else if (m_titleState == TitleState::Mods)
    switchState(TitleState::Main);
  else if (m_titleState == TitleState::SinglePlayerSelectCharacter)
    switchState(TitleState::Main);
  else if (m_titleState == TitleState::SinglePlayerCreateCharacter)
    switchState(TitleState::SinglePlayerSelectCharacter);
  else if (m_titleState == TitleState::MultiPlayerSelectCharacter)
    switchState(TitleState::Main);
  else if (m_titleState == TitleState::MultiPlayerCreateCharacter)
    switchState(TitleState::MultiPlayerSelectCharacter);
  else if (m_titleState == TitleState::MultiPlayerConnect)
    switchState(TitleState::MultiPlayerSelectCharacter);
}

void TitleScreen::renderCursor() {
  auto assets = Root::singleton().assets();

  Vec2I cursorPos = m_cursorScreenPos;
  Vec2I cursorSize = m_cursor.size();
  Vec2I cursorOffset = m_cursor.offset();
  float cursorScale = m_cursor.scale(interfaceScale());
  Drawable cursorDrawable = m_cursor.drawable();

  cursorPos[0] -= cursorOffset[0] * cursorScale;
  cursorPos[1] -= (cursorSize[1] - cursorOffset[1]) * cursorScale;

  if (!m_guiContext->trySetCursor(cursorDrawable, cursorOffset, cursorScale))
    m_guiContext->drawDrawable(cursorDrawable, Vec2F(cursorPos), cursorScale);
}

float TitleScreen::interfaceScale() const {
  return m_guiContext->interfaceScale();
}

unsigned TitleScreen::windowHeight() const {
  return m_guiContext->windowHeight();
}

unsigned TitleScreen::windowWidth() const {
  return m_guiContext->windowWidth();
}

}
