#include "StarUniverseServerLuaBindings.hpp"

#include "StarUniverseServer.hpp"

import std;

namespace Star {

auto LuaBindings::makeUniverseServerCallbacks(UniverseServer* universe) -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<std::optional<String>, ConnectionId>("uuidForClient", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::uuidForClient(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<List<ConnectionId>>("clientIds", [universe] -> List<ConnectionId> { return UniverseServerCallbacks::clientIds(universe); });
  callbacks.registerCallbackWithSignature<size_t>("numberOfClients", [universe] -> size_t { return UniverseServerCallbacks::numberOfClients(universe); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isConnectedClient", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::isConnectedClient(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientNick", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::clientNick(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<ConnectionId>, String>("findNick", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::findNick(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String>("adminBroadcast", [universe](auto&& PH1) -> auto { UniverseServerCallbacks::adminBroadcast(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, String>("adminWhisper", [universe](auto&& PH1, auto&& PH2) -> auto { UniverseServerCallbacks::adminWhisper(universe, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isAdmin", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::isAdmin(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isPvp", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::isPvp(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, bool>("setPvp", [universe](auto&& PH1, auto&& PH2) -> auto { UniverseServerCallbacks::setPvp(universe, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, String>("isWorldActive", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::isWorldActive(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<StringList>("activeWorlds", [universe] -> StringList { return UniverseServerCallbacks::activeWorlds(universe); });
  callbacks.registerCallbackWithSignature<RpcThreadPromise<Json>, String, String, LuaVariadic<Json>>("sendWorldMessage", [universe](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return UniverseServerCallbacks::sendWorldMessage(universe, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId, String, Json>("sendPacket", [universe](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return UniverseServerCallbacks::sendPacket(universe, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientWorld", [universe](auto&& PH1) -> auto { return UniverseServerCallbacks::clientWorld(universe, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, std::optional<String>>("disconnectClient", [universe](auto&& PH1, auto&& PH2) -> auto { UniverseServerCallbacks::disconnectClient(universe, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, std::optional<String>, bool, bool, std::optional<int>>("banClient", [universe](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5) -> auto { UniverseServerCallbacks::banClient(universe, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); });
  callbacks.registerCallback("warpClient", [universe](ConnectionId clientId, String action, std::optional<bool> deploy) -> void {
    universe->clientWarpPlayer(clientId, parseWarpAction(action), deploy.value_or(false));
  });

  return callbacks;
}

// Gets a list of client ids
//
// @return A list of numerical client IDs.
auto LuaBindings::UniverseServerCallbacks::uuidForClient(UniverseServer* universe, ConnectionId arg1) -> std::optional<String> {
  return universe->uuidForClient(arg1).transform([](Uuid const& str) -> String { return str.hex(); });
}

// Gets a list of client ids
//
// @return A list of numerical client IDs.
auto LuaBindings::UniverseServerCallbacks::clientIds(UniverseServer* universe) -> List<ConnectionId> {
  return universe->clientIds();
}

// Gets the number of logged in clients
//
// @return An integer containing the number of logged in clients
auto LuaBindings::UniverseServerCallbacks::numberOfClients(UniverseServer* universe) -> size_t {
  return universe->numberOfClients();
}

// Returns whether or not the provided client ID is currently connected
//
// @param clientId the client ID in question
// @return A bool that is true if the client is connected and false otherwise
auto LuaBindings::UniverseServerCallbacks::isConnectedClient(UniverseServer* universe, ConnectionId arg1) -> bool {
  return universe->isConnectedClient(arg1);
}

// Returns the nickname for the given client ID
//
// @param clientId the client ID in question
// @return A string containing the nickname of the given client
auto LuaBindings::UniverseServerCallbacks::clientNick(UniverseServer* universe, ConnectionId arg1) -> String {
  return universe->clientNick(arg1);
}

// Returns the client ID for the given nick
//
// @param nick the nickname of the client to search for
// @return An integer containing the clientID of the nick in question
auto LuaBindings::UniverseServerCallbacks::findNick(UniverseServer* universe, String const& arg1) -> std::optional<ConnectionId> {
  return universe->findNick(arg1);
}

// Sends a message to all logged in clients
//
// @param message the message to broadcast
// @return nil
void LuaBindings::UniverseServerCallbacks::adminBroadcast(UniverseServer* universe, String const& arg1) {
  universe->adminBroadcast(arg1);
}

// Sends a message to a specific client
//
// @param clientId the client id to whisper
// @param message the message to whisper
// @return nil
void LuaBindings::UniverseServerCallbacks::adminWhisper(UniverseServer* universe, ConnectionId arg1, String const& arg2) {
  ConnectionId client = arg1;
  String message = arg2;
  universe->adminWhisper(client, message);
}

// Returns whether or not a specific client is flagged as an admin
//
// @param clientId the client id to check
// @return a boolean containing true if the client is an admin, false otherwise
auto LuaBindings::UniverseServerCallbacks::isAdmin(UniverseServer* universe, ConnectionId arg1) -> bool {
  return universe->isAdmin(arg1);
}

// Returns whether or not a specific client is flagged as pvp
//
// @param clientId the client id to check
// @return a boolean containing true if the client is flagged as pvp, false
// otherwise
auto LuaBindings::UniverseServerCallbacks::isPvp(UniverseServer* universe, ConnectionId arg1) -> bool {
  return universe->isPvp(arg1);
}

// Set (or unset) the pvp status of a specific user
//
// @param clientId the client id to check
// @param setPvp set pvp status to this bool, defaults to true
// @return nil
void LuaBindings::UniverseServerCallbacks::setPvp(UniverseServer* universe, ConnectionId arg1, std::optional<bool> arg2) {
  ConnectionId client = arg1;
  bool setPvpTo = arg2.value_or(true);
  universe->setPvp(client, setPvpTo);
}

auto LuaBindings::UniverseServerCallbacks::isWorldActive(UniverseServer* universe, String const& worldId) -> bool {
  return universe->isWorldActive(parseWorldId(worldId));
}

auto LuaBindings::UniverseServerCallbacks::activeWorlds(UniverseServer* universe) -> StringList {
  return universe->activeWorlds().transformed(printWorldId);
}

auto LuaBindings::UniverseServerCallbacks::sendWorldMessage(UniverseServer* universe, String const& worldId, String const& message, LuaVariadic<Json> args) -> RpcThreadPromise<Json> {
  return universe->sendWorldMessage(parseWorldId(worldId), message, JsonArray::from(std::move(args)));
}

auto LuaBindings::UniverseServerCallbacks::sendPacket(UniverseServer* universe, ConnectionId clientId, String const& packetTypeName, Json const& args) -> bool {
  auto packetType = PacketTypeNames.getLeft(packetTypeName);
  auto packet = createPacket(packetType, args);
  return universe->sendPacket(clientId, packet);
}

auto LuaBindings::UniverseServerCallbacks::clientWorld(UniverseServer* universe, ConnectionId clientId) -> String {
  return printWorldId(universe->clientWorld(clientId));
}

void LuaBindings::UniverseServerCallbacks::disconnectClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason) {
  return universe->disconnectClient(clientId, reason.value());
}

void LuaBindings::UniverseServerCallbacks::banClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason, bool banIp, bool banUuid, std::optional<int> timeout) {
  return universe->banUser(clientId, reason.value(), std::make_pair(banIp, banUuid), timeout);
}

}// namespace Star
