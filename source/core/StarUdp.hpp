#pragma once

#include "StarConfig.hpp"
#include "StarSocket.hpp"
#include <cstdint>

import std;

namespace Star {

// A Good default assumption for a maximum size of a UDP datagram without
// fragmentation
constexpr std::uint32_t MaxUdpData = 1460;

class UdpSocket : public Socket {
public:
  UdpSocket(NetworkMode networkMode);

  auto receive(HostAddressWithPort* address, char* data, std::size_t size) -> std::size_t;
  auto send(HostAddressWithPort const& address, char const* data, std::size_t size) -> std::size_t;
};

class UdpServer {
public:
  UdpServer(HostAddressWithPort const& address);
  ~UdpServer();

  void close();
  [[nodiscard]] auto isListening() const -> bool;

  auto receive(HostAddressWithPort* address, char* data, std::size_t size, unsigned timeout) -> std::size_t;
  auto send(HostAddressWithPort const& address, char const* data, std::size_t size) -> std::size_t;

private:
  HostAddressWithPort const m_hostAddress;
  Ptr<UdpSocket> m_listenSocket;
};

}// namespace Star
