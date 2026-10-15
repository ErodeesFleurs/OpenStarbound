import star.buffer;
import star.json_extra;
import std;
import star.json;
import star.xx_hash;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.json_rpc;
import star.strong_typedef;
import star.vector;
import star.string;
import star.byte_array;
import star.encode;
import star.bytes;
import star.format;
import star.file;
import star.ordered_set;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.version;
import star.either;
import star.variant;
import star.weighted_pool;
import star.data_stream_devices;

import star.lexical_cast;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.id_map;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.btree_database;
import star.rpc_promise;
import star.image;
import star.a_star;
import star.periodic_function;

import star.lua_root;
import star.lua_actor_movement_component;
import star.item;
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
import star.configuration;
import star.root;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.lua_components;
import star.uuid;
import star.warping;
import star.quests;
import star.quest_manager;
import star.shell_parser;
import star.host_address;
import star.sky_types;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
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
import star.cinematic;
import star.client_command_processor;
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
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.player_tech;
import star.player_log;

import star.ai_types;
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
import star.animation;
import star.tech_database;



import star.ai_interface;



import star.quest_interface;
import star.input;



import star.player_inventory;
import star.statistics;

namespace Star {

ClientCommandProcessor::ClientCommandProcessor(std::shared_ptr<UniverseClient> universeClient, std::shared_ptr<Cinematic> cinematicOverlay,
  MainInterfacePaneManager* paneManager, StringMap<StringList> macroCommands)
  : m_universeClient(std::move(universeClient)), m_cinematicOverlay(std::move(cinematicOverlay)),
  m_paneManager(paneManager), m_macroCommands(std::move(macroCommands)) {
  m_builtinCommands = {
    {"reload", std::bind(&ClientCommandProcessor::reload, this)},
    {"hotReload", std::bind(&ClientCommandProcessor::hotReload, this)},
    {"whoami", std::bind(&ClientCommandProcessor::whoami, this)},
    {"gravity", std::bind(&ClientCommandProcessor::gravity, this)},
    {"debug", std::bind(&ClientCommandProcessor::debug, this, std::placeholders::_1)},
    {"boxes", std::bind(&ClientCommandProcessor::boxes, this)},
    {"fullbright", std::bind(&ClientCommandProcessor::fullbright, this)},
    {"asyncLighting", std::bind(&ClientCommandProcessor::asyncLighting, this)},
    {"setGravity", std::bind(&ClientCommandProcessor::setGravity, this, std::placeholders::_1)},
    {"resetGravity", std::bind(&ClientCommandProcessor::resetGravity, this)},
    {"fixedCamera", std::bind(&ClientCommandProcessor::fixedCamera, this)},
    {"monochromeLighting", std::bind(&ClientCommandProcessor::monochromeLighting, this)},
    {"radioMessage", std::bind(&ClientCommandProcessor::radioMessage, this, std::placeholders::_1)},
    {"clearRadioMessages", std::bind(&ClientCommandProcessor::clearRadioMessages, this)},
    {"clearCinematics", std::bind(&ClientCommandProcessor::clearCinematics, this)},
    {"startQuest", std::bind(&ClientCommandProcessor::startQuest, this, std::placeholders::_1)},
    {"completeQuest", std::bind(&ClientCommandProcessor::completeQuest, this, std::placeholders::_1)},
    {"failQuest", std::bind(&ClientCommandProcessor::failQuest, this, std::placeholders::_1)},
    {"previewNewQuest", std::bind(&ClientCommandProcessor::previewNewQuest, this, std::placeholders::_1)},
    {"previewQuestComplete", std::bind(&ClientCommandProcessor::previewQuestComplete, this, std::placeholders::_1)},
    {"previewQuestFailed", std::bind(&ClientCommandProcessor::previewQuestFailed, this, std::placeholders::_1)},
    {"clearScannedObjects", std::bind(&ClientCommandProcessor::clearScannedObjects, this)},
    {"played", std::bind(&ClientCommandProcessor::playTime, this)},
    {"deaths", std::bind(&ClientCommandProcessor::deathCount, this)},
    {"cinema", std::bind(&ClientCommandProcessor::cinema, this, std::placeholders::_1)},
    {"suicide", std::bind(&ClientCommandProcessor::suicide, this)},
    {"naked", std::bind(&ClientCommandProcessor::naked, this)},
    {"resetAchievements", std::bind(&ClientCommandProcessor::resetAchievements, this)},
    {"statistic", std::bind(&ClientCommandProcessor::statistic, this, std::placeholders::_1)},
    {"giveessentialitem", std::bind(&ClientCommandProcessor::giveEssentialItem, this, std::placeholders::_1)},
    {"maketechavailable", std::bind(&ClientCommandProcessor::makeTechAvailable, this, std::placeholders::_1)},
    {"enabletech", std::bind(&ClientCommandProcessor::enableTech, this, std::placeholders::_1)},
    {"upgradeship", std::bind(&ClientCommandProcessor::upgradeShip, this, std::placeholders::_1)},
    {"swap", std::bind(&ClientCommandProcessor::swap, this, std::placeholders::_1)},
    {"respawnInWorld", std::bind(&ClientCommandProcessor::respawnInWorld, this, std::placeholders::_1)},
    {"render", std::bind(&ClientCommandProcessor::render, this, std::placeholders::_1)}
  };
}

bool ClientCommandProcessor::adminCommandAllowed() const {
  return Root::singleton().configuration()->get("allowAdminCommandsFromAnyone").toBool() ||
    m_universeClient->mainPlayer()->isAdmin();
}

String ClientCommandProcessor::previewQuestPane(StringList const& arguments, std::function<std::shared_ptr<Pane>(std::shared_ptr<Quest>)> createPane) {
  Maybe<String> templateId = {};
  templateId = arguments[0];
  if (auto quest = createPreviewQuest(*templateId, arguments.at(1), arguments.at(2), m_universeClient->mainPlayer().get())) {
    auto pane = createPane(quest);
    m_paneManager->displayPane(PaneLayer::ModalWindow, pane);
    return "Previewed quest";
  }
  return "No such quest";
}

StringList ClientCommandProcessor::handleCommand(String const& commandLine, bool userInput) {
  Maybe<Input::ClipboardUnlock> unlock;
  if (userInput) // allow clipboard usage during this code
    unlock = Input::singleton().unlockClipboard();
  try {
    if (!commandLine.beginsWith("/"))
      throw StarException("ClientCommandProcessor expected command, does not start with '/'");

    String allArguments = commandLine.substr(1);
    String command = allArguments.extract();

    StringList result;
    if (auto builtinCommand = m_builtinCommands.maybe(command)) {
      result.append((*builtinCommand)(allArguments));
    } else if (auto macroCommand = m_macroCommands.maybe(command)) {
      for (auto const& c : *macroCommand) {
        if (c.beginsWith("/"))
          result.appendAll(handleCommand(c, false));
        else
          result.append(c);
      }
    } else {
      auto player = m_universeClient->mainPlayer();
      if (auto messageResult = player->receiveMessage(connectionForEntity(player->entityId()), "/" + command, {allArguments})) {
        if (messageResult->is<Json>()) {
          auto res = messageResult->ptr<Json>();
          if (res->isType(Json::Type::String))
            result.append(*res->stringPtr());
          else if (!res->isNull())
            result.append(res->repr(1, true));
        } else {
          Uuid uuid;
          m_commandPromises[uuid] = messageResult->get<RpcPromise<Json>>();
        }
      } else {
        m_universeClient->sendChat(commandLine, ChatSendMode::Broadcast);
      }
    }
    return result;
  } catch (ShellParsingException const& e) {
    Logger::error("Shell parsing exception: {}", outputException(e, false));
    return {"Shell parsing exception"};
  } catch (std::exception const& e) {
    Logger::error("Exception caught handling client command {}: {}", commandLine, outputException(e, true));
    return {strf("Exception caught handling client command {}", commandLine)};
  }
}

StringList ClientCommandProcessor::updatePromises() {
  StringList result;
  for (auto const& uuid : m_commandPromises.keys()) {
    if (m_commandPromises[uuid].finished()) {
      if (m_commandPromises[uuid].succeeded()) {
        auto res = m_commandPromises[uuid].result();
        if (res->isType(Json::Type::String))
          result.append(*res->stringPtr());
        else if (!res->isNull())
          result.append(res->repr(1, true));
      } else {
        result.append(strf("Error: {}", *m_commandPromises[uuid].error()));
      }
      m_commandPromises.remove(uuid);
    }
  }
  return result;
}

bool ClientCommandProcessor::debugDisplayEnabled() const {
  return m_debugDisplayEnabled;
}

bool ClientCommandProcessor::debugHudEnabled() const {
  return m_debugHudEnabled;
}

bool ClientCommandProcessor::fixedCameraEnabled() const {
  return m_fixedCameraEnabled;
}

String ClientCommandProcessor::reload() {
  Root::singleton().reload();
  return "Client Star::Root reloaded";
}

String ClientCommandProcessor::hotReload() {
  Root::singleton().hotReload();
  return "Hot-reloaded assets";
}

String ClientCommandProcessor::whoami() {
  return strf("Client: You are {}. You are {}an Admin.",
              m_universeClient->mainPlayer()->name(), m_universeClient->mainPlayer()->isAdmin() ? "" : "not ");
}

String ClientCommandProcessor::gravity() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return toString(m_universeClient->worldClient()->gravity(m_universeClient->mainPlayer()->position()));
}

