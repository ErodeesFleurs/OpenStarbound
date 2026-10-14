module;
import std;
import star.format;
import star.stream_formatter;

import star.string;
import star.either;

namespace Star {

struct NetworkExceptionTag {
  static constexpr char const* name() { return "NetworkException"; }
};
using NetworkException = StarError<NetworkExceptionTag, IOException>;

class HostAddress;
using HostAddressPtr = std::shared_ptr<HostAddress>;
using HostAddressConstPtr = std::shared_ptr<const HostAddress>;
using HostAddressWeakPtr = std::weak_ptr<HostAddress>;
using HostAddressConstWeakPtr = std::weak_ptr<const HostAddress>;
using HostAddressUPtr = std::unique_ptr<HostAddress>;
using HostAddressConstUPtr = std::unique_ptr<const HostAddress>;

enum class NetworkMode {
  IPv4,
  IPv6
};

class HostAddress {
public:
  friend std::ostream& operator<<(std::ostream& os, HostAddress const& address);
  friend struct hash<HostAddress>;

  static HostAddress localhost(NetworkMode mode = NetworkMode::IPv4);

  // Returns either error or valid HostAddress
  static Either<String, HostAddress> lookup(String const& address);

  // The default address is the IPv4 zero address.
  HostAddress();
  // If address is nullptr, constructs the zero address for the given mode.
  HostAddress(NetworkMode mode, std::uint8_t* address = nullptr);
  // Throws if address is not valid
  explicit HostAddress(String const& address);

  NetworkMode mode() const;
  std::uint8_t const* bytes() const;
  std::uint8_t octet(std::size_t i) const;
  std::size_t size() const;

  bool isLocalHost() const;
  bool isZero() const;

  bool operator==(HostAddress const& a) const;

private:
  void set(String const& address);
  void set(NetworkMode mode, std::uint8_t const* addr);

  NetworkMode m_mode;
  std::uint8_t m_address[16];
};

std::ostream& operator<<(std::ostream& os, HostAddress const& address);

template <>
struct hash<HostAddress> {
  std::size_t operator()(HostAddress const& address) const;
};

class HostAddressWithPort {
public:
  friend std::ostream& operator<<(std::ostream& os, HostAddressWithPort const& address);
  friend struct hash<HostAddressWithPort>;

  // Returns either error or valid HostAddressWithPort
  static Either<String, HostAddressWithPort> lookup(String const& address, std::uint16_t port);
  // Format may have [] brackets around address or not, to distinguish address
  // portion from port portion.
  static Either<String, HostAddressWithPort> lookupWithPort(String const& address);

  HostAddressWithPort();
  HostAddressWithPort(HostAddress const& address, std::uint16_t port);
  HostAddressWithPort(NetworkMode mode, std::uint8_t* address, std::uint16_t port);
  // Throws if address or port is not valid
  HostAddressWithPort(String const& address, std::uint16_t port);
  explicit HostAddressWithPort(String const& address);

  HostAddress address() const;
  std::uint16_t port() const;

  bool operator==(HostAddressWithPort const& a) const;

private:
  HostAddress m_address;
  std::uint16_t m_port;
};

std::ostream& operator<<(std::ostream& os, HostAddressWithPort const& address);

template <>
struct hash<HostAddressWithPort> {
  std::size_t operator()(HostAddressWithPort const& address) const;
};

}

export module star.host_address;

export namespace Star {
  template <>
  struct hash<HostAddress>;
  template <>
  struct hash<HostAddressWithPort>;
}


export template <> struct std::formatter<Star::HostAddress> : Star::StreamFormatter {};
export template <> struct std::formatter<Star::HostAddressWithPort> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::NetworkExceptionTag;
  using ::Star::NetworkException;
  using ::Star::NetworkMode;
  using ::Star::HostAddressWithPort;
  using ::Star::HostAddress;
  using ::Star::HostAddressPtr;
  using ::Star::HostAddressConstPtr;
  using ::Star::HostAddressWeakPtr;
  using ::Star::HostAddressConstWeakPtr;
  using ::Star::HostAddressUPtr;
  using ::Star::HostAddressConstUPtr;
}
