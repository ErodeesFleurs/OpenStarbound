import std;
import star.format;
import star.json;
import star.thread;
import star.string;
import star.version;
import star.configuration;
import star.file;
import star.logging;

namespace Star {

Configuration::Configuration(Json defaultConfiguration, Json currentConfiguration)
  : m_defaultConfig(defaultConfiguration), m_currentConfig(currentConfiguration) {}

Json Configuration::defaultConfiguration() const {
  return m_defaultConfig;
}

Json Configuration::currentConfiguration() const {
  MutexLocker locker(m_mutex);
  return m_currentConfig;
}

std::expected<String, StarException> Configuration::printConfiguration() const {
  MutexLocker locker(m_mutex);
  return m_currentConfig.printJson(2, true);
}

std::expected<Json, StarException> Configuration::get(String const& key, Json def) const {
  MutexLocker locker(m_mutex);
  return m_currentConfig.get(key, def);
}

std::expected<Json, StarException> Configuration::getPath(String const& path, Json def) const {
  MutexLocker locker(m_mutex);
  return m_currentConfig.query(path, def);
}

std::expected<Json, StarException> Configuration::getDefault(String const& key) const {
  MutexLocker locker(m_mutex);
  return m_defaultConfig.get(key, {});
}

std::expected<Json, StarException> Configuration::getDefaultPath(String const& path) const {
  MutexLocker locker(m_mutex);
  return m_defaultConfig.query(path, {});
}

std::expected<void, StarException> Configuration::set(String const& key, Json const& value) {
  MutexLocker locker(m_mutex);
  if (key == "configurationVersion")
    return std::unexpected(ConfigurationException("cannot set configurationVersion"));

  auto updated = value ? m_currentConfig.set(key, value) : m_currentConfig.eraseKey(key);
  if (!updated)
    return std::unexpected(std::move(updated.error()));
  m_currentConfig = std::move(*updated);
  return {};
}

std::expected<void, StarException> Configuration::setPath(String const& path, Json const& value) {
  MutexLocker locker(m_mutex);
  auto components = path.splitAny("[].");
  if (!components)
    return std::unexpected(std::move(components.error()));
  if (!components->empty() && (*components)[0] == "configurationVersion")
    return std::unexpected(ConfigurationException("cannot set configurationVersion"));

  auto updated = value ? m_currentConfig.setPath(path, value) : m_currentConfig.erasePath(path);
  if (!updated)
    return std::unexpected(std::move(updated.error()));
  m_currentConfig = std::move(*updated);
  return {};
}

}
