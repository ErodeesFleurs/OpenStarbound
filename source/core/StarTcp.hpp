#pragma once

#include "StarConfig.hpp"
#include "StarSocket.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

class TcpSocket : public Socket {
public:
  static auto connectTo(HostAddressWithPort const& address) -> Ptr<TcpSocket>;
  static auto listen(HostAddressWithPort const& address) -> Ptr<TcpSocket>;

  auto accept() -> Ptr<TcpSocket>;

  // Must be called after connect.  Sets TCP_NODELAY option.
  void setNoDelay(bool noDelay);

  auto receive(char* data, std::size_t len) -> std::size_t;
  auto send(char const* data, std::size_t len) -> std::size_t;

  auto localAddress() const -> HostAddressWithPort;
  auto remoteAddress() const -> HostAddressWithPort;

private:
  TcpSocket(NetworkMode networkMode);
  TcpSocket(NetworkMode networkMode, Ptr<SocketImpl> impl);

  void connect(HostAddressWithPort const& address);

  HostAddressWithPort m_remoteAddress;
};

// Simple class to listen for and open TcpSocket instances.
class TcpServer {
public:
  using AcceptCallback = std::function<void(Ptr<TcpSocket> socket)>;

  TcpServer(HostAddressWithPort const& address);
  // Listens to all interfaces.
  TcpServer(std::uint16_t port);
  ~TcpServer();

  void stop();
  auto isListening() const -> bool;

  // Blocks until next connection available for the given timeout.  Throws
  // ServerClosed if close() is called.  Cannot be called if AcceptCallback is
  // set.
  auto accept(unsigned timeout) -> Ptr<TcpSocket>;

  // Rather than calling and blocking on accept(), if an AcceptCallback is set
  // here, it will be called whenever a new connection is available.
  // Exceptions thrown from the callback function will be caught and logged,
  // and will cause the server to close.  The timeout here is the timeout that
  // is passed to accept in the loop, the longer the timeout the slower it will
  // shutdown on a call to close.
  void setAcceptCallback(AcceptCallback callback, unsigned timeout = 20);

private:
  mutable Mutex m_mutex;

  AcceptCallback m_callback;
  ThreadFunction<void> m_callbackThread;
  HostAddressWithPort m_hostAddress;
  Ptr<TcpSocket> m_listenSocket;
};

}// namespace Star
