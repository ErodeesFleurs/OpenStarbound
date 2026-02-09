#pragma once

#include "StarAlgorithm.hpp"
#include "StarConfig.hpp"
#include "StarHostAddress.hpp"
#include "StarPlatformServices_pc.hpp"
#include "StarRpcPromise.hpp"
#include "StarStrongTypedef.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

class PcP2PNetworkingService : public P2PNetworkingService {
public:
  PcP2PNetworkingService(Ptr<PcPlatformServicesState> state);
  ~PcP2PNetworkingService() override;

  void setJoinUnavailable() override;
  void setJoinLocal(std::uint32_t capacity) override;
  void setJoinRemote(HostAddressWithPort location) override;
  void setActivityData(const char* title, const char* details, std::int64_t startTime, std::optional<std::pair<std::uint16_t, std::uint16_t>>) override;

  auto pullPendingJoin() -> MVariant<P2PNetworkingPeerId, HostAddressWithPort> override;
  auto pullJoinRequest() -> std::optional<std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> override;

  void setAcceptingP2PConnections(bool acceptingP2PConnections) override;
  auto acceptP2PConnections() -> List<UPtr<P2PSocket>> override;
  void update() override;
  auto connectToPeer(P2PNetworkingPeerId peerId) -> Either<String, UPtr<P2PSocket>> override;

  void addPendingJoin(String connectionString);

private:
  using JoinUnavailable = StrongTypedef<Empty>;
  struct JoinLocal {
    auto operator==(JoinLocal const& rhs) const -> bool { return capacity == rhs.capacity; };
    std::uint32_t capacity;
  };
  using JoinRemote = StrongTypedef<HostAddressWithPort>;
  using JoinLocation = Variant<JoinUnavailable, JoinLocal, JoinRemote>;

#ifdef STAR_ENABLE_STEAM_INTEGRATION

  struct SteamP2PSocket : P2PSocket {
    SteamP2PSocket() = default;
    ~SteamP2PSocket();

    bool isOpen() override;
    bool sendMessage(ByteArray const& message) override;
    std::optional<ByteArray> receiveMessage() override;

    Mutex mutex;
    PcP2PNetworkingService* parent = nullptr;
    CSteamID steamId = CSteamID();
    Deque<ByteArray> incoming;
    bool connected = false;
  };

  unique_ptr<SteamP2PSocket> createSteamP2PSocket(CSteamID steamId);

  STEAM_CALLBACK(PcP2PNetworkingService, steamOnConnectionFailure, P2PSessionConnectFail_t, m_callbackConnectionFailure);
  STEAM_CALLBACK(PcP2PNetworkingService, steamOnJoinRequested, GameRichPresenceJoinRequested_t, m_callbackJoinRequested);
  STEAM_CALLBACK(PcP2PNetworkingService, steamOnSessionRequest, P2PSessionRequest_t, m_callbackSessionRequest);

  void steamCloseSocket(SteamP2PSocket* socket);
  void steamReceiveAll();

#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION

  enum class DiscordSocketMode {
    Startup,
    Connected,
    Disconnected
  };

  struct DiscordP2PSocket : P2PSocket {
    DiscordP2PSocket() = default;
    ~DiscordP2PSocket();

    bool isOpen() override;
    bool sendMessage(ByteArray const& message) override;
    std::optional<ByteArray> receiveMessage() override;

    Mutex mutex;
    PcP2PNetworkingService* parent = nullptr;
    DiscordSocketMode mode = DiscordSocketMode::Disconnected;
    discord::LobbyId lobbyId = {};
    discord::UserId remoteUserId;
    Deque<ByteArray> incoming;
  };

  P2PSocketUPtr discordConnectRemote(discord::UserId remoteUserId, discord::LobbyId lobbyId, String const& lobbySecret);
  void discordCloseSocket(DiscordP2PSocket* socket);

  void discordOnReceiveMessage(discord::LobbyId lobbyId, discord::UserId userId, discord::NetworkChannelId channel, uint8_t* data, uint32_t size);
  void discordOnLobbyMemberConnect(discord::LobbyId lobbyId, discord::UserId userId);
  void discordOnLobbyMemberUpdate(discord::LobbyId lobbyId, discord::UserId userId);
  void discordOnLobbyMemberDisconnect(discord::LobbyId lobbyId, discord::UserId userId);

#endif

  void setJoinLocation(JoinLocation joinLocation);

  Ptr<PcPlatformServicesState> m_state;

  Mutex m_mutex;
  JoinLocation m_joinLocation;
  bool m_acceptingP2PConnections = false;
  List<UPtr<P2PSocket>> m_pendingIncomingConnections;
  MVariant<P2PNetworkingPeerId, HostAddressWithPort> m_pendingJoin;

#ifdef STAR_ENABLE_STEAM_INTEGRATION

  HashMap<uint64, SteamP2PSocket*> m_steamOpenSockets;

#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION

  List<pair<discord::UserId, String>> m_discordJoinRequests;
  List<pair<discord::UserId, RpcPromise<P2PJoinRequestReply>>> m_pendingDiscordJoinRequests;

  HashMap<discord::UserId, DiscordP2PSocket*> m_discordOpenSockets;
  String m_discordActivityTitle;
  String m_discordActivityDetails;
  int64_t m_discordActivityStartTime = 0;
  std::optional<pair<uint16_t, uint16_t>> m_discordPartySize;
  bool m_discordForceUpdateActivity = false;
  bool m_discordUpdatingActivity = false;
  std::optional<pair<discord::LobbyId, String>> m_discordServerLobby = {};

  int m_discordOnActivityJoinToken = 0;
  int m_discordOnActivityRequestToken = 0;
  int m_discordOnReceiveMessage = 0;
  int m_discordOnLobbyMemberConnect = 0;
  int m_discordOnLobbyMemberUpdate = 0;
  int m_discordOnLobbyMemberDisconnect = 0;

#endif
};

}// namespace Star
