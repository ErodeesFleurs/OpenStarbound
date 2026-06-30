#pragma once

#include "StarIODevice.hpp"
#include "StarSocket.hpp"
#include "StarThread.hpp"

namespace Star {

class TcpSocket;
using TcpSocketPtr = SharedPtr<TcpSocket>;
class TcpServer;
using TcpServerPtr = SharedPtr<TcpServer>;

class TcpSocket : public Socket {
  struct ConstructorToken {};

public:
  [[nodiscard]] static TcpSocketPtr connectTo(HostAddressWithPort const& address);
  [[nodiscard]] static TcpSocketPtr listen(HostAddressWithPort const& address);

  TcpSocket(ConstructorToken, NetworkMode networkMode);
  TcpSocket(ConstructorToken, NetworkMode networkMode, SocketImplPtr impl);

  [[nodiscard]] TcpSocketPtr accept();

  // Must be called after connect.  Sets TCP_NODELAY option.
  void setNoDelay(bool noDelay);

  [[nodiscard]] size_t receive(char* data, size_t len);
  [[nodiscard]] size_t send(char const* data, size_t len);

  [[nodiscard]] HostAddressWithPort localAddress() const;
  [[nodiscard]] HostAddressWithPort remoteAddress() const;

private:
  void connect(HostAddressWithPort const& address);

  HostAddressWithPort m_remoteAddress;
};

// Simple class to listen for and open TcpSocket instances.
class TcpServer {
public:
  using AcceptCallback = function<void(TcpSocketPtr socket)>;

  TcpServer(HostAddressWithPort const& address);
  // Listens to all interfaces.
  TcpServer(uint16_t port);
  ~TcpServer();

  void stop();
  [[nodiscard]] bool isListening() const;

  // Blocks until next connection available for the given timeout.  Throws
  // ServerClosed if close() is called.  Cannot be called if AcceptCallback is
  // set.
  [[nodiscard]] TcpSocketPtr accept(unsigned timeout);

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
  TcpSocketPtr m_listenSocket;
};

}// namespace Star
