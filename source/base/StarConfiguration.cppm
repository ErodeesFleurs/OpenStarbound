module;

import std;
import star.json;
import star.thread;
import star.string;
import star.format;
import star.version;

namespace Star {

struct ConfigurationExceptionTag {
  static constexpr char const* name() { return "ConfigurationException"; }
};
using ConfigurationException = StarError<ConfigurationExceptionTag, StarException>;

class Configuration {
public:
  Configuration(Json defaultConfiguration, Json currentConfiguration);

  Json defaultConfiguration() const;
  Json currentConfiguration() const;
  String printConfiguration() const;

  Json get(String const& key, Json def = {}) const;
  Json getPath(String const& path, Json def = {}) const;

  Json getDefault(String const& key) const;
  Json getDefaultPath(String const& path) const;

  void set(String const& key, Json const& value);
  void setPath(String const& path, Json const& value);

private:
  mutable Mutex m_mutex;

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
