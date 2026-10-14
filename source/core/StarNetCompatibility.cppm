module;
import std;
import star.hash;
import star.version;

namespace Star {

extern VersionNumber const OpenProtocolVersion;

inline constexpr VersionNumber AnyVersion = 0xFFFFFFFF;
inline constexpr VersionNumber LegacyVersion = 0;

class NetCompatibilityRules {
public:
  NetCompatibilityRules();
  NetCompatibilityRules(std::uint64_t) = delete;
  NetCompatibilityRules(VersionNumber version);

  VersionNumber version() const;
  void setVersion(VersionNumber version);

  bool isAdmin() const;
  void setIsAdmin(bool admin);

  bool isLegacy() const;

  bool operator==(NetCompatibilityRules const& a) const;

private:
  VersionNumber m_version = OpenProtocolVersion;
  bool m_isAdmin = false;
};

inline NetCompatibilityRules::NetCompatibilityRules() : m_version(OpenProtocolVersion) {}

inline NetCompatibilityRules::NetCompatibilityRules(VersionNumber v) : m_version(v) {}

inline VersionNumber NetCompatibilityRules::version() const {
  return m_version;
}

inline void NetCompatibilityRules::setVersion(VersionNumber version) {
  m_version = version;
}

inline bool NetCompatibilityRules::isAdmin() const {
  return m_isAdmin;
}

inline void NetCompatibilityRules::setIsAdmin(bool admin) {
  m_isAdmin = admin;
}

inline bool NetCompatibilityRules::isLegacy() const {
  return m_version == LegacyVersion;
}

inline bool NetCompatibilityRules::operator==(NetCompatibilityRules const& a) const {
  return m_version == a.m_version;
}

template <>
struct hash<NetCompatibilityRules> {
  std::size_t operator()(NetCompatibilityRules const& s) const {
    return s.version();
  }
};

}

export module star.net_compatibility;

export namespace Star {
  template <>
  struct hash<NetCompatibilityRules>;
}


export namespace Star {
  using ::Star::OpenProtocolVersion;
  using ::Star::AnyVersion;
  using ::Star::LegacyVersion;
  using ::Star::NetCompatibilityRules;
  using ::Star::hash;
}
