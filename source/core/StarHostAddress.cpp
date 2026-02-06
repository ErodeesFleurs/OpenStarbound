#include "StarHostAddress.hpp"
#include "StarLexicalCast.hpp"
#include "StarNetImpl.hpp"

import std;

namespace Star {

auto HostAddress::localhost(NetworkMode mode) -> HostAddress {
  if (mode == NetworkMode::IPv4) {
    std::array<std::uint8_t, 4> addr{127, 0, 0, 1};
    return {mode, addr.data()};
  } else if (mode == NetworkMode::IPv6) {
    std::array<std::uint8_t, 16> addr{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    return {mode, addr.data()};
  }

  return {};
}

auto HostAddress::lookup(String const& address) -> Either<String, HostAddress> {
  try {
    HostAddress ha;
    ha.set(address);
    return makeRight(std::move(ha));
  } catch (NetworkException const& e) {
    return makeLeft(String(e.what()));
  }
}

HostAddress::HostAddress(NetworkMode mode, uint8_t* address) {
  set(mode, address);
}

HostAddress::HostAddress(String const& address) {
  auto a = lookup(address);
  if (a.isLeft())
    throw NetworkException(a.left().takeUtf8());
  else
    *this = std::move(a.right());
}

auto HostAddress::mode() const -> NetworkMode {
  return m_mode;
}

auto HostAddress::bytes() const -> uint8_t const* {
  return m_address.data();
}

auto HostAddress::octet(size_t i) const -> uint8_t {
  return m_address[i];
}

auto HostAddress::isLocalHost() const -> bool {
  if (m_mode == NetworkMode::IPv4) {
    return (m_address[0] == 127 && m_address[1] == 0 && m_address[2] == 0 && m_address[3] == 1);

  } else {
    for (size_t i = 0; i < 15; ++i) {
      if (m_address[i] != 0)
        return false;
    }

    return m_address[15] == 1;
  }
}

auto HostAddress::isZero() const -> bool {
  if (mode() == NetworkMode::IPv4)
    return m_address[0] == 0 && m_address[1] == 0 && m_address[2] == 0 && m_address[3] == 0;

  if (mode() == NetworkMode::IPv6) {
    for (size_t i = 0; i < 16; i++) {
      if (m_address[i] != 0)
        return false;
    }
    return true;
  }

  return false;
}

auto HostAddress::size() const -> size_t {
  switch (m_mode) {
    case NetworkMode::IPv4:
      return 4;
    case NetworkMode::IPv6:
      return 16;
    default:
      return 0;
  }
}

auto HostAddress::operator==(HostAddress const& a) const -> bool {
  if (m_mode != a.m_mode)
    return false;

  size_t len = a.size();
  for (size_t i = 0; i < len; i++) {
    if (m_address[i] != a.m_address[i])
      return false;
  }

  return true;
}

void HostAddress::set(String const& address) {
  if (address.empty())
    return;

  if (address.compare("*") == 0 || address.compare("0.0.0.0") == 0) {
    std::array<std::uint8_t, 4> inaddr_any{};
    set(NetworkMode::IPv4, inaddr_any.data());
  } else if (address.compare("::") == 0) {
    // NOTE: This will likely bind to both IPv6 and IPv4, but it does depending
    // on the OS settings
    std::array<std::uint8_t, 16> inaddr_any{};
    set(NetworkMode::IPv6, inaddr_any.data());
  } else {
    struct addrinfo* result = nullptr;
    struct addrinfo* ptr = nullptr;
    struct addrinfo hints;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    // Eliminate duplicates being returned one for each socket type.
    // As we're not using the return socket type or protocol this doesn't effect
    // us.
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    // Request only usable addresses e.g. IPv6 only if IPv6 is available
    hints.ai_flags = AI_ADDRCONFIG;

    if (::getaddrinfo(address.utf8Ptr(), nullptr, &hints, &result) != 0)
      throw NetworkException(strf("Failed to determine address for '{}' ({})", address, netErrorString()));

    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
      NetworkMode mode;
      switch (ptr->ai_family) {
        case AF_INET:
          mode = NetworkMode::IPv4;
          break;
        case AF_INET6:
          mode = NetworkMode::IPv6;
          break;
        default:
          continue;
      }
      if (mode == NetworkMode::IPv4) {
        auto* info = (struct sockaddr_in*)ptr->ai_addr;
        set(mode, (uint8_t*)(&info->sin_addr));
      } else {
        auto* info = (struct sockaddr_in6*)ptr->ai_addr;
        set(mode, (uint8_t*)(&info->sin6_addr));
      }
      break;
    }
    freeaddrinfo(result);
  }
}

void HostAddress::set(NetworkMode mode, uint8_t const* addr) {
  m_mode = mode;
  if (addr)
    std::memcpy(m_address.data(), addr, size());
  else
    std::memset(m_address.data(), 0, 16);
}

auto operator<<(std::ostream& os, HostAddress const& address) -> std::ostream& {
  switch (address.mode()) {
    case NetworkMode::IPv4:
      format(os, "{}.{}.{}.{}", address.octet(0), address.octet(1), address.octet(2), address.octet(3));
      break;

    case NetworkMode::IPv6:
      format(os,
          "{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}",
          address.octet(0),
          address.octet(1),
          address.octet(2),
          address.octet(3),
          address.octet(4),
          address.octet(5),
          address.octet(6),
          address.octet(7),
          address.octet(8),
          address.octet(9),
          address.octet(10),
          address.octet(11),
          address.octet(12),
          address.octet(13),
          address.octet(14),
          address.octet(15));
      break;

    default:
      throw NetworkException(strf("Unknown address mode ({})", (int)address.mode()));
  }
  return os;
}

auto hash<HostAddress>::operator()(HostAddress const& address) const -> size_t {
  PLHasher hash;
  for (size_t i = 0; i < address.size(); ++i)
    hash.put(address.octet(i));
  return hash.hash();
}

auto HostAddressWithPort::lookup(String const& address, uint16_t port) -> Either<String, HostAddressWithPort> {
  auto hostAddress = HostAddress::lookup(address);
  if (hostAddress.isLeft())
    return makeLeft(std::move(hostAddress.left()));
  else
    return makeRight(HostAddressWithPort(std::move(hostAddress.right()), port));
}

auto HostAddressWithPort::lookupWithPort(String const& address) -> Either<String, HostAddressWithPort> {
  String host = address;
  String port = host.rextract(":");
  if (host.beginsWith("[") && host.endsWith("]"))
    host = host.substr(1, host.size() - 2);

  auto portNum = maybeLexicalCast<uint16_t>(port);
  if (!portNum)
    return makeLeft(strf("Could not parse port portion of HostAddressWithPort '{}'", port));

  auto hostAddress = HostAddress::lookup(host);
  if (hostAddress.isLeft())
    return makeLeft(std::move(hostAddress.left()));

  return makeRight(HostAddressWithPort(std::move(hostAddress.right()), *portNum));
}

HostAddressWithPort::HostAddressWithPort() : m_port(0) {}

HostAddressWithPort::HostAddressWithPort(HostAddress const& address, uint16_t port)
  : m_address(address), m_port(port) {}

HostAddressWithPort::HostAddressWithPort(NetworkMode mode, uint8_t* address, uint16_t port) {
  m_address = HostAddress(mode, address);
  m_port = port;
}

HostAddressWithPort::HostAddressWithPort(String const& address, uint16_t port) {
  auto a = lookup(address, port);
  if (a.isLeft())
    throw NetworkException(a.left().takeUtf8());
  *this = std::move(a.right());
}

HostAddressWithPort::HostAddressWithPort(String const& address) {
  auto a = lookupWithPort(address);
  if (a.isLeft())
    throw NetworkException(a.left().takeUtf8());
  *this = std::move(a.right());
}

auto HostAddressWithPort::address() const -> HostAddress {
  return m_address;
}

auto HostAddressWithPort::port() const -> uint16_t {
  return m_port;
}

auto HostAddressWithPort::operator==(HostAddressWithPort const& rhs) const -> bool {
  return std::tie(m_address, m_port) == std::tie(rhs.m_address, rhs.m_port);
}

auto operator<<(std::ostream& os, HostAddressWithPort const& addressWithPort) -> std::ostream& {
  os << addressWithPort.address() << ":" << addressWithPort.port();
  return os;
}

auto hash<HostAddressWithPort>::operator()(HostAddressWithPort const& addressWithPort) const -> size_t {
  return hashOf(addressWithPort.address(), addressWithPort.port());
}

}
