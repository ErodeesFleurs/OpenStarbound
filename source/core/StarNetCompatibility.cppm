export module star.net_compatibility;

import std;
import star.hash;
import star.version;

export namespace star {

constexpr version_number open_protocol_version = 14;

constexpr version_number AnyVersion = 0xFFFFFFFF;
constexpr version_number LegacyVersion = 0;

class net_compatibility_rules {
  public:
    net_compatibility_rules();
    net_compatibility_rules(std::uint64_t) = delete;
    explicit net_compatibility_rules(version_number version);

    [[nodiscard]] auto version() const -> version_number;
    void setVersion(version_number version);
    [[nodiscard]] auto isLegacy() const -> bool;

    auto operator==(net_compatibility_rules const& a) const -> bool;

  private:
    version_number m_version = open_protocol_version;
};

inline net_compatibility_rules::net_compatibility_rules() = default;

inline net_compatibility_rules::net_compatibility_rules(version_number v) : m_version(v) {}

inline auto net_compatibility_rules::version() const -> version_number { return m_version; }

inline void net_compatibility_rules::setVersion(version_number version) { m_version = version; }

inline auto net_compatibility_rules::isLegacy() const -> bool { return m_version == LegacyVersion; }

inline auto net_compatibility_rules::operator==(net_compatibility_rules const& a) const -> bool {
    return m_version == a.m_version;
}

template <> struct hash<net_compatibility_rules> {
    auto operator()(net_compatibility_rules const& s) const -> std::size_t { return s.version(); }
};

}// namespace star
