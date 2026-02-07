#pragma once
#include "StarHash.hpp"
#include "StarVersion.hpp"

import std;

namespace Star {

extern VersionNumber const OpenProtocolVersion;

constexpr VersionNumber AnyVersion = 0xFFFFFFFF;
constexpr VersionNumber LegacyVersion = 0;

class NetCompatibilityRules {
public:
  NetCompatibilityRules();
  NetCompatibilityRules(std::uint64_t) = delete;
  NetCompatibilityRules(VersionNumber version);

  [[nodiscard]] auto version() const -> VersionNumber;
  void setVersion(VersionNumber version);
  [[nodiscard]] auto isLegacy() const -> bool;

  auto operator==(NetCompatibilityRules const& a) const -> bool;

private:
  VersionNumber m_version = OpenProtocolVersion;
};

inline NetCompatibilityRules::NetCompatibilityRules() : m_version(OpenProtocolVersion) {}

inline NetCompatibilityRules::NetCompatibilityRules(VersionNumber v) : m_version(v) {}

inline auto NetCompatibilityRules::version() const -> VersionNumber {
  return m_version;
}

inline void NetCompatibilityRules::setVersion(VersionNumber version) {
  m_version = version;
}

inline auto NetCompatibilityRules::isLegacy() const -> bool {
  return m_version == LegacyVersion;
}

inline auto NetCompatibilityRules::operator==(NetCompatibilityRules const& a) const -> bool {
  return m_version == a.m_version;
}

template <>
struct hash<NetCompatibilityRules> {
  auto operator()(NetCompatibilityRules const& s) const -> std::size_t {
    return s.version();
  }
};

}// namespace Star
