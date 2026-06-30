#include "StarUniverseServerLuaBindings.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaGameConverters.hpp"
#include "StarUniverseServer.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeUniverseServerCallbacks(UniverseServer* universe) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Maybe<String>, ConnectionId>("uuidForClient", [universe](auto&&... args) { return UniverseServerCallbacks::uuidForClient(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<List<ConnectionId>>("clientIds", [universe]() { return UniverseServerCallbacks::clientIds(universe); });
  callbacks.registerCallbackWithSignature<size_t>("numberOfClients", [universe]() { return UniverseServerCallbacks::numberOfClients(universe); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isConnectedClient", [universe](auto&&... args) { return UniverseServerCallbacks::isConnectedClient(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientNick", [universe](auto&&... args) { return UniverseServerCallbacks::clientNick(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Maybe<ConnectionId>, String>("findNick", [universe](auto&&... args) { return UniverseServerCallbacks::findNick(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String>("adminBroadcast", [universe](auto&&... args) { UniverseServerCallbacks::adminBroadcast(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, String>("adminWhisper", [universe](auto&&... args) { UniverseServerCallbacks::adminWhisper(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isAdmin", [universe](auto&&... args) { return UniverseServerCallbacks::isAdmin(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isPvp", [universe](auto&&... args) { return UniverseServerCallbacks::isPvp(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, bool>("setPvp", [universe](auto&&... args) { UniverseServerCallbacks::setPvp(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String>("isWorldActive", [universe](auto&&... args) { return UniverseServerCallbacks::isWorldActive(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<StringList>("activeWorlds", [universe]() { return UniverseServerCallbacks::activeWorlds(universe); });
  callbacks.registerCallbackWithSignature<RpcThreadPromise<Json>, String, String, LuaVariadic<Json>>("sendWorldMessage", [universe](auto&&... args) { return UniverseServerCallbacks::sendWorldMessage(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, ConnectionId, String, Json>("sendPacket", [universe](auto&&... args) { return UniverseServerCallbacks::sendPacket(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientWorld", [universe](auto&&... args) { return UniverseServerCallbacks::clientWorld(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, Maybe<String>>("disconnectClient", [universe](auto&&... args) { UniverseServerCallbacks::disconnectClient(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, ConnectionId, Maybe<String>, bool, bool, Maybe<int>>("banClient", [universe](auto&&... args) { UniverseServerCallbacks::banClient(universe, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallback("warpClient", [universe](ConnectionId clientId, String action, Maybe<bool> deploy) {
    universe->clientWarpPlayer(clientId, parseWarpAction(action), deploy.value(false));
  });

  return callbacks;
}

// Gets a list of client ids
//
// @return A list of numerical client IDs.
Maybe<String> LuaBindings::UniverseServerCallbacks::uuidForClient(UniverseServer* universe, ConnectionId arg1) {
  return universe->uuidForClient(arg1).apply([](Uuid const& str) { return str.hex(); });
}

// Gets a list of client ids
//
// @return A list of numerical client IDs.
List<ConnectionId> LuaBindings::UniverseServerCallbacks::clientIds(UniverseServer* universe) {
  return universe->clientIds();
}

// Gets the number of logged in clients
//
// @return An integer containing the number of logged in clients
size_t LuaBindings::UniverseServerCallbacks::numberOfClients(UniverseServer* universe) {
  return universe->numberOfClients();
}

// Returns whether or not the provided client ID is currently connected
//
// @param clientId the client ID in question
// @return A bool that is true if the client is connected and false otherwise
bool LuaBindings::UniverseServerCallbacks::isConnectedClient(UniverseServer* universe, ConnectionId arg1) {
  return universe->isConnectedClient(arg1);
}

// Returns the nickname for the given client ID
//
// @param clientId the client ID in question
// @return A string containing the nickname of the given client
String LuaBindings::UniverseServerCallbacks::clientNick(UniverseServer* universe, ConnectionId arg1) {
  return universe->clientNick(arg1);
}

// Returns the client ID for the given nick
//
// @param nick the nickname of the client to search for
// @return An integer containing the clientID of the nick in question
Maybe<ConnectionId> LuaBindings::UniverseServerCallbacks::findNick(UniverseServer* universe, String const& arg1) {
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
bool LuaBindings::UniverseServerCallbacks::isAdmin(UniverseServer* universe, ConnectionId arg1) {
  return universe->isAdmin(arg1);
}

// Returns whether or not a specific client is flagged as pvp
//
// @param clientId the client id to check
// @return a boolean containing true if the client is flagged as pvp, false
// otherwise
bool LuaBindings::UniverseServerCallbacks::isPvp(UniverseServer* universe, ConnectionId arg1) {
  return universe->isPvp(arg1);
}

// Set (or unset) the pvp status of a specific user
//
// @param clientId the client id to check
// @param setPvp set pvp status to this bool, defaults to true
// @return nil
void LuaBindings::UniverseServerCallbacks::setPvp(UniverseServer* universe, ConnectionId arg1, Maybe<bool> arg2) {
  ConnectionId client = arg1;
  bool setPvpTo = arg2.value(true);
  universe->setPvp(client, setPvpTo);
}

bool LuaBindings::UniverseServerCallbacks::isWorldActive(UniverseServer* universe, String const& worldId) {
  return universe->isWorldActive(parseWorldId(worldId));
}

StringList LuaBindings::UniverseServerCallbacks::activeWorlds(UniverseServer* universe) {
  return universe->activeWorlds().transformed(printWorldId);
}

RpcThreadPromise<Json> LuaBindings::UniverseServerCallbacks::sendWorldMessage(UniverseServer* universe, String const& worldId, String const& message, LuaVariadic<Json> args) {
  return universe->sendWorldMessage(parseWorldId(worldId), message, JsonArray::from(std::move(args)));
}

bool LuaBindings::UniverseServerCallbacks::sendPacket(UniverseServer* universe, ConnectionId clientId, String const& packetTypeName, Json const& args) {
  auto packetType = PacketTypeNames.getLeft(packetTypeName);
  auto packet = createPacket(packetType, args);
  return universe->sendPacket(clientId, packet);
}

String LuaBindings::UniverseServerCallbacks::clientWorld(UniverseServer* universe, ConnectionId clientId) {
  return printWorldId(universe->clientWorld(clientId));
}

void LuaBindings::UniverseServerCallbacks::disconnectClient(UniverseServer* universe, ConnectionId clientId, Maybe<String> const& reason) {
  return universe->disconnectClient(clientId, reason.value());
}

void LuaBindings::UniverseServerCallbacks::banClient(UniverseServer* universe, ConnectionId clientId, Maybe<String> const& reason, bool banIp, bool banUuid, Maybe<int> timeout) {
  return universe->banUser(clientId, reason.value(), make_pair(banIp, banUuid), timeout);
}

}
