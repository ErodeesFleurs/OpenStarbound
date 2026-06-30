#pragma once

#include "StarJson.hpp"
#include "StarThread.hpp"
#include "StarVersion.hpp"

namespace Star {

class Configuration;
using ConfigurationPtr = SharedPtr<Configuration>;
using ConfigurationConstPtr = SharedPtr<Configuration const>;

struct ConfigurationExceptionTag { static constexpr char const* typeName = "ConfigurationException"; };
using ConfigurationException = TypedException<StarException, ConfigurationExceptionTag>;

class Configuration {
public:
  Configuration(Json defaultConfiguration, Json currentConfiguration);

  [[nodiscard]] Json defaultConfiguration() const;
  [[nodiscard]] Json currentConfiguration() const;
  [[nodiscard]] String printConfiguration() const;

  [[nodiscard]] Json get(String const& key, Json def = {}) const;
  [[nodiscard]] Json getPath(String const& path, Json def = {}) const;

  [[nodiscard]] Json getDefault(String const& key) const;
  [[nodiscard]] Json getDefaultPath(String const& path) const;

  void set(String const& key, Json const& value);
  void setPath(String const& path, Json const& value);

private:
  mutable Mutex m_mutex;

  Json m_defaultConfig;
  Json m_currentConfig;
};

}
