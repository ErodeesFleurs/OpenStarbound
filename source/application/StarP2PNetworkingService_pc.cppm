module;
#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#endif
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import std;
import star.format;
import star.byte_array;

import star.string;
import star.list;
import star.map;
import star.thread;
import star.input_event;
import star.application;
import star.host_address;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
import star.steam_dispatch;
#endif

import star.platform_services_pc;
import star.algorithm;
import star.strong_typedef;
import star.rpc_promise;

namespace Star {

class PcP2PNetworkingService : public P2PNetworkingService {
public:
  PcP2PNetworkingService(std::shared_ptr<PcPlatformServicesState> state);
  ~PcP2PNetworkingService();

  [[nodiscard]] std::expected<void, StarException> setJoinUnavailable() override;
  [[nodiscard]] std::expected<void, StarException> setJoinLocal(std::uint32_t capacity) override;
  [[nodiscard]] std::expected<void, StarException> setJoinRemote(HostAddressWithPort location) override;
  [[nodiscard]] std::expected<void, StarException> setActivityData(const char* title, const char* details, std::int64_t startTime, std::optional<std::pair<std::uint16_t, std::uint16_t>>) override;

  std::variant<std::monostate, P2PNetworkingPeerId, HostAddressWithPort> pullPendingJoin() override;
  std::optional<std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> pullJoinRequest() override;

  void setAcceptingP2PConnections(bool acceptingP2PConnections) override;
  List<std::unique_ptr<P2PSocket>> acceptP2PConnections() override;
  [[nodiscard]] std::expected<void, StarException> update() override;
  [[nodiscard]] std::expected<std::unique_ptr<P2PSocket>, StarException> connectToPeer(P2PNetworkingPeerId peerId) override;

  [[nodiscard]] std::expected<void, StarException> addPendingJoin(String connectionString);

private:
  template <typename BaseType>
  struct JoinUnavailableWrapper : BaseType {
    using BaseType::BaseType;

    JoinUnavailableWrapper() : BaseType() {}

    JoinUnavailableWrapper(JoinUnavailableWrapper const& nt) : BaseType(nt) {}

    JoinUnavailableWrapper(JoinUnavailableWrapper&& nt) : BaseType(std::move(nt)) {}

    explicit JoinUnavailableWrapper(BaseType const& bt) : BaseType(bt) {}

    explicit JoinUnavailableWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

    JoinUnavailableWrapper& operator=(JoinUnavailableWrapper const& rhs) {
      BaseType::operator=(rhs);
      return *this;
    }

    JoinUnavailableWrapper& operator=(JoinUnavailableWrapper&& rhs) {
      BaseType::operator=(std::move(rhs));
      return *this;
    }

    template <class Arg>
    JoinUnavailableWrapper& operator=(Arg&& other) {
      static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
              || std::is_same<JoinUnavailableWrapper, typename std::decay<Arg>::type>::value,
          "JoinUnavailable can not implicitly be assigned from Empty-derived classes or strong Empty typedefs");

      BaseType::operator=(std::forward<Arg>(other));
      return *this;
    }
  };
  typedef JoinUnavailableWrapper<Empty> JoinUnavailable;
  struct JoinLocal {
    bool operator==(JoinLocal const& rhs) const { return capacity == rhs.capacity; };
    std::uint32_t capacity;
  };
  template <typename BaseType>
  struct JoinRemoteWrapper : BaseType {
    using BaseType::BaseType;

    JoinRemoteWrapper() : BaseType() {}

    JoinRemoteWrapper(JoinRemoteWrapper const& nt) : BaseType(nt) {}

    JoinRemoteWrapper(JoinRemoteWrapper&& nt) : BaseType(std::move(nt)) {}

    explicit JoinRemoteWrapper(BaseType const& bt) : BaseType(bt) {}

    explicit JoinRemoteWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

    JoinRemoteWrapper& operator=(JoinRemoteWrapper const& rhs) {
      BaseType::operator=(rhs);
      return *this;
    }

    JoinRemoteWrapper& operator=(JoinRemoteWrapper&& rhs) {
      BaseType::operator=(std::move(rhs));
      return *this;
    }

    template <class Arg>
    JoinRemoteWrapper& operator=(Arg&& other) {
      static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
              || std::is_same<JoinRemoteWrapper, typename std::decay<Arg>::type>::value,
          "JoinRemote can not implicitly be assigned from HostAddressWithPort-derived classes or strong HostAddressWithPort typedefs");

      BaseType::operator=(std::forward<Arg>(other));
      return *this;
    }
  };
  typedef JoinRemoteWrapper<HostAddressWithPort> JoinRemote;
  typedef std::variant<JoinUnavailable, JoinLocal, JoinRemote> JoinLocation;

