#pragma once
#include "StarVersion.hpp"
#include "StarHash.hpp"

namespace Star {

extern VersionNumber const OpenProtocolVersion;

constexpr VersionNumber AnyVersion = 0xFFFFFFFF;
constexpr VersionNumber LegacyVersion = 0;

class NetCompatibilityRules {
public:
  NetCompatibilityRules() = default;
  NetCompatibilityRules(uint64_t) = delete;
  NetCompatibilityRules(VersionNumber version);

  [[nodiscard]] VersionNumber version() const;
  void setVersion(VersionNumber version);

  [[nodiscard]] bool isAdmin() const;
  void setIsAdmin(bool admin);

  [[nodiscard]] bool isLegacy() const;

  bool operator==(NetCompatibilityRules const& a) const;

private:
  VersionNumber m_version = OpenProtocolVersion;
  bool m_isAdmin = false;
};

inline NetCompatibilityRules::NetCompatibilityRules(VersionNumber v) : m_version(v) {}

[[nodiscard]] inline VersionNumber NetCompatibilityRules::version() const {
  return m_version;
}

inline void NetCompatibilityRules::setVersion(VersionNumber version) {
  m_version = version;
}

[[nodiscard]] inline bool NetCompatibilityRules::isAdmin() const {
  return m_isAdmin;
}

inline void NetCompatibilityRules::setIsAdmin(bool admin) {
  m_isAdmin = admin;
}

[[nodiscard]] inline bool NetCompatibilityRules::isLegacy() const {
  return m_version == LegacyVersion;
}

inline bool NetCompatibilityRules::operator==(NetCompatibilityRules const& a) const {
  return m_version == a.m_version;
}

template <>
struct hash<NetCompatibilityRules> {
  size_t operator()(NetCompatibilityRules const& s) const {
    return s.version();
  }
};

}
