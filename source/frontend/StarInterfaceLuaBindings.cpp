module;
#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
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
#include "StarJsonExtra.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRpcPromise.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"
#include "StarEncode.hpp"
#include "StarBytes.hpp"
#include "StarFormat.hpp"
#include "StarTime.hpp"

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
#include "StarRoot.hpp"
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
#include "StarUniverseClient.hpp"
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
