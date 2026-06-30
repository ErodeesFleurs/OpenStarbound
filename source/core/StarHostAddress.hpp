#pragma once

#include "StarString.hpp"
#include "StarEither.hpp"

namespace Star {

struct NetworkExceptionTag { static constexpr char const* typeName = "NetworkException"; };
using NetworkException = TypedException<IOException, NetworkExceptionTag>;

class HostAddress;

enum class NetworkMode {
  IPv4,
  IPv6
};

class HostAddress {
public:
  [[nodiscard]] static HostAddress localhost(NetworkMode mode = NetworkMode::IPv4);

  // Returns either error or valid HostAddress
  [[nodiscard]] static Either<String, HostAddress> lookup(String const& address);

  // If address is nullptr, constructs the zero address.
  HostAddress(NetworkMode mode = NetworkMode::IPv4, uint8_t* address = nullptr);
  // Throws if address is not valid
  explicit HostAddress(String const& address);

  [[nodiscard]] NetworkMode mode() const;
  [[nodiscard]] uint8_t const* bytes() const;
  [[nodiscard]] uint8_t octet(size_t i) const;
  [[nodiscard]] size_t size() const;

  [[nodiscard]] bool isLocalHost() const;
  [[nodiscard]] bool isZero() const;

  bool operator==(HostAddress const& a) const;

private:
  void set(String const& address);
  void set(NetworkMode mode, uint8_t const* addr);

  NetworkMode m_mode;
  uint8_t m_address[16];
};

std::ostream& operator<<(std::ostream& os, HostAddress const& address);

template <>
struct hash<HostAddress> {
  size_t operator()(HostAddress const& address) const;
};

class HostAddressWithPort {
public:
  // Returns either error or valid HostAddressWithPort
  [[nodiscard]] static Either<String, HostAddressWithPort> lookup(String const& address, uint16_t port);
  // Format may have [] brackets around address or not, to distinguish address
  // portion from port portion.
  [[nodiscard]] static Either<String, HostAddressWithPort> lookupWithPort(String const& address);

  HostAddressWithPort();
  HostAddressWithPort(HostAddress const& address, uint16_t port);
  HostAddressWithPort(NetworkMode mode, uint8_t* address, uint16_t port);
  // Throws if address or port is not valid
  HostAddressWithPort(String const& address, uint16_t port);
  explicit HostAddressWithPort(String const& address);

  [[nodiscard]] HostAddress address() const;
  [[nodiscard]] uint16_t port() const;

  bool operator==(HostAddressWithPort const& a) const;

private:
  HostAddress m_address;
  uint16_t m_port;
};

std::ostream& operator<<(std::ostream& os, HostAddressWithPort const& address);

template <>
struct hash<HostAddressWithPort> {
  size_t operator()(HostAddressWithPort const& address) const;
};

}

template <> struct std::formatter<Star::HostAddress> : Star::OstreamFormatter {};
template <> struct std::formatter<Star::HostAddressWithPort> : Star::OstreamFormatter {};
