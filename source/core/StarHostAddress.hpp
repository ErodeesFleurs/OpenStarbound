#pragma once

#include "StarException.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"

import std;

namespace Star {

using NetworkException = ExceptionDerived<"IOException">;

enum class NetworkMode {
  IPv4,
  IPv6
};

class HostAddress {
public:
  static auto localhost(NetworkMode mode = NetworkMode::IPv4) -> HostAddress;

  // Returns either error or valid HostAddress
  static auto lookup(String const& address) -> Either<String, HostAddress>;

  // If address is nullptr, constructs the zero address.
  HostAddress(NetworkMode mode = NetworkMode::IPv4, std::uint8_t* address = nullptr);
  // Throws if address is not valid
  explicit HostAddress(String const& address);

  [[nodiscard]] auto mode() const -> NetworkMode;
  [[nodiscard]] auto bytes() const -> std::uint8_t const*;
  [[nodiscard]] auto octet(std::size_t i) const -> std::uint8_t;
  [[nodiscard]] auto size() const -> std::size_t;

  [[nodiscard]] auto isLocalHost() const -> bool;
  [[nodiscard]] auto isZero() const -> bool;

  auto operator==(HostAddress const& a) const -> bool;

private:
  void set(String const& address);
  void set(NetworkMode mode, std::uint8_t const* addr);

  NetworkMode m_mode;
  std::array<std::uint8_t, 16> m_address;
};

auto operator<<(std::ostream& os, HostAddress const& address) -> std::ostream&;

template <>
struct hash<HostAddress> {
  auto operator()(HostAddress const& address) const -> std::size_t;
};

class HostAddressWithPort {
public:
  // Returns either error or valid HostAddressWithPort
  static auto lookup(String const& address, std::uint16_t port) -> Either<String, HostAddressWithPort>;
  // Format may have [] brackets around address or not, to distinguish address
  // portion from port portion.
  static auto lookupWithPort(String const& address) -> Either<String, HostAddressWithPort>;

  HostAddressWithPort();
  HostAddressWithPort(HostAddress const& address, std::uint16_t port);
  HostAddressWithPort(NetworkMode mode, std::uint8_t* address, std::uint16_t port);
  // Throws if address or port is not valid
  HostAddressWithPort(String const& address, std::uint16_t port);
  explicit HostAddressWithPort(String const& address);

  [[nodiscard]] auto address() const -> HostAddress;
  [[nodiscard]] auto port() const -> std::uint16_t;

  auto operator==(HostAddressWithPort const& a) const -> bool;

private:
  HostAddress m_address;
  std::uint16_t m_port;
};

auto operator<<(std::ostream& os, HostAddressWithPort const& address) -> std::ostream&;

template <>
struct hash<HostAddressWithPort> {
  auto operator()(HostAddressWithPort const& address) const -> std::size_t;
};

}

template <> struct std::formatter<Star::HostAddress> : Star::ostream_formatter {};
template <> struct std::formatter<Star::HostAddressWithPort> : Star::ostream_formatter {};
