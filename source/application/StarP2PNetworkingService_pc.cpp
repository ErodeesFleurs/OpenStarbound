#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#endif
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import std;
import star.maybe;
import star.format;
import star.string;
import star.list;
import star.logging;
import star.variant;
import star.thread;
import star.random;
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
import star.p2p_networking_service_pc;
import star.lexical_cast;
import star.either;
import star.logging;
import star.string;
import star.byte_array;
import star.encode;
import star.data_stream;
import star.uuid;

namespace Star {

#ifdef STAR_ENABLE_DISCORD_INTEGRATION

discord::NetworkChannelId const DiscordMainNetworkChannel = 0;

static void deleteDiscordLobby(PcPlatformServicesState* state, discord::LobbyId lobbyId) {
  state->discordCore->LobbyManager().DeleteLobby(lobbyId, [lobbyId, logger = &state->logger](discord::Result res) {
    if (res != discord::Result::Ok)
      logger->error("Could not delete Discord lobby {} (err {})", lobbyId, (int)res);
  });
}

static void disconnectDiscordLobby(PcPlatformServicesState* state, discord::LobbyId lobbyId) {
  state->discordCore->LobbyManager().DisconnectLobby(lobbyId, [lobbyId, logger = &state->logger](discord::Result res) {
    if (res != discord::Result::Ok)
      logger->error("Could not leave Discord lobby {} (err {})", lobbyId, (int)res);
  });
}

#endif

PcP2PNetworkingService::PcP2PNetworkingService(std::shared_ptr<PcPlatformServicesState> state)
  : m_state(std::move(state))
#ifdef STAR_ENABLE_STEAM_INTEGRATION
  , m_callbackConnectionFailure(m_state->steamDispatch, this, &PcP2PNetworkingService::steamOnConnectionFailure),
    m_callbackJoinRequested(m_state->steamDispatch, this, &PcP2PNetworkingService::steamOnJoinRequested),
    m_callbackSessionRequest(m_state->steamDispatch, this, &PcP2PNetworkingService::steamOnSessionRequest)
#endif
{

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  if (m_state->discordAvailable) {
    MutexLocker discordLocker(m_state->discordMutex);
    m_discordLifetime->service = this;

    m_discordPartySize = {};

    try {
    m_discordOnActivityJoinToken = m_state->discordCore->ActivityManager().OnActivityJoin.Connect([this](char const* peerId) {
        m_state->logger.info("Joining Discord peer at '{}'", peerId);
        addPendingJoin(strf("+platform:{}", peerId));
      });
    m_discordOnActivityRequestToken = m_state->discordCore->ActivityManager().OnActivityJoinRequest.Connect([this](discord::User const& user) {
        MutexLocker serviceLocker(m_mutex);
        String userName = String(user.GetUsername());
        m_state->logger.info("Received join request from user '{}'", userName);
        m_discordJoinRequests.emplace_back(std::make_pair(user.GetId(), userName));
      });
    m_discordOnReceiveMessage = m_state->discordCore->LobbyManager().OnNetworkMessage.Connect(std::bind(&PcP2PNetworkingService::discordOnReceiveMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    m_discordOnLobbyMemberConnect = m_state->discordCore->LobbyManager().OnMemberConnect.Connect(std::bind(&PcP2PNetworkingService::discordOnLobbyMemberConnect, this, std::placeholders::_1, std::placeholders::_2));
    m_discordOnLobbyMemberUpdate = m_state->discordCore->LobbyManager().OnMemberUpdate.Connect(std::bind(&PcP2PNetworkingService::discordOnLobbyMemberUpdate, this, std::placeholders::_1, std::placeholders::_2));
    m_discordOnLobbyMemberDisconnect = m_state->discordCore->LobbyManager().OnMemberDisconnect.Connect(std::bind(&PcP2PNetworkingService::discordOnLobbyMemberDisconnect, this, std::placeholders::_1, std::placeholders::_2));
    } catch (...) {
      m_discordLifetime->service = nullptr;
      discordDisconnectCallbacks();
      throw;
    }
  }
#endif
}

PcP2PNetworkingService::~PcP2PNetworkingService() {
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  if (m_state->discordAvailable) {
    MutexLocker discordLocker(m_state->discordMutex);
    m_discordLifetime->service = nullptr;
    for (auto const& entry : m_discordOpenSockets) {
      MutexLocker socketLocker(entry.second->mutex);
      entry.second->mode = DiscordSocketMode::Disconnected;
    }
    m_discordOpenSockets.clear();
    if (m_discordServerLobby) {
      auto const& lobby = *m_discordServerLobby;
      auto res = m_state->discordCore->LobbyManager().DisconnectNetwork(lobby.first);
      if (res != discord::Result::Ok)
        m_state->logger.error("Could not disconnect Discord lobby network {} (err {})", lobby.first, (int)res);
      if (lobby.second.empty())
        disconnectDiscordLobby(m_state.get(), lobby.first);
      else
        deleteDiscordLobby(m_state.get(), lobby.first);
      m_discordServerLobby = {};
    }

    discordDisconnectCallbacks();
  }
#endif
}

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
void PcP2PNetworkingService::discordDisconnectCallbacks() {
  m_state->discordCore->ActivityManager().OnActivityJoin.Disconnect(std::exchange(m_discordOnActivityJoinToken, -1));
  m_state->discordCore->ActivityManager().OnActivityJoinRequest.Disconnect(std::exchange(m_discordOnActivityRequestToken, -1));
  m_state->discordCore->LobbyManager().OnNetworkMessage.Disconnect(std::exchange(m_discordOnReceiveMessage, -1));
  m_state->discordCore->LobbyManager().OnMemberConnect.Disconnect(std::exchange(m_discordOnLobbyMemberConnect, -1));
  m_state->discordCore->LobbyManager().OnMemberUpdate.Disconnect(std::exchange(m_discordOnLobbyMemberUpdate, -1));
  m_state->discordCore->LobbyManager().OnMemberDisconnect.Disconnect(std::exchange(m_discordOnLobbyMemberDisconnect, -1));
}
#endif

void PcP2PNetworkingService::setJoinUnavailable() {
  setJoinLocation(JoinUnavailable());
}

void PcP2PNetworkingService::setJoinLocal(std::uint32_t capacity) {
  setJoinLocation(JoinLocal{capacity});
}

void PcP2PNetworkingService::setJoinRemote(HostAddressWithPort location) {
  setJoinLocation(JoinRemote(location));
}

void Star::PcP2PNetworkingService::setActivityData(
  [[maybe_unused]] const char* title,
  [[maybe_unused]] const char* details,
  [[maybe_unused]] std::int64_t startTime,
  [[maybe_unused]] Maybe<std::pair<std::uint16_t, std::uint16_t>> party) {
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  MutexLocker discordLocker(m_state->discordMutex);
#endif
  MutexLocker serviceLocker(m_mutex);

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  if (m_state->discordAvailable && m_state->discordCurrentUser) {
    if (m_discordUpdatingActivity)
      return;

    if (title != m_discordActivityTitle
     || details != m_discordActivityDetails
     || startTime != m_discordActivityStartTime || party != m_discordPartySize || m_discordForceUpdateActivity) {
      m_discordForceUpdateActivity = false;
      m_discordPartySize = party;
      m_discordActivityTitle = title;
      m_discordActivityDetails = details;
      m_discordActivityStartTime = startTime;

      discord::Activity activity = {};
      activity.SetType(discord::ActivityType::Playing);
      activity.SetName("Starbound");
      activity.SetState(title);
      activity.SetDetails(details);
      activity.GetTimestamps().SetStart(startTime);
      if (party) {
        auto& size = activity.GetParty().GetSize();
        size.SetCurrentSize(party->first);
        size.SetMaxSize(party->second);
      }
  
      if (auto lobby = m_discordServerLobby)
        activity.GetParty().SetId(toString(lobby->first).c_str());

      if (m_joinLocation.is<JoinLocal>()) {
        if (auto lobby = m_discordServerLobby) {
          String joinSecret = strf("connect:discord_{}_{}_{}", m_state->discordCurrentUser->GetId(), lobby->first, lobby->second);
          m_state->logger.info("Setting Discord join secret as {}", joinSecret);
          activity.GetSecrets().SetJoin(joinSecret.utf8Ptr());
        }
      } else if (m_joinLocation.is<JoinRemote>()) {
        String address = toString((HostAddressWithPort)m_joinLocation.get<JoinRemote>());
        String joinSecret = strf("connect:address_{}", address);
        m_state->logger.info("Setting Discord join secret as {}", joinSecret);
        activity.GetSecrets().SetJoin(joinSecret.utf8Ptr());
      
        activity.GetParty().SetId(address.utf8Ptr());
      }
    
      m_discordUpdatingActivity = true;
      m_state->discordCore->ActivityManager().UpdateActivity(activity, [lifetime = m_discordLifetime, logger = &m_state->logger](discord::Result res) {
          if (res != discord::Result::Ok)
            logger->error("Failed to set Discord activity (err {})", (int)res);
          if (auto service = lifetime->service) {
            MutexLocker serviceLocker(service->m_mutex);
            service->m_discordUpdatingActivity = false;
          }
        });
    }
  }
#endif
}

MVariant<P2PNetworkingPeerId, HostAddressWithPort> PcP2PNetworkingService::pullPendingJoin() {
  MutexLocker serviceLocker(m_mutex);
  return take(m_pendingJoin);
}

Maybe<std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> Star::PcP2PNetworkingService::pullJoinRequest() {
  MutexLocker serviceLocker(m_mutex);

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  if (auto request = m_discordJoinRequests.maybeTakeLast()) {
    auto promisePair = RpcPromise<P2PJoinRequestReply>::createPair();
    m_pendingDiscordJoinRequests.push_back(std::make_pair(request->first, promisePair.first));
    return std::make_pair(request->second, promisePair.second);
  }
#endif

  return {};
}

void PcP2PNetworkingService::setAcceptingP2PConnections(bool acceptingP2PConnections) {
  List<std::unique_ptr<P2PSocket>> rejectedConnections;
  {
    MutexLocker serviceLocker(m_mutex);
    m_acceptingP2PConnections = acceptingP2PConnections;
    if (!m_acceptingP2PConnections)
      rejectedConnections = take(m_pendingIncomingConnections);
  }
  // Socket destructors acquire SDK/service/socket locks in that order.
}

List<std::unique_ptr<P2PSocket>> PcP2PNetworkingService::acceptP2PConnections() {
  MutexLocker serviceLocker(m_mutex);
  return take(m_pendingIncomingConnections);
}

void Star::PcP2PNetworkingService::update() {
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  MutexLocker discordLocker(m_state->discordMutex);
#endif
  MutexLocker serviceLocker(m_mutex);
  
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  for (auto& p : m_pendingDiscordJoinRequests) {
    if (auto res = p.second.result()) {
      auto reply = discord::ActivityJoinRequestReply::Ignore;
      switch (*res) {
        case P2PJoinRequestReply::Yes:
          reply = discord::ActivityJoinRequestReply::Yes;
          break;
        case P2PJoinRequestReply::No:
          reply = discord::ActivityJoinRequestReply::No;
          break;
        case P2PJoinRequestReply::Ignore:
          reply = discord::ActivityJoinRequestReply::Ignore;
          break;
      }

      m_state->discordCore->ActivityManager().SendRequestReply(p.first, reply, [logger = &m_state->logger](discord::Result res) {
          if (res != discord::Result::Ok)
            logger->error("Could not send Discord activity join response (err {})", (int)res);
        });
    }
  }
  m_pendingDiscordJoinRequests = m_pendingDiscordJoinRequests.filtered([](std::pair<discord::UserId, RpcPromise<P2PJoinRequestReply>>& p) {
      return !p.second.finished();
    });
#endif
}

Either<String, std::unique_ptr<P2PSocket>> PcP2PNetworkingService::connectToPeer(P2PNetworkingPeerId peerId) {
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  MutexLocker discordLocker(m_state->discordMutex);
#endif
  MutexLocker serviceLocker(m_mutex);
  String type = peerId.extract("_");

#ifdef STAR_ENABLE_STEAM_INTEGRATION
  if (m_state->steamAvailable) {
    if (type == "steamid") {
      CSteamID steamId(lexicalCast<uint64>(peerId));
      return makeRight(createSteamP2PSocket(steamId));
    }
  }
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  if (m_state->discordAvailable) {
    if (type == "discord") {
      auto remoteUserId = lexicalCast<discord::UserId>(peerId.extract("_"));
      auto lobbyId = lexicalCast<discord::LobbyId>(peerId.extract("_"));
      String lobbySecret = std::move(peerId);
      return makeRight(discordConnectRemote(remoteUserId, lobbyId, lobbySecret));
    }
  }
#endif

  return makeLeft(strf("Unsupported peer type '{}'", type));
}

void PcP2PNetworkingService::addPendingJoin(String connectionString) {
  MutexLocker serviceLocker(m_mutex);

  if (connectionString.extract(":") != "+platform")
    throw ApplicationException::format("malformed connection string '{}'", connectionString);

  if (connectionString.extract(":") != "connect")
    throw ApplicationException::format("malformed connection string '{}'", connectionString);

  String target = std::move(connectionString);
  String targetType = target.extract("_");

  if (targetType == "address")
    m_pendingJoin = HostAddressWithPort(target);
  else
    m_pendingJoin = P2PNetworkingPeerId(strf("{}_{}", targetType, target));
}

#ifdef STAR_ENABLE_STEAM_INTEGRATION

PcP2PNetworkingService::SteamP2PSocket::~SteamP2PSocket() {
  MutexLocker serviceLocker(parent->m_mutex);
  MutexLocker socketLocker(mutex);
  parent->steamCloseSocket(this);
}

bool PcP2PNetworkingService::SteamP2PSocket::isOpen() {
  MutexLocker socketLocker(mutex);
  return connected;
}

bool PcP2PNetworkingService::SteamP2PSocket::sendMessage(ByteArray const& message) {
  MutexLocker socketLocker(mutex);
  if (!connected)
    return false;

  if (!SteamAPI_ISteamNetworking_SendP2PPacket(SteamAPI_SteamNetworking(), steamId.ConvertToUint64(),
      message.ptr(), message.size(), k_EP2PSendReliable, 0))
    throw ApplicationException("SteamNetworking::SendP2PPacket unexpectedly returned false");
  return true;
}

Maybe<ByteArray> PcP2PNetworkingService::SteamP2PSocket::receiveMessage() {
  MutexLocker socketLocker(mutex);
  if (!incoming.empty())
    return incoming.takeFirst();

  if (connected) {
    socketLocker.unlock();
    {
      MutexLocker serviceLocker(parent->m_mutex);
      parent->steamReceiveAll();
    }

    socketLocker.lock();
    if (!incoming.empty())
      return incoming.takeFirst();
  }

  return {};
}

auto PcP2PNetworkingService::createSteamP2PSocket(CSteamID steamId) -> std::unique_ptr<SteamP2PSocket> {
  if (auto oldSocket = m_steamOpenSockets.value(steamId.ConvertToUint64())) {
    MutexLocker socketLocker(oldSocket->mutex);
    steamCloseSocket(oldSocket);
  }

  std::unique_ptr<SteamP2PSocket> socket(new SteamP2PSocket);
  socket->parent = this;
  socket->steamId = steamId;
  socket->connected = true;

  m_steamOpenSockets[steamId.ConvertToUint64()] = socket.get();

  return socket;
}

void PcP2PNetworkingService::steamOnConnectionFailure(P2PSessionConnectFail_t* callback) {
  MutexLocker serviceLocker(m_mutex);
  m_state->logger.warn("Connection with Steam user {} failed (error code: {})", callback->m_steamIDRemote.ConvertToUint64(), callback->m_eP2PSessionError);
  if (auto socket = m_steamOpenSockets.value(callback->m_steamIDRemote.ConvertToUint64())) {
    MutexLocker socketLocker(socket->mutex);
    steamCloseSocket(socket);
  }
}

void PcP2PNetworkingService::steamOnJoinRequested(GameRichPresenceJoinRequested_t* callback) {
  m_state->logger.info("Enqueueing join request with Steam friend id {} to address {}", callback->m_steamIDFriend.ConvertToUint64(), callback->m_rgchConnect);
  addPendingJoin(callback->m_rgchConnect);
}

void PcP2PNetworkingService::steamOnSessionRequest(P2PSessionRequest_t* callback) {
  MutexLocker serviceLocker(m_mutex);
  // Not sure whether this HasFriend call is actually necessary, or whether
  // non-friends can even initiate P2P sessions.
  if (m_acceptingP2PConnections && SteamAPI_ISteamFriends_HasFriend(SteamAPI_SteamFriends(),
      callback->m_steamIDRemote.ConvertToUint64(), k_EFriendFlagImmediate)) {
    if (SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser(SteamAPI_SteamNetworking(),
        callback->m_steamIDRemote.ConvertToUint64())) {
      m_state->logger.info("Accepted Steam P2P connection with user {}", callback->m_steamIDRemote.ConvertToUint64());
      m_pendingIncomingConnections.append(createSteamP2PSocket(callback->m_steamIDRemote));
    } else {
      m_state->logger.error("Accepting Steam P2P connection from user {} failed!", callback->m_steamIDRemote.ConvertToUint64());
    }
  } else {
    m_state->logger.error("Ignoring Steam P2P connection from user {}", callback->m_steamIDRemote.ConvertToUint64());
  }
}

void PcP2PNetworkingService::steamCloseSocket(SteamP2PSocket* socket) {
  if (socket->connected) {
    m_state->logger.info("Closing P2P connection with Steam user {}", socket->steamId.ConvertToUint64());
    m_steamOpenSockets.remove(socket->steamId.ConvertToUint64());
    socket->connected = false;
  }
  SteamAPI_ISteamNetworking_CloseP2PSessionWithUser(SteamAPI_SteamNetworking(), socket->steamId.ConvertToUint64());
}

void PcP2PNetworkingService::steamReceiveAll() {
  std::uint32_t messageSize;
  CSteamID messageRemoteUser;
  while (SteamAPI_ISteamNetworking_IsP2PPacketAvailable(SteamAPI_SteamNetworking(), &messageSize, 0)) {
    ByteArray data(messageSize, 0);
    SteamAPI_ISteamNetworking_ReadP2PPacket(SteamAPI_SteamNetworking(), data.ptr(), messageSize,
        &messageSize, &messageRemoteUser, 0);
    if (auto openSocket = m_steamOpenSockets.value(messageRemoteUser.ConvertToUint64())) {
      MutexLocker socketLocker(openSocket->mutex);
      openSocket->incoming.append(std::move(data));
    }
  }
}

#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION

PcP2PNetworkingService::DiscordP2PSocket::~DiscordP2PSocket() {
  MutexLocker discordLocker(state->discordMutex);
  if (auto service = lifetime->service) {
    MutexLocker serviceLocker(service->m_mutex);
    MutexLocker socketLocker(mutex);
    service->discordCloseSocket(this);
  }
}

bool PcP2PNetworkingService::DiscordP2PSocket::isOpen() {
  MutexLocker socketLocker(mutex);
  return mode != DiscordSocketMode::Disconnected;
}

bool PcP2PNetworkingService::DiscordP2PSocket::sendMessage(ByteArray const& message) {
  MutexLocker discordLocker(state->discordMutex);
  MutexLocker socketLocker(mutex);
  if (!lifetime->service || mode != DiscordSocketMode::Connected)
    return false;

  discord::Result res = state->discordCore->LobbyManager().SendNetworkMessage(lobbyId, remoteUserId, DiscordMainNetworkChannel, (std::uint8_t*)message.ptr(), message.size());
  if (res != discord::Result::Ok)
    throw ApplicationException::format("discord::Network::Send returned error (err {})", (int)res);

  return true;
}

Maybe<ByteArray> PcP2PNetworkingService::DiscordP2PSocket::receiveMessage() {
  MutexLocker socketLocker(mutex);
  if (!incoming.empty())
    return incoming.takeFirst();
  else
    return {};
}

void PcP2PNetworkingService::discordCloseSocket(DiscordP2PSocket* socket) {
  if (socket->mode != DiscordSocketMode::Disconnected) {
    m_discordOpenSockets.remove(socket->remoteUserId);

    if (socket->mode == DiscordSocketMode::Connected) {
      if (!m_joinLocation.is<JoinLocal>() && m_discordOpenSockets.empty()) {
        auto res = m_state->discordCore->LobbyManager().DisconnectNetwork(socket->lobbyId);
        if (res != discord::Result::Ok)
          m_state->logger.error("Failed to leave network for lobby {} (err {})", socket->lobbyId, (int)res);

        disconnectDiscordLobby(m_state.get(), socket->lobbyId);
        if (m_discordServerLobby && m_discordServerLobby->first == socket->lobbyId) {
          m_discordServerLobby = {};
          m_discordForceUpdateActivity = true;
        }
      }
    }

    socket->mode = DiscordSocketMode::Disconnected;
  }
}

std::unique_ptr<P2PSocket> PcP2PNetworkingService::discordConnectRemote(discord::UserId remoteUserId, discord::LobbyId lobbyId, String const& lobbySecret) {
  if (auto oldSocket = m_discordOpenSockets.value(remoteUserId)) {
    MutexLocker socketLocker(oldSocket->mutex);
    discordCloseSocket(oldSocket);
  }

  std::unique_ptr<DiscordP2PSocket> socket(new DiscordP2PSocket);
  socket->state = m_state;
  socket->lifetime = m_discordLifetime;
  socket->connectionGeneration = ++m_discordConnectionGeneration;
  socket->mode = DiscordSocketMode::Startup;
  socket->remoteUserId = remoteUserId;
  socket->lobbyId = lobbyId;
  m_discordOpenSockets[remoteUserId] = socket.get();

  m_state->logger.info("Connecting to Discord lobby {}", lobbyId);
  m_state->discordCore->LobbyManager().ConnectLobby(lobbyId, lobbySecret.utf8Ptr(),
    [lifetime = m_discordLifetime, state = m_state.get(), remoteUserId, lobbyId,
     generation = socket->connectionGeneration](discord::Result res, discord::Lobby const&) {
      if (res != discord::Result::Ok)
        state->logger.error("Failed to connect to remote lobby {} (err {})", lobbyId, (int)res);
      auto service = lifetime->service;
      if (!service) {
        if (res == discord::Result::Ok)
          disconnectDiscordLobby(state, lobbyId);
        return;
      }
      MutexLocker serviceLocker(service->m_mutex);
      auto socket = service->m_discordOpenSockets.value(remoteUserId);
      if (!socket || socket->connectionGeneration != generation) {
        if (res == discord::Result::Ok) {
          // A replacement connection to this lobby owns the shared membership.
          bool inUse = service->m_discordServerLobby && service->m_discordServerLobby->first == lobbyId;
          for (auto const& entry : service->m_discordOpenSockets)
            inUse = inUse || entry.second->lobbyId == lobbyId;
          if (!inUse)
            disconnectDiscordLobby(state, lobbyId);
        }
        return;
      }
      MutexLocker socketLocker(socket->mutex);
      if (res != discord::Result::Ok) {
        service->discordCloseSocket(socket);
        return;
      }
      res = state->discordCore->LobbyManager().ConnectNetwork(lobbyId);
      if (res != discord::Result::Ok) {
        state->logger.error("Could not connect to Discord lobby network {} (err {})", lobbyId, (int)res);
        service->discordCloseSocket(socket);
        disconnectDiscordLobby(state, lobbyId);
        return;
      }
      res = state->discordCore->LobbyManager().OpenNetworkChannel(lobbyId, DiscordMainNetworkChannel, true);
      if (res != discord::Result::Ok) {
        state->logger.error("Could not open Discord main network channel (err {})", (int)res);
        auto disconnectResult = state->discordCore->LobbyManager().DisconnectNetwork(lobbyId);
        if (disconnectResult != discord::Result::Ok)
          state->logger.error("Could not disconnect Discord lobby network {} (err {})", lobbyId, (int)disconnectResult);
        service->discordCloseSocket(socket);
        disconnectDiscordLobby(state, lobbyId);
        return;
      }
      socket->mode = DiscordSocketMode::Connected;
      state->logger.info("Discord P2P connection opened to remote user {} via lobby {}", remoteUserId, lobbyId);
      service->m_discordServerLobby = std::make_pair(lobbyId, String());
      service->m_discordForceUpdateActivity = true;
    });

  return std::unique_ptr<P2PSocket>(std::move(socket));
}

void PcP2PNetworkingService::discordOnReceiveMessage(discord::LobbyId lobbyId, discord::UserId userId, discord::NetworkChannelId channel, std::uint8_t* data, std::uint32_t size) {
  MutexLocker serviceLocker(m_mutex);

  if (!m_discordServerLobby || lobbyId != m_discordServerLobby->first)
    return m_state->logger.error("Received message from unexpected lobby {}", lobbyId);

  if (auto socket = m_discordOpenSockets.value(userId)) {
      if (channel == DiscordMainNetworkChannel) {
        MutexLocker socketLocker(socket->mutex);
        socket->incoming.append(ByteArray((char const*)data, size));
      } else {
        m_state->logger.error("Received Discord message on unexpected channel {}, ignoring", channel);
      }
    } else {
    m_state->logger.error("Could not find associated Discord socket for user id {}", userId);
    }
  }

void PcP2PNetworkingService::discordOnLobbyMemberConnect(discord::LobbyId lobbyId, discord::UserId userId) {
  MutexLocker serviceLocker(m_mutex);

  if (m_discordServerLobby && m_discordServerLobby->first == lobbyId && userId != m_state->discordCurrentUser->GetId()) {
    if (!m_discordOpenSockets.contains(userId)) {
      std::unique_ptr<DiscordP2PSocket> socket(new DiscordP2PSocket);
      socket->state = m_state;
      socket->lifetime = m_discordLifetime;
      socket->lobbyId = lobbyId;
      socket->remoteUserId = userId;
      socket->mode = DiscordSocketMode::Connected;

      m_discordOpenSockets[userId] = socket.get();
      m_pendingIncomingConnections.append(std::move(socket));
      m_state->logger.info("Accepted new Discord connection from remote user {}", userId);
    }
  }
}

void PcP2PNetworkingService::discordOnLobbyMemberUpdate(discord::LobbyId lobbyId, discord::UserId userId) {
  discordOnLobbyMemberConnect(lobbyId, userId);
}

void PcP2PNetworkingService::discordOnLobbyMemberDisconnect(discord::LobbyId lobbyId, discord::UserId userId) {
  MutexLocker serviceLocker(m_mutex);

  if (m_discordServerLobby && m_discordServerLobby->first == lobbyId && userId != m_state->discordCurrentUser->GetId()) {
    if (auto socket = m_discordOpenSockets.value(userId)) {
      MutexLocker socketLocker(socket->mutex);
      discordCloseSocket(socket);
    }
  }
}

#endif

void PcP2PNetworkingService::setJoinLocation(JoinLocation location) {
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  MutexLocker discordLocker(m_state->discordMutex);
#endif
  MutexLocker serviceLocker(m_mutex);

  if (location == m_joinLocation)
    return;
  m_joinLocation = location;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
  if (m_state->steamAvailable) {
    if (m_joinLocation.is<JoinUnavailable>()) {
      m_state->logger.info("Clearing Steam rich presence connection");
      SteamAPI_ISteamFriends_SetRichPresence(SteamAPI_SteamFriends(), "connect", "");

    } else if (m_joinLocation.is<JoinLocal>()) {
      auto steamId = SteamAPI_ISteamUser_GetSteamID(SteamAPI_SteamUser());
      m_state->logger.info("Setting Steam rich presence connection as steamid_{}", steamId);
      SteamAPI_ISteamFriends_SetRichPresence(SteamAPI_SteamFriends(), "connect",
          strf("+platform:connect:steamid_{}", steamId).c_str());

    } else if (m_joinLocation.is<JoinRemote>()) {
      auto address = (HostAddressWithPort)location.get<JoinRemote>();
      m_state->logger.info("Setting Steam rich presence connection as address_{}", address);
      SteamAPI_ISteamFriends_SetRichPresence(SteamAPI_SteamFriends(), "connect",
          strf("+platform:connect:address_{}", address).c_str());
    }
  }
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  ++m_discordLobbyGeneration;
  if (m_state->discordAvailable && m_state->discordCurrentUser) {
    if (m_discordServerLobby) {
      auto lobby = take(m_discordServerLobby);
      auto res = m_state->discordCore->LobbyManager().DisconnectNetwork(lobby->first);
      if (res != discord::Result::Ok)
        m_state->logger.error("Could not disconnect Discord lobby network {} (err {})", lobby->first, (int)res);
      if (lobby->second.empty())
        disconnectDiscordLobby(m_state.get(), lobby->first);
      else
        deleteDiscordLobby(m_state.get(), lobby->first);
      m_discordForceUpdateActivity = true;
    }

    if (auto joinLocal = m_joinLocation.maybe<JoinLocal>()) {
      discord::LobbyTransaction createLobby{};
      if (m_state->discordCore->LobbyManager().GetLobbyCreateTransaction(&createLobby) != discord::Result::Ok)
        throw ApplicationException::format("discord::Lobbies::CreateLobbyTransaction failed");

      createLobby.SetCapacity(joinLocal->capacity);
      createLobby.SetType(discord::LobbyType::Private);
      m_state->discordCore->LobbyManager().CreateLobby(createLobby,
        [lifetime = m_discordLifetime, state = m_state.get(), generation = m_discordLobbyGeneration]
        (discord::Result res, discord::Lobby const& lobby) {
          if (res != discord::Result::Ok) {
            state->logger.error("Failed to create Discord lobby (err {})", (int)res);
            return;
          }
          auto lobbyId = lobby.GetId();
          auto service = lifetime->service;
          if (!service) {
            deleteDiscordLobby(state, lobbyId);
            return;
          }
          MutexLocker serviceLocker(service->m_mutex);
          if (generation != service->m_discordLobbyGeneration) {
            deleteDiscordLobby(state, lobbyId);
            return;
          }
          res = state->discordCore->LobbyManager().ConnectNetwork(lobbyId);
          if (res == discord::Result::Ok) {
            res = state->discordCore->LobbyManager().OpenNetworkChannel(lobbyId, DiscordMainNetworkChannel, true);
            if (res == discord::Result::Ok) {
              service->m_discordServerLobby = std::make_pair(lobbyId, String(lobby.GetSecret()));
              service->m_discordForceUpdateActivity = true;
              return;
            }
            state->logger.error("Failed to open Discord main network channel (err {})", (int)res);
            auto disconnectResult = state->discordCore->LobbyManager().DisconnectNetwork(lobbyId);
            if (disconnectResult != discord::Result::Ok)
              state->logger.error("Could not disconnect Discord lobby network {} (err {})", lobbyId, (int)disconnectResult);
          } else {
            state->logger.error("Failed to join Discord lobby network (err {})", (int)res);
          }
          deleteDiscordLobby(state, lobbyId);
        });
    }
  }
#endif
}

}
