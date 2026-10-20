module;
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
import star.string;
import star.either;
import star.host_address;
import star.thread;
import star.socket;
import star.logging;
import star.net_impl;

export module star.udp;

export namespace Star {

// A Good default assumption for a maximum size of a UDP datagram without
// fragmentation
inline constexpr unsigned MaxUdpData = 1460;

class UdpSocket : public Socket {
public:
  UdpSocket(NetworkMode networkMode);

  std::size_t receive(HostAddressWithPort* address, char* data, std::size_t size);
  std::size_t send(HostAddressWithPort const& address, char const* data, std::size_t size);
};

class UdpServer {
public:
  UdpServer(Logger& logger, HostAddressWithPort const& address);
  ~UdpServer();

  void close();
  bool isListening() const;

  std::size_t receive(HostAddressWithPort* address, char* data, std::size_t size, unsigned timeout);
  std::size_t send(HostAddressWithPort const& address, char const* data, std::size_t size);

private:
  HostAddressWithPort const m_hostAddress;
  std::shared_ptr<UdpSocket> m_listenSocket;
};

}

namespace Star {

UdpSocket::UdpSocket(NetworkMode networkMode) : Socket(SocketType::Udp, networkMode) {}

std::size_t UdpSocket::receive(HostAddressWithPort* address, char* data, std::size_t datasize) {
  ReadLocker locker(m_mutex);
  checkOpen("UdpSocket::receive");

  int flags = 0;
  int len;
  struct sockaddr_storage sockAddr;
  socklen_t sockAddrLen = sizeof(sockAddr);

  len = ::recvfrom(m_impl->socketDesc, data, datasize, flags, (struct sockaddr*)&sockAddr, &sockAddrLen);

  if (len < 0) {
    if (!isActive())
      throw SocketClosedException("Connection closed");
    else if (netErrorInterrupt())
      len = 0;
    else
      throw NetworkException(strf("udp recv error: {}", netErrorString()));
  }

  if (address)
    setAddressFromNative(*address, m_localAddress.address().mode(), &sockAddr);

  return len;
}

std::size_t UdpSocket::send(HostAddressWithPort const& address, char const* data, std::size_t size) {
  ReadLocker locker(m_mutex);
  checkOpen("UdpSocket::send");

  struct sockaddr_storage sockAddr;
  socklen_t sockAddrLen;
  setNativeFromAddress(address, &sockAddr, &sockAddrLen);

  int len = ::sendto(m_impl->socketDesc, data, size, 0, (struct sockaddr*)&sockAddr, sockAddrLen);
  if (len < 0) {
    if (!isActive())
      throw SocketClosedException("Connection closed");
    else if (netErrorInterrupt())
      len = 0;
    else
      throw NetworkException(strf("udp send error: {}", netErrorString()));
  }

  return len;
}

UdpServer::UdpServer(Logger& logger, HostAddressWithPort const& address)
  : m_hostAddress(address), m_listenSocket(std::make_shared<UdpSocket>(m_hostAddress.address().mode())) {
  m_listenSocket->setNonBlocking(true);
  m_listenSocket->bind(logger, m_hostAddress);
  logger.debug("UdpServer listening on: {}", m_hostAddress);
}

UdpServer::~UdpServer() {
  close();
}

std::size_t UdpServer::receive(HostAddressWithPort* address, char* data, std::size_t bufsize, unsigned timeout) {
  Socket::poll({{m_listenSocket, {true, false}}}, timeout);
  return m_listenSocket->receive(address, data, bufsize);
}

std::size_t UdpServer::send(HostAddressWithPort const& address, char const* data, std::size_t len) {
  return m_listenSocket->send(address, data, len);
}

void UdpServer::close() {
  m_listenSocket->close();
}

bool UdpServer::isListening() const {
  return m_listenSocket->isActive();
}

}
