#pragma once

#include "StarLua.hpp"
#include "StarGameTypes.hpp"
#include "StarRpcThreadPromise.hpp"

namespace Star {

class UniverseServer;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeUniverseServerCallbacks(UniverseServer& universe);

  namespace UniverseServerCallbacks {
    [[nodiscard]] Maybe<String> uuidForClient(UniverseServer& universe, ConnectionId arg1);
    [[nodiscard]] List<ConnectionId> clientIds(UniverseServer& universe);
    [[nodiscard]] size_t numberOfClients(UniverseServer& universe);
    [[nodiscard]] bool isConnectedClient(UniverseServer& universe, ConnectionId arg1);
    [[nodiscard]] String clientNick(UniverseServer& universe, ConnectionId arg1);
    [[nodiscard]] Maybe<ConnectionId> findNick(UniverseServer& universe, String const& arg1);
    void adminBroadcast(UniverseServer& universe, String const& arg1);
    void adminWhisper(UniverseServer& universe, ConnectionId arg1, String const& arg2);
    [[nodiscard]] bool isAdmin(UniverseServer& universe, ConnectionId arg1);
    [[nodiscard]] bool isPvp(UniverseServer& universe, ConnectionId arg1);
    void setPvp(UniverseServer& universe, ConnectionId arg1, Maybe<bool> arg2);
    [[nodiscard]] bool isWorldActive(UniverseServer& universe, String const& worldId);
    [[nodiscard]] StringList activeWorlds(UniverseServer& universe);
    [[nodiscard]] RpcThreadPromise<Json> sendWorldMessage(UniverseServer& universe, String const& worldId, String const& message, LuaVariadic<Json> args);
    [[nodiscard]] bool sendPacket(UniverseServer& universe, ConnectionId clientId, String const& packetTypeName, Json const& args);
    [[nodiscard]] String clientWorld(UniverseServer& universe, ConnectionId clientId);
    void disconnectClient(UniverseServer& universe, ConnectionId clientId, Maybe<String> const& reason);
    void banClient(UniverseServer& universe, ConnectionId clientId, Maybe<String> const& reason, bool banIp, bool banUuid, Maybe<int> timeout);
  }
}
}