String ClientCommandProcessor::debug(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!arguments.empty() && arguments.at(0).equalsIgnoreCase("hud")) {
    m_debugHudEnabled = !m_debugHudEnabled;
    return strf("Debug HUD {}", m_debugHudEnabled ? "enabled" : "disabled");
  }
  else {
    m_debugDisplayEnabled = !m_debugDisplayEnabled;
    return strf("Debug display {}", m_debugDisplayEnabled ? "enabled" : "disabled");
  }
}

String ClientCommandProcessor::boxes() {
  auto worldClient = m_universeClient->worldClient();
  bool state = !worldClient->collisionDebug();
  worldClient->setCollisionDebug(state);
  return strf("Geometry debug display {}", state ? "enabled" : "disabled");
}

String ClientCommandProcessor::fullbright() {
  auto worldClient = m_universeClient->worldClient();
  bool state = !worldClient->fullBright();
  worldClient->setFullBright(state);
  return strf("Fullbright render lighting {}", state ? "enabled" : "disabled");
}

String ClientCommandProcessor::asyncLighting() {
  auto worldClient = m_universeClient->worldClient();
  bool state = !worldClient->asyncLighting();
  worldClient->setAsyncLighting(state);
  return strf("Asynchronous render lighting {}", state ? "enabled" : "disabled");
}

