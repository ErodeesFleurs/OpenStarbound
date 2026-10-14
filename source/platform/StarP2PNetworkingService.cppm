module;

import std;
import star.variant;

import star.byte_array;
import star.either;
import star.list;
import star.maybe;
import star.string;
#include "StarStrongTypedef.hpp"
import star.rpc_promise;

import star.host_address;

namespace Star {

class P2PSocket;
using P2PSocketPtr = std::shared_ptr<P2PSocket>;
using P2PSocketConstPtr = std::shared_ptr<P2PSocket const>;
using P2PSocketWeakPtr = std::weak_ptr<P2PSocket>;
using P2PSocketConstWeakPtr = std::weak_ptr<P2PSocket const>;
using P2PSocketUPtr = std::unique_ptr<P2PSocket>;
using P2PSocketConstUPtr = std::unique_ptr<P2PSocket const>;
class P2PNetworkingService;
using P2PNetworkingServicePtr = std::shared_ptr<P2PNetworkingService>;
using P2PNetworkingServiceConstPtr = std::shared_ptr<P2PNetworkingService const>;
using P2PNetworkingServiceWeakPtr = std::weak_ptr<P2PNetworkingService>;
using P2PNetworkingServiceConstWeakPtr = std::weak_ptr<P2PNetworkingService const>;
using P2PNetworkingServiceUPtr = std::unique_ptr<P2PNetworkingService>;
using P2PNetworkingServiceConstUPtr = std::unique_ptr<P2PNetworkingService const>;
  
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

  virtual bool isOpen() = 0;
  virtual bool sendMessage(ByteArray const& message) = 0;
  virtual Maybe<ByteArray> receiveMessage() = 0;
};

strong_typedef(String, P2PNetworkingPeerId);

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
  virtual void setActivityData(const char* title, const char* details, std::int64_t startTime, Maybe<std::pair<std::uint16_t, std::uint16_t>>) = 0;

  // If this player joins another peer's game using the P2P UI, this will return
  // a pending join location
  virtual MVariant<P2PNetworkingPeerId, HostAddressWithPort> pullPendingJoin() = 0;
  // This will return a username and a promise keeper to respond to the join request
  virtual Maybe<std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> pullJoinRequest() = 0;

  virtual void setAcceptingP2PConnections(bool acceptingP2PConnections) = 0;
  virtual List<P2PSocketUPtr> acceptP2PConnections() = 0;
  virtual void update() = 0;

  virtual Either<String, P2PSocketUPtr> connectToPeer(P2PNetworkingPeerId peerId) = 0;
};

};

export module star.p2p_networking_service;

export namespace Star {
  using ::Star::P2PSocket;
  using ::Star::P2PSocketPtr;
  using ::Star::P2PSocketConstPtr;
  using ::Star::P2PSocketWeakPtr;
  using ::Star::P2PSocketConstWeakPtr;
  using ::Star::P2PSocketUPtr;
  using ::Star::P2PSocketConstUPtr;
  using ::Star::P2PNetworkingService;
  using ::Star::P2PNetworkingServicePtr;
  using ::Star::P2PNetworkingServiceConstPtr;
  using ::Star::P2PNetworkingServiceWeakPtr;
  using ::Star::P2PNetworkingServiceConstWeakPtr;
  using ::Star::P2PNetworkingServiceUPtr;
  using ::Star::P2PNetworkingServiceConstUPtr;
  using ::Star::P2PJoinRequestReply;
  using ::Star::P2PNetworkingPeerId;
  using ::Star::P2PNetworkingPeerIdWrapper;
}