#ifdef STAR_ENABLE_STEAM_INTEGRATION

  struct SteamP2PSocket : P2PSocket {
    SteamP2PSocket() = default;
    ~SteamP2PSocket();

    bool isOpen() override;
    [[nodiscard]] std::expected<bool, StarException> sendMessage(ByteArray const& message) override;
    [[nodiscard]] std::expected<std::optional<ByteArray>, StarException> receiveMessage() override;

    Mutex mutex;
    std::optional<StarException> error;
    PcP2PNetworkingService* parent = nullptr;
    CSteamID steamId = CSteamID();
    Deque<ByteArray> incoming;
    bool connected = false;
  };

  std::unique_ptr<SteamP2PSocket> createSteamP2PSocket(CSteamID steamId);

  std::expected<void, StarException> steamOnConnectionFailure(P2PSessionConnectFail_t* callback);
  std::expected<void, StarException> steamOnJoinRequested(GameRichPresenceJoinRequested_t* callback);
  std::expected<void, StarException> steamOnSessionRequest(P2PSessionRequest_t* callback);

  void steamCloseSocket(SteamP2PSocket* socket);
  std::expected<void, StarException> steamReceiveAll();

#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION

  // Core dispatch, token access and service teardown are serialized by
  // PcPlatformServicesState::discordMutex (always before m_mutex).
  // SDK callbacks retain only this token, never the service or its Core owner.
  struct DiscordCallbackLifetime {
    PcP2PNetworkingService* service = nullptr;
  };

  enum class DiscordSocketMode {
    Startup,
    Connected,
    Disconnected
  };

  struct DiscordP2PSocket : P2PSocket {
    DiscordP2PSocket() = default;
    ~DiscordP2PSocket();

    bool isOpen() override;
    [[nodiscard]] std::expected<bool, StarException> sendMessage(ByteArray const& message) override;
    [[nodiscard]] std::expected<std::optional<ByteArray>, StarException> receiveMessage() override;

    Mutex mutex;
    std::optional<StarException> error;
    std::shared_ptr<PcPlatformServicesState> state;
    std::shared_ptr<DiscordCallbackLifetime> lifetime;
    std::uint64_t connectionGeneration = 0;
    DiscordSocketMode mode = DiscordSocketMode::Disconnected;
    discord::LobbyId lobbyId = {};
    discord::UserId remoteUserId;
    Deque<ByteArray> incoming;
  };

  std::expected<std::unique_ptr<P2PSocket>, StarException> discordConnectRemote(discord::UserId remoteUserId, discord::LobbyId lobbyId, String const& lobbySecret);
  void discordCloseSocket(DiscordP2PSocket* socket);
  void discordDisconnectCallbacks();

  void discordOnReceiveMessage(discord::LobbyId lobbyId, discord::UserId userId, discord::NetworkChannelId channel, std::uint8_t* data, std::uint32_t size);
  void discordOnLobbyMemberConnect(discord::LobbyId lobbyId, discord::UserId userId);
  void discordOnLobbyMemberUpdate(discord::LobbyId lobbyId, discord::UserId userId);
  void discordOnLobbyMemberDisconnect(discord::LobbyId lobbyId, discord::UserId userId);

#endif
  
  std::expected<void, StarException> setJoinLocation(JoinLocation joinLocation);

  std::shared_ptr<PcPlatformServicesState> m_state;
#ifdef STAR_ENABLE_STEAM_INTEGRATION
  SteamCallback<PcP2PNetworkingService, P2PSessionConnectFail_t> m_callbackConnectionFailure;
  SteamCallback<PcP2PNetworkingService, GameRichPresenceJoinRequested_t> m_callbackJoinRequested;
  SteamCallback<PcP2PNetworkingService, P2PSessionRequest_t> m_callbackSessionRequest;
#endif

  Mutex m_mutex;
  JoinLocation m_joinLocation;
  bool m_acceptingP2PConnections = false;
  List<std::unique_ptr<P2PSocket>> m_pendingIncomingConnections;
  std::variant<std::monostate, P2PNetworkingPeerId, HostAddressWithPort> m_pendingJoin;

#ifdef STAR_ENABLE_STEAM_INTEGRATION

  HashMap<uint64, SteamP2PSocket*> m_steamOpenSockets;

#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION

  std::shared_ptr<DiscordCallbackLifetime> m_discordLifetime = std::make_shared<DiscordCallbackLifetime>();
  std::uint64_t m_discordLobbyGeneration = 0;
  std::uint64_t m_discordConnectionGeneration = 0;
  
  List<std::pair<discord::UserId, String>> m_discordJoinRequests;
  List<std::pair<discord::UserId, RpcPromise<P2PJoinRequestReply>>> m_pendingDiscordJoinRequests;

  HashMap<discord::UserId, DiscordP2PSocket*> m_discordOpenSockets;
  String m_discordActivityTitle;
  String m_discordActivityDetails;
  std::int64_t m_discordActivityStartTime = 0;
  std::optional<std::pair<std::uint16_t, std::uint16_t>> m_discordPartySize;
  bool m_discordForceUpdateActivity = false;
  bool m_discordUpdatingActivity = false;
  // Hosted lobbies retain their secret; a remote membership has an empty secret.
  std::optional<std::pair<discord::LobbyId, String>> m_discordServerLobby = {};

  int m_discordOnActivityJoinToken = -1;
  int m_discordOnActivityRequestToken = -1;
  int m_discordOnReceiveMessage = -1;
  int m_discordOnLobbyMemberConnect = -1;
  int m_discordOnLobbyMemberUpdate = -1;
  int m_discordOnLobbyMemberDisconnect = -1;

#endif
};

}

export module star.p2p_networking_service_pc;

export namespace Star {
  using ::Star::PcP2PNetworkingService;
}