String ClientCommandProcessor::setGravity(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->worldClient()->overrideGravity(lexicalCast<float>(arguments.at(0)));
  return strf("Gravity set to {} (This is client-side!)", arguments.at(0));
}

String ClientCommandProcessor::resetGravity() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->worldClient()->resetGravity();
  return "Gravity reset";
}

String ClientCommandProcessor::fixedCamera() {
  m_fixedCameraEnabled = !m_fixedCameraEnabled;
  return strf("Fixed camera {}", m_fixedCameraEnabled ? "enabled" : "disabled");
}

String ClientCommandProcessor::monochromeLighting() {
  bool monochrome = !Root::singleton().configuration()->get("monochromeLighting").toBool();
  Root::singleton().configuration()->set("monochromeLighting", monochrome);
  return strf("Monochrome lighting {}", monochrome ? "enabled" : "disabled");
}

String ClientCommandProcessor::radioMessage(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() != 1)
    return "Must provide one argument";

  m_universeClient->mainPlayer()->queueRadioMessage(arguments.at(0));
  return "Queued radio message";
}

String ClientCommandProcessor::clearRadioMessages() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->mainPlayer()->log()->clearRadioMessages();
  return "Player radio message records cleared!";
}

String ClientCommandProcessor::clearCinematics() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->mainPlayer()->log()->clearCinematics();
  return "Player cinematic records cleared!";
}

String ClientCommandProcessor::startQuest(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  auto questArc = QuestArcDescriptor::fromJson(Json::parseSequence(arguments.at(0)).get(0));
  m_universeClient->questManager()->offer(std::make_shared<Quest>(questArc, 0, m_universeClient->mainPlayer().get()));
  return "Quest started";
}

String ClientCommandProcessor::completeQuest(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->questManager()->getQuest(arguments.at(0))->complete();
  return strf("Quest {} complete", arguments.at(0));
}

