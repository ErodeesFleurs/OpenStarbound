#pragma once

#include "StarGameTypes.hpp"
#include "StarLua.hpp"
#include "StarRpcThreadPromise.hpp"

import std;

namespace Star {

class UniverseServer;

namespace LuaBindings {
auto makeUniverseServerCallbacks(UniverseServer* universe) -> LuaCallbacks;

namespace UniverseServerCallbacks {
auto uuidForClient(UniverseServer* universe, ConnectionId arg1) -> std::optional<String>;
auto clientIds(UniverseServer* universe) -> List<ConnectionId>;
auto numberOfClients(UniverseServer* universe) -> size_t;
auto isConnectedClient(UniverseServer* universe, ConnectionId arg1) -> bool;
auto clientNick(UniverseServer* universe, ConnectionId arg1) -> String;
auto findNick(UniverseServer* universe, String const& arg1) -> std::optional<ConnectionId>;
void adminBroadcast(UniverseServer* universe, String const& arg1);
void adminWhisper(UniverseServer* universe, ConnectionId arg1, String const& arg2);
auto isAdmin(UniverseServer* universe, ConnectionId arg1) -> bool;
auto isPvp(UniverseServer* universe, ConnectionId arg1) -> bool;
void setPvp(UniverseServer* universe, ConnectionId arg1, std::optional<bool> arg2);
auto isWorldActive(UniverseServer* universe, String const& worldId) -> bool;
auto activeWorlds(UniverseServer* universe) -> StringList;
auto sendWorldMessage(UniverseServer* universe, String const& worldId, String const& message, LuaVariadic<Json> args) -> RpcThreadPromise<Json>;
auto sendPacket(UniverseServer* universe, ConnectionId clientId, String const& packetTypeName, Json const& args) -> bool;
auto clientWorld(UniverseServer* universe, ConnectionId clientId) -> String;
void disconnectClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason);
void banClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason, bool banIp, bool banUuid, std::optional<int> timeout);
}// namespace UniverseServerCallbacks
}// namespace LuaBindings
}// namespace Star
