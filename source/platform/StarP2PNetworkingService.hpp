#pragma once

#include "StarEither.hpp"
#include "StarHostAddress.hpp"
#include "StarRpcPromise.hpp"
#include "StarString.hpp"
#include "StarStrongTypedef.hpp"

import std;

namespace Star {

enum class P2PJoinRequestReply {
  No,
  Yes,
  Ignore,
};

// P2P networking is assumed to be guaranteed in order delivery of arbitrarily
// sized messages.  Neither the P2PSocket or the P2PNetworkingService are
// assumed to be thread safe interfaces, but access to independent P2PSockets
// from different threads or access to a P2PSocket and the P2PNetworkingService
// from different threads is assumed to be safe.
class P2PSocket {
public:
  virtual ~P2PSocket() = default;

  virtual auto isOpen() -> bool = 0;
  virtual auto sendMessage(ByteArray const& message) -> bool = 0;
  virtual auto receiveMessage() -> std::optional<ByteArray> = 0;
};

using P2PNetworkingPeerId = StrongTypedef<String>;

// API for platform specific peer to peer multiplayer services.
class P2PNetworkingService {
public:
  virtual ~P2PNetworkingService() = default;

  // P2P friends cannot join this player
  virtual void setJoinUnavailable() = 0;
  // P2P friends can join this player's local game
  virtual void setJoinLocal(std::uint32_t capacity) = 0;
  // P2P friends can join this player at the given remote server
  virtual void setJoinRemote(HostAddressWithPort location) = 0;
  // Updates rich presence activity info
  virtual void setActivityData(const char* title, const char* details, std::int64_t startTime, std::optional<std::pair<std::uint16_t, std::uint16_t>>) = 0;

  // If this player joins another peer's game using the P2P UI, this will return
  // a pending join location
  virtual auto pullPendingJoin() -> MVariant<P2PNetworkingPeerId, HostAddressWithPort> = 0;
  // This will return a username and a promise keeper to respond to the join request
  virtual auto pullJoinRequest() -> std::optional<std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> = 0;

  virtual void setAcceptingP2PConnections(bool acceptingP2PConnections) = 0;
  virtual auto acceptP2PConnections() -> List<UPtr<P2PSocket>> = 0;
  virtual void update() = 0;

  virtual auto connectToPeer(P2PNetworkingPeerId peerId) -> Either<String, UPtr<P2PSocket>> = 0;
};

};// namespace Star