String ClientCommandProcessor::failQuest(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->questManager()->getQuest(arguments.at(0))->fail();
  return strf("Quest {} failed", arguments.at(0));
}

String ClientCommandProcessor::previewNewQuest(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return previewQuestPane(arguments, [this](std::shared_ptr<Quest> const& quest) {
    return std::make_shared<NewQuestInterface>(m_universeClient->questManager(), quest, m_universeClient->mainPlayer());
  });
}

String ClientCommandProcessor::previewQuestComplete(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return previewQuestPane(arguments, [this](std::shared_ptr<Quest> const& quest) {
    return std::make_shared<QuestCompleteInterface>(quest, m_universeClient->mainPlayer(), std::shared_ptr<Cinematic>{});
  });
}

String ClientCommandProcessor::previewQuestFailed(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return previewQuestPane(arguments, [this](std::shared_ptr<Quest> const& quest) {
    return std::make_shared<QuestFailedInterface>(quest, m_universeClient->mainPlayer());
  });
}

String ClientCommandProcessor::clearScannedObjects() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->mainPlayer()->log()->clearScannedObjects();
  return "Player scanned objects cleared!";
}

String ClientCommandProcessor::playTime() {
  return strf("Total play time: {}", Time::printDuration(m_universeClient->mainPlayer()->log()->playTime()));
}

String ClientCommandProcessor::deathCount() {
  auto deaths = m_universeClient->mainPlayer()->log()->deathCount();
  return deaths ? strf("Total deaths: {}", deaths) : "Total deaths: 0. Well done!";
}

String ClientCommandProcessor::cinema(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_cinematicOverlay->load(Root::singleton().assets()->json(arguments.at(0)));
  if (arguments.size() > 1)
    m_cinematicOverlay->setTime(lexicalCast<float>(arguments.at(1)));
  return strf("Started cinematic {} at {}", arguments.at(0), arguments.size() > 1 ? arguments.at(1) : "beginning");
}

String ClientCommandProcessor::suicide() {
  m_universeClient->mainPlayer()->kill();
  return "You are now dead";
}

String ClientCommandProcessor::naked() {
  auto playerInventory = m_universeClient->mainPlayer()->inventory();
  for (auto slot : EquipmentSlotNames.leftValues())
    playerInventory->addItems(playerInventory->addToBags(playerInventory->takeSlot(slot)));
  return "You are now naked";
}

String ClientCommandProcessor::resetAchievements() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (m_universeClient->statistics()->reset()) {
    return "Achievements reset";
  }
  return "Unable to reset achievements";
}

String ClientCommandProcessor::statistic(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  StringList values;
  for (String const& statName : arguments) {
    values.append(strf("{} = {}", statName, m_universeClient->statistics()->stat(statName)));
  }
  return values.join("\n");
}

String ClientCommandProcessor::giveEssentialItem(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() < 2)
    return "Not enough arguments to /giveessentialitem";

  try {
    auto item = Root::singleton().itemDatabase()->item(ItemDescriptor(arguments.at(0)));
    auto slot = EssentialItemNames.getLeft(arguments.at(1));
    m_universeClient->mainPlayer()->inventory()->setEssentialItem(slot, item);
    return strf("Put {} in player slot {}", item->name(), arguments.at(1));
  } catch (MapException const& e) {
    return strf("Invalid essential item slot {}.", arguments.at(1));
  }
}

String ClientCommandProcessor::makeTechAvailable(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() == 0)
    return "Not enough arguments to /maketechavailable";

  m_universeClient->mainPlayer()->techs()->makeAvailable(arguments.at(0));
  return strf("Added {} to player's visible techs", arguments.at(0));
}

String ClientCommandProcessor::enableTech(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() == 0)
    return "Not enough arguments to /enabletech";

  m_universeClient->mainPlayer()->techs()->makeAvailable(arguments.at(0));
  m_universeClient->mainPlayer()->techs()->enable(arguments.at(0));
  return strf("Player tech {} enabled", arguments.at(0));
}

String ClientCommandProcessor::upgradeShip(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() == 0)
    return "Not enough arguments to /upgradeship";

  auto shipUpgrades = Json::parseJson(arguments.at(0));
  m_universeClient->rpcInterface()->invokeRemote("ship.applyShipUpgrades", shipUpgrades);
  return strf("Upgraded ship");
}

