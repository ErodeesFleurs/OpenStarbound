#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarThread.hpp"

namespace Star {

using ConfigurationException = ExceptionDerived<"ConfigurationException">;

class Configuration {
public:
  Configuration(Json defaultConfiguration, Json currentConfiguration);

  auto defaultConfiguration() const -> Json;
  auto currentConfiguration() const -> Json;
  auto printConfiguration() const -> String;

  auto get(String const& key, Json def = {}) const -> Json;
  auto getPath(String const& path, Json def = {}) const -> Json;

  auto getDefault(String const& key) const -> Json;
  auto getDefaultPath(String const& path) const -> Json;

  void set(String const& key, Json const& value);
  void setPath(String const& path, Json const& value);

private:
  mutable Mutex m_mutex;

  Json m_defaultConfig;
  Json m_currentConfig;
};

}// namespace Star
