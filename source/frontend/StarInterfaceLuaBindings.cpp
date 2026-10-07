module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarLua.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
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
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRpcPromise.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"
#include "StarByteArray.hpp"
import star.encode;
#include "StarBytes.hpp"
#include "StarFormat.hpp"
import star.time;
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarEither.hpp"
#include "StarVariant.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarOrderedSet.hpp"
#include "StarDataStreamDevices.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
#include "StarNetCompatibility.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include <functional>
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarBTree.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
import star.btree_database;
#include "StarNetElementFloatFields.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
import star.application_controller;
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
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
import star.widget_parsing;
import star.gui_reader;
import star.widget_lua_bindings;
#include "StarLuaGameConverters.hpp"
import star.inventory_types;
import star.item_descriptor;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.container_entity;
import star.container_interactor;
import star.game_timers;
import star.inventory;
import star.animation;
import star.interface_cursor;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;
import star.uuid;
import star.warping;
import star.main_interface;


import star.pane;
#include "StarLuaComponents.hpp"


import star.base_script_pane;
import star.chat_types;


import star.chat;
import star.host_address;
import star.sky_types;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.ai_types;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.liquid_types;
import star.worker_pool;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.tile_modification;
import star.wiring;
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
import star.shell_parser;
#include "StarLuaRoot.hpp"
import star.cinematic;
import star.client_command_processor;

module star.interface_lua_bindings;
import star.client_context;

namespace Star {

LuaCallbacks LuaBindings::makeInterfaceCallbacks(MainInterface* mainInterface) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<bool>(
    "hudVisible", bind(mem_fn(&MainInterface::hudVisible), mainInterface));
  callbacks.registerCallbackWithSignature<void, bool>(
    "setHudVisible", bind(mem_fn(&MainInterface::setHudVisible), mainInterface, _1));

  callbacks.registerCallback("bindCanvas", [mainInterface](String const& canvasName, Maybe<bool> ignoreInterfaceScale) -> Maybe<CanvasWidgetPtr> {
    if (auto canvas = mainInterface->fetchCanvas(canvasName, ignoreInterfaceScale.value(false)))
      return canvas;
    return {};
  });

  callbacks.registerCallback("bindRegisteredPane", [mainInterface](String const& registeredPaneName) -> Maybe<LuaCallbacks> {
    if (auto pane = mainInterface->paneManager()->maybeRegisteredPane(MainInterfacePanesNames.getLeft(registeredPaneName)))
      return pane->makePaneCallbacks();
    return {};
  });

  callbacks.registerCallback("displayRegisteredPane", [mainInterface](String const& registeredPaneName) {
    auto pane = MainInterfacePanesNames.getLeft(registeredPaneName);
    auto paneManager = mainInterface->paneManager();
    if (paneManager->maybeRegisteredPane(pane))
      paneManager->displayRegisteredPane(pane);
  });

  callbacks.registerCallback("scale", []() {
    return GuiContext::singleton().interfaceScale();
  });

  callbacks.registerCallback("queueMessage", [mainInterface](String const& message, Maybe<float> cooldown, Maybe<float> springState) {
    mainInterface->queueMessage(message, cooldown, springState.value(0));
  });

  return callbacks;
}

LuaCallbacks LuaBindings::makeChatCallbacks(MainInterface* mainInterface, UniverseClient* client) {
  LuaCallbacks callbacks;

  auto chat = as<Chat>(mainInterface->paneManager()->registeredPane(MainInterfacePanes::Chat).get());

  callbacks.registerCallback("send", [client](String const& message, Maybe<String> modeName, Maybe<bool> speak, Maybe<JsonObject> data) {
    auto sendMode = modeName ? ChatSendModeNames.getLeft(*modeName) : ChatSendMode::Broadcast;
    client->sendChat(message, sendMode, speak, data);
  });

  // just for SE compat - this shoulda been a utility callback :moyai:
  callbacks.registerCallback("parseArguments", [](String const& args) -> LuaVariadic<Json> {
    return Json::parseSequence(args).toArray();
  });

  callbacks.registerCallback("command", [mainInterface](String const& command) -> StringList {
    return mainInterface->commandProcessor()->handleCommand(command);
  });

  callbacks.registerCallback("addMessage", [client, chat](String const& text, Maybe<Json> config) {
    ChatReceivedMessage message({MessageContext::Mode::CommandResult, ""}, client->clientContext()->connectionId(), "", text);
    if (config) {
      if (auto mode = config->optString("mode"))
        message.context.mode = MessageContextModeNames.getLeft(*mode);
      if (auto channelName = config->optString("channelName"))
        message.context.channelName = std::move(*channelName);
      if (auto portrait = config->optString("portrait"))
        message.portrait = std::move(*portrait);
      if (auto fromNick = config->optString("fromNick"))
        message.fromNick = std::move(*fromNick);
    }
    chat->addMessages({std::move(message)}, config ? config->getBool("showPane", true) : true);
  });

  callbacks.registerCallback("input", [chat]() -> String {
    return chat->currentChat();
  });

  callbacks.registerCallback("setInput", [chat](String const& text, Maybe<bool> moveCursor) -> bool {
    return chat->setCurrentChat(text, moveCursor.value(false));
  });

  callbacks.registerCallback("clear", [chat](Maybe<size_t> count) {
    chat->clear(count.value(std::numeric_limits<size_t>::max()));
  });

  return callbacks;
}

}