String ClientCommandProcessor::swap(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);

  if (arguments.size() == 0) {
    m_paneManager->displayRegisteredPane(MainInterfacePanes::CharacterSwap);
    return "";
  }

  if (m_universeClient->switchPlayer(arguments[0]))
    return "Successfully swapped player";
  else
    return "Failed to swap player";
}

String ClientCommandProcessor::respawnInWorld(String const& argumentsString) {
  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  auto worldClient = m_universeClient->worldClient();
  
  if (arguments.size() == 0)
    return strf("Respawn in this world is currently {}", worldClient->respawnInWorld() ? "true" : "false");

  bool respawnInWorld = Json::parse(arguments.at(0)).toBool();
  worldClient->setRespawnInWorld(respawnInWorld);
  return strf("Respawn in this world set to {} (This is client-side!)", respawnInWorld ? "true" : "false");
}

// Hardcoded render command, future version will write to the clipboard and possibly be implemented in Lua
String ClientCommandProcessor::render(String const& path) {
  if (path.empty()) {
    return "Specify a path to render an image, or for worn armor: "
           "^cyan;hat^reset;/^cyan;chest^reset;/^cyan;legs^reset;/^cyan;back^reset; "
           "or for body parts: "
           "^cyan;head^reset;/^cyan;body^reset;/^cyan;hair^reset;/^cyan;facialhair^reset;/"
           "^cyan;facialmask^reset;/^cyan;frontarm^reset;/^cyan;backarm^reset;/^cyan;emote^reset;";
  }
  AssetPath assetPath;
  bool outputSheet = false;
  String outputName = "render";
  auto player = m_universeClient->mainPlayer();
  if (player && path.utf8Size() < 100) {
    auto args = m_parser.tokenizeToStringList(path);
    auto first = args.maybeFirst().value().toLower();
    auto humanoid = player->humanoid();
    auto& identity = humanoid->identity();
    auto species = identity.imagePath.value(identity.species);
    outputSheet = true;
    outputName = first;
    if (first.equals("hat")) {
      assetPath.basePath = humanoid->headArmorFrameset();
      assetPath.directives += humanoid->headArmorDirectives();
    } else if (first.equals("chest")) {
      if (args.size() <= 1) {
        return "Chest armors have multiple spritesheets. Do: "
               "^white;/render chest ^cyan;front^reset;/^cyan;torso^reset;/^cyan;back^reset;. "
               "To repair old generated clothes, then also specify ^cyan;old^reset;.";
      }
      String sheet = args[1].toLower();
      outputName += " " + sheet;
      if (sheet == "torso") {
        assetPath.basePath = humanoid->chestArmorFrameset();
        assetPath.directives += humanoid->chestArmorDirectives();
      } else if (sheet == "front") {
        assetPath.basePath = humanoid->frontSleeveFrameset();
        assetPath.directives += humanoid->chestArmorDirectives();
      } else if (sheet == "back") {
        assetPath.basePath = humanoid->backSleeveFrameset();
        assetPath.directives += humanoid->chestArmorDirectives();
      } else {
        return strf("^red;Invalid chest sheet type '{}'^reset;", sheet);
      }
      // recovery for custom chests made by a very old generator
      if (args.size() > 2 && args[2].toLower() == "old" && assetPath.basePath.beginsWith("/items/armors/avian/avian-tier6separator/"))
          assetPath.basePath = "/items/armors/avian/avian-tier6separator/old/" + assetPath.basePath.substr(41);
    } else if (first.equals("legs")) {
      assetPath.basePath = humanoid->legsArmorFrameset();
      assetPath.directives += humanoid->legsArmorDirectives();
    } else if (first.equals("back")) {
      assetPath.basePath = humanoid->backArmorFrameset();
      assetPath.directives += humanoid->backArmorDirectives();
    } else if (first.equals("body")) {
      assetPath.basePath = humanoid->getBodyFromIdentity();
      assetPath.directives += identity.bodyDirectives;
    } else if (first.equals("head")) {
      outputSheet = false;
      assetPath.basePath = humanoid->getHeadFromIdentity();
      assetPath.subPath = String("normal");
      assetPath.directives += identity.bodyDirectives;
    } else if (first.equals("hair")) {
      outputSheet = false;
      assetPath.basePath = humanoid->getHairFromIdentity();
      assetPath.subPath = String("normal");
      assetPath.directives += identity.hairDirectives;
    } else if (first.equals("facialhair")) {
      outputSheet = false;
      assetPath.basePath = humanoid->getFacialHairFromIdentity();
      assetPath.subPath = String("normal");
      assetPath.directives += identity.facialHairDirectives;
    } else if (first.equals("facialmask")) {
      outputSheet = false;
      assetPath.basePath = humanoid->getFacialMaskFromIdentity();
      assetPath.subPath = String("normal");
      assetPath.directives += identity.facialMaskDirectives;
    } else if (first.equals("frontarm")) {
      assetPath.basePath = humanoid->getFrontArmFromIdentity();
      assetPath.directives += identity.bodyDirectives;
    } else if (first.equals("backarm")) {
      assetPath.basePath = humanoid->getBackArmFromIdentity();
      assetPath.directives += identity.bodyDirectives;
    } else if (first.equals("emote")) {
      assetPath.basePath = humanoid->getFacialEmotesFromIdentity();
      assetPath.directives += identity.emoteDirectives;
    } else {
      outputSheet = false;
      outputName = "render";
    }
  }
  if (assetPath == AssetPath()) {
    assetPath = AssetPath::split(path);
    if (!assetPath.basePath.beginsWith("/"))
      assetPath.basePath = "/assetmissing.png" + assetPath.basePath;
  }
  auto assets = Root::singleton().assets();
  std::shared_ptr<Image const> image;
  if (outputSheet) {
    auto sheet = std::make_shared<Image>(*assets->image(assetPath.basePath));
    sheet->convert(PixelFormat::RGBA32);
    AssetPath framePath = assetPath;

    StringMap<std::pair<RectU, std::shared_ptr<Image const>>> frames;
    if (auto imageFrames = assets->imageFrames(assetPath.basePath))
      for (auto& pair : imageFrames->frames)
        frames[pair.first] = std::make_pair(pair.second, std::shared_ptr<Image const>());

    if (frames.empty())
      return "^red;Failed to save image^reset;";

    for (auto& entry : frames) {
      framePath.subPath = entry.first;
      entry.second.second = assets->image(framePath);
    }

    Vec2U frameSize = frames.begin()->second.first.size();
    Vec2U imageSize = frames.begin()->second.second->size().piecewiseMin(Vec2U{256, 256});
    if (imageSize.min() == 0)
      return "^red;Resulting image is empty^reset;";

    for (auto& frame : frames) {
      RectU& box = frame.second.first;
      box.setXMin((box.xMin() / frameSize[0]) * imageSize[0]);
      box.setYMin(((sheet->height() - box.yMin() - box.height()) / frameSize[1]) * imageSize[1]);
      box.setXMax(box.xMin() + imageSize[0]);
      box.setYMax(box.yMin() + imageSize[1]);
    }

    if (frameSize != imageSize) {
      unsigned sheetWidth  = (sheet->width()  / frameSize[0]) * imageSize[0];
      unsigned sheetHeight = (sheet->height() / frameSize[0]) * imageSize[0];
      sheet->reset(sheetWidth, sheetHeight, PixelFormat::RGBA32);
    }

    for (auto& entry : frames)
      sheet->copyInto(entry.second.first.min(), *entry.second.second);

    image = std::move(sheet);
  } else {
    image = assets->image(assetPath);
  }
  if (image->size().min() == 0)
    return "^red;Resulting image is empty^reset;";
  auto outputDirectory = Root::singleton().toStoragePath("output");
  auto outputPath = File::relativeTo(outputDirectory, strf("{}.png", outputName));
  if (!File::isDirectory(outputDirectory))
    File::makeDirectory(outputDirectory);

  auto buffer = std::make_shared<Buffer>();
  image->writePng(buffer);
  auto file = File::open(outputPath, IOMode::Write | IOMode::Truncate);
  file->writeFull(buffer->ptr(), buffer->size());
  file->close();
  auto fullPath = File::fullPath(outputPath);
  GuiContext::singleton().setClipboardImage(*image, &buffer->data(), &fullPath);
  return strf("Saved '{}.png' ({}x{}) and copied to clipboard", outputName, image->width(), image->height());
}


}
