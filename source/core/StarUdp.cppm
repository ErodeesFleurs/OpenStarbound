module;

#include "StarSocket.hpp"
#include "StarLogging.hpp"
#include "StarNetImpl.hpp"

export module star.udp;

export namespace Star {

STAR_CLASS(UdpSocket);
STAR_CLASS(UdpServer);

// A Good default assumption for a maximum size of a UDP datagram without
// fragmentation
inline constexpr unsigned MaxUdpData = 1460;

class UdpSocket : public Socket {
public:
  UdpSocket(NetworkMode networkMode);

  size_t receive(HostAddressWithPort* address, char* data, size_t size);
  size_t send(HostAddressWithPort const& address, char const* data, size_t size);
};

class UdpServer {
public:
  UdpServer(HostAddressWithPort const& address);
  ~UdpServer();

  void close();
  bool isListening() const;

  size_t receive(HostAddressWithPort* address, char* data, size_t size, unsigned timeout);
  size_t send(HostAddressWithPort const& address, char const* data, size_t size);

private:
  HostAddressWithPort const m_hostAddress;
  UdpSocketPtr m_listenSocket;
};

}

namespace Star {

UdpSocket::UdpSocket(NetworkMode networkMode) : Socket(SocketType::Udp, networkMode) {}

size_t UdpSocket::receive(HostAddressWithPort* address, char* data, size_t datasize) {
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

size_t UdpSocket::send(HostAddressWithPort const& address, char const* data, size_t size) {
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

UdpServer::UdpServer(HostAddressWithPort const& address)
  : m_hostAddress(address), m_listenSocket(make_shared<UdpSocket>(m_hostAddress.address().mode())) {
  m_listenSocket->setNonBlocking(true);
  m_listenSocket->bind(m_hostAddress);
  Logger::debug("UdpServer listening on: {}", m_hostAddress);
}

UdpServer::~UdpServer() {
  close();
}

size_t UdpServer::receive(HostAddressWithPort* address, char* data, size_t bufsize, unsigned timeout) {
  Socket::poll({{m_listenSocket, {true, false}}}, timeout);
  return m_listenSocket->receive(address, data, bufsize);
}

size_t UdpServer::send(HostAddressWithPort const& address, char const* data, size_t len) {
  return m_listenSocket->send(address, data, len);
}

void UdpServer::close() {
  m_listenSocket->close();
}

bool UdpServer::isListening() const {
  return m_listenSocket->isActive();
}

}
