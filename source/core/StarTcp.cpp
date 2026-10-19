#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#else
#ifdef STAR_SYSTEM_FREEBSD
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#endif

import std;
import star.format;
import star.io_device;
import star.string;
import star.either;
import star.host_address;
import star.thread;
import star.socket;
import star.tcp;
import star.logging;
import star.net_impl;

namespace Star {

std::shared_ptr<TcpSocket> TcpSocket::connectTo(HostAddressWithPort const& addressWithPort) {
  auto socket = std::shared_ptr<TcpSocket>(new TcpSocket(addressWithPort.address().mode()));
  socket->connect(addressWithPort);
  return socket;
}

std::shared_ptr<TcpSocket> TcpSocket::listen(HostAddressWithPort const& addressWithPort) {
  auto socket = std::shared_ptr<TcpSocket>(new TcpSocket(addressWithPort.address().mode()));
  socket->bind(addressWithPort);
  ((Socket&)(*socket)).listen(32);
  return socket;
}

std::shared_ptr<TcpSocket> TcpSocket::accept() {
  ReadLocker locker(m_mutex);

  if (m_socketMode != SocketMode::Bound)
    throw SocketClosedException("TcpSocket not bound in TcpSocket::accept");

  struct sockaddr_storage sockAddr;
  socklen_t sockAddrLen = sizeof(sockAddr);

  auto socketDesc = ::accept(m_impl->socketDesc, (struct sockaddr*)&sockAddr, &sockAddrLen);

  if (invalidSocketDescriptor(socketDesc)) {
    if (netErrorInterrupt())
      return {};
    throw NetworkException(strf("Cannot accept connection: {}", netErrorString()));
  }

  auto socketImpl = std::make_shared<SocketImpl>();
  socketImpl->socketDesc = socketDesc;

#if defined STAR_SYSTEM_MACOS || defined STAR_SYSTEM_FREEBSD
  // Don't generate sigpipe
  int set = 1;
  socketImpl->setSockOpt(SOL_SOCKET, SO_NOSIGPIPE, (void*)&set, sizeof(int));
#endif

  std::shared_ptr<TcpSocket> sockPtr(new TcpSocket(m_localAddress.address().mode(), socketImpl));

  sockPtr->m_localAddress = m_localAddress;
  setAddressFromNative(sockPtr->m_remoteAddress, m_localAddress.address().mode(), &sockAddr);
  Logger::debug("accept from {} ({})", sockPtr->m_remoteAddress, sockPtr->m_impl->socketDesc);

  return sockPtr;
}

void TcpSocket::setNoDelay(bool noDelay) {
  ReadLocker locker(m_mutex);
  checkOpen("TcpSocket::setNoDelay");

  int flag = noDelay ? 1 : 0;
  m_impl->setSockOpt(IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}

std::size_t TcpSocket::receive(char* data, std::size_t size) {
  ReadLocker locker(m_mutex);
  checkOpen("TcpSocket::receive");

  if (m_socketMode == SocketMode::Closed)
    throw SocketClosedException("TcpSocket not open in TcpSocket::receive");

  int flags = 0;
#ifdef STAR_SYSTEM_LINUX
  // Don't generate sigpipe
  flags |= MSG_NOSIGNAL;
#endif

  auto r = ::recv(m_impl->socketDesc, data, size, flags);
  if (r < 0) {
    if (m_socketMode == SocketMode::Shutdown) {
      throw SocketClosedException("Connection closed");
    } else if (netErrorConnectionReset()) {
      doShutdown();
      throw SocketClosedException("Connection reset");
    } else if (netErrorInterrupt()) {
      r = 0;
    } else {
      throw NetworkException(strf("tcp recv error: {}", netErrorString()));
    }
  } else if (r == 0 && size != 0) {
    doShutdown();
  }

  return r;
}

std::size_t TcpSocket::send(char const* data, std::size_t size) {
  ReadLocker locker(m_mutex);
  checkOpen("TcpSocket::send");

  if (m_socketMode == SocketMode::Closed)
    throw SocketClosedException("TcpSocket not open in TcpSocket::send");

  int flags = 0;
#ifdef STAR_SYSTEM_LINUX
  // Don't generate sigpipe
  flags |= MSG_NOSIGNAL;
#endif

  auto w = ::send(m_impl->socketDesc, data, size, flags);
  if (w < 0) {
    if (m_socketMode == SocketMode::Shutdown) {
      throw SocketClosedException("Connection closed");
    } else if (netErrorConnectionReset()) {
      doShutdown();
      throw SocketClosedException("Connection reset");
    } else if (netErrorInterrupt()) {
      w = 0;
    } else {
      throw NetworkException(strf("tcp send error: {}", netErrorString()));
    }
  }

  return w;
}

HostAddressWithPort TcpSocket::localAddress() const {
  ReadLocker locker(m_mutex);
  return m_localAddress;
}

HostAddressWithPort TcpSocket::remoteAddress() const {
  ReadLocker locker(m_mutex);
  return m_remoteAddress;
}

TcpSocket::TcpSocket(NetworkMode networkMode) : Socket(SocketType::Tcp, networkMode) {}

TcpSocket::TcpSocket(NetworkMode networkMode, std::shared_ptr<SocketImpl> impl) : Socket(networkMode, impl, SocketMode::Connected) {}

void TcpSocket::connect(HostAddressWithPort const& addressWithPort) {
  WriteLocker locker(m_mutex);
  checkOpen("TcpSocket::connect");

  if (m_networkMode != addressWithPort.address().mode())
    throw NetworkException("Socket address type mismatch between address and socket.");

  struct sockaddr_storage sockAddr;
  socklen_t sockAddrLen;
  setNativeFromAddress(addressWithPort, &sockAddr, &sockAddrLen);
  if (::connect(m_impl->socketDesc, (struct sockaddr*)&sockAddr, sockAddrLen) < 0)
    throw NetworkException(strf("cannot connect to {}: {}", addressWithPort, netErrorString()));

#if defined STAR_SYSTEM_MACOS || defined STAR_SYSTEM_FREEBSD
  // Don't generate sigpipe
  int set = 1;
  m_impl->setSockOpt(SOL_SOCKET, SO_NOSIGPIPE, (void*)&set, sizeof(set));
#endif

  m_socketMode = SocketMode::Connected;
  m_remoteAddress = addressWithPort;
}

TcpServer::TcpServer(HostAddressWithPort const& address) : m_hostAddress(address) {
  m_hostAddress = address;
  m_listenSocket = TcpSocket::listen(address);
  m_listenSocket->setNonBlocking(true);
  Logger::debug("TcpServer listening on: {}", address);
}

TcpServer::TcpServer(std::uint16_t port) : TcpServer(HostAddressWithPort("*", port)) {}

TcpServer::~TcpServer() {
  stop();
}

void TcpServer::stop() {
  m_listenSocket->shutdown();
  m_callbackThread.finish();
  m_listenSocket->close();
}

bool TcpServer::isListening() const {
  return m_listenSocket->isActive();
}

std::shared_ptr<TcpSocket> TcpServer::accept(unsigned timeout) {
  MutexLocker locker(m_mutex);
  Socket::poll({{m_listenSocket, {true, false}}}, timeout);
  try {
    return m_listenSocket->accept();
  } catch (SocketClosedException const&) {
    return {};
  }
}

void TcpServer::setAcceptCallback(AcceptCallback callback, unsigned timeout) {
  MutexLocker locker(m_mutex);
  m_callback = callback;
  if (m_listenSocket->isActive() && !m_callbackThread) {
    m_callbackThread = Thread::invoke("TcpServer::acceptCallback", [this, timeout]() {
        try {
          while (true) {
            std::shared_ptr<TcpSocket> conn;
            try {
              conn = accept(timeout);
            } catch (NetworkException const& e) {
              Logger::error("TcpServer caught exception accepting connection {}", outputException(e, false));
            }

            if (conn)
              m_callback(conn);

            if (!m_listenSocket->isActive())
              break;
          }
        } catch (std::exception const& e) {
          Logger::error("TcpServer will close, listener thread caught exception:  {}", outputException(e, true));
          m_listenSocket->close();
        }
      });
  }
}

}
