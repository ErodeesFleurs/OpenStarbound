#pragma once

#include "StarSocket.hpp"

namespace Star {

class UdpSocket;
using UdpSocketPtr = SharedPtr<UdpSocket>;

// A Good default assumption for a maximum size of a UDP datagram without
// fragmentation
unsigned const MaxUdpData = 1460;

class UdpSocket : public Socket {
public:
  UdpSocket(NetworkMode networkMode);

  [[nodiscard]] size_t receive(HostAddressWithPort* address, char* data, size_t size);
  [[nodiscard]] size_t send(HostAddressWithPort const& address, char const* data, size_t size);
};

class UdpServer {
public:
  UdpServer(HostAddressWithPort const& address);
  ~UdpServer();

  void close();
  [[nodiscard]] bool isListening() const;

  [[nodiscard]] size_t receive(HostAddressWithPort* address, char* data, size_t size, unsigned timeout);
  [[nodiscard]] size_t send(HostAddressWithPort const& address, char const* data, size_t size);

private:
  HostAddressWithPort const m_hostAddress;
  UdpSocketPtr m_listenSocket;
};

}
