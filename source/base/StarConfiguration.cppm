module;

import std;
import star.json;
import star.string;
import star.format;

namespace Star {

struct ConfigurationExceptionTag {
  static constexpr std::string_view name() { return "ConfigurationException"; }
};
using ConfigurationException = StarError<ConfigurationExceptionTag, StarException>;

class Configuration {
public:
  Configuration(Json defaultConfiguration, Json currentConfiguration);

  Json defaultConfiguration() const;
  Json currentConfiguration() const;
  [[nodiscard]] std::expected<String, StarException> printConfiguration() const;

  [[nodiscard]] std::expected<Json, StarException> get(String const& key, Json def = {}) const;
  [[nodiscard]] std::expected<Json, StarException> getPath(String const& path, Json def = {}) const;

  [[nodiscard]] std::expected<Json, StarException> getDefault(String const& key) const;
  [[nodiscard]] std::expected<Json, StarException> getDefaultPath(String const& path) const;

  [[nodiscard]] std::expected<void, StarException> set(String const& key, Json const& value);
  [[nodiscard]] std::expected<void, StarException> setPath(String const& path, Json const& value);

private:
  mutable std::mutex m_mutex;

  Json m_defaultConfig;
  Json m_currentConfig;
};

}

export module star.configuration;

export namespace Star {
  using ::Star::ConfigurationExceptionTag;
  using ::Star::ConfigurationException;
  using ::Star::Configuration;
}
