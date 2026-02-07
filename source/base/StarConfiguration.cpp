#include "StarConfiguration.hpp"

import std;

namespace Star {

Configuration::Configuration(Json defaultConfiguration, Json currentConfiguration)
    : m_defaultConfig(std::move(defaultConfiguration)), m_currentConfig(std::move(currentConfiguration)) {}

auto Configuration::defaultConfiguration() const -> Json {
  return m_defaultConfig;
}

auto Configuration::currentConfiguration() const -> Json {
  MutexLocker locker(m_mutex);
  return m_currentConfig;
}

auto Configuration::printConfiguration() const -> String {
  MutexLocker locker(m_mutex);
  return m_currentConfig.printJson(2, true);
}

auto Configuration::get(String const& key, Json def) const -> Json {
  MutexLocker locker(m_mutex);
  return m_currentConfig.get(key, def);
}

auto Configuration::getPath(String const& path, Json def) const -> Json {
  MutexLocker locker(m_mutex);
  return m_currentConfig.query(path, def);
}

auto Configuration::getDefault(String const& key) const -> Json {
  MutexLocker locker(m_mutex);
  return m_defaultConfig.get(key, {});
}

auto Configuration::getDefaultPath(String const& path) const -> Json {
  MutexLocker locker(m_mutex);
  return m_defaultConfig.query(path, {});
}

void Configuration::set(String const& key, Json const& value) {
  MutexLocker locker(m_mutex);
  if (key == "configurationVersion")
    throw ConfigurationException("cannot set configurationVersion");

  if (value)
    m_currentConfig = m_currentConfig.set(key, value);
  else
    m_currentConfig = m_currentConfig.eraseKey(key);
}

void Configuration::setPath(String const& path, Json const& value) {
  MutexLocker locker(m_mutex);
  if (path.splitAny("[].").get(0) == "configurationVersion")
    throw ConfigurationException("cannot set configurationVersion");

  if (value)
    m_currentConfig = m_currentConfig.setPath(path, value);
  else
    m_currentConfig = m_currentConfig.erasePath(path);
}

}// namespace Star
