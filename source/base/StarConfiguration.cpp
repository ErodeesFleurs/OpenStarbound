#include "StarConfiguration.hpp"

namespace Star {

Configuration::Configuration(Json defaultConfiguration, Json currentConfiguration)
  : m_defaultConfig(defaultConfiguration), m_currentConfig(currentConfiguration) {}

[[nodiscard]] Json Configuration::defaultConfiguration() const {
  return m_defaultConfig;
}

[[nodiscard]] Json Configuration::currentConfiguration() const {
  ReadLocker locker(m_mutex);
  return m_currentConfig;
}

[[nodiscard]] String Configuration::printConfiguration() const {
  ReadLocker locker(m_mutex);
  return m_currentConfig.printJson(2, true);
}

[[nodiscard]] Json Configuration::get(String const& key, Json def) const {
  ReadLocker locker(m_mutex);
  return m_currentConfig.get(key, def);
}

[[nodiscard]] Json Configuration::getPath(String const& path, Json def) const {
  ReadLocker locker(m_mutex);
  return m_currentConfig.query(path, def);
}

[[nodiscard]] Json Configuration::getDefault(String const& key) const {
  ReadLocker locker(m_mutex);
  return m_defaultConfig.get(key, {});
}

[[nodiscard]] Json Configuration::getDefaultPath(String const& path) const {
  ReadLocker locker(m_mutex);
  return m_defaultConfig.query(path, {});
}

void Configuration::set(String const& key, Json const& value) {
  WriteLocker locker(m_mutex);
  if (key == "configurationVersion")
    throw ConfigurationException("cannot set configurationVersion");

  if (value)
    m_currentConfig = m_currentConfig.set(key, value);
  else
    m_currentConfig = m_currentConfig.eraseKey(key);
}

void Configuration::setPath(String const& path, Json const& value) {
  WriteLocker locker(m_mutex);
  if (path.splitAny("[].").get(0) == "configurationVersion")
    throw ConfigurationException("cannot set configurationVersion");

  if (value)
    m_currentConfig = m_currentConfig.setPath(path, value);
  else
    m_currentConfig = m_currentConfig.erasePath(path);
}

}
