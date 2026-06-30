#include "StarWorldServerProperties.hpp"
#include "StarNetPackets.hpp"

namespace Star {

WorldServerProperties::WorldServerProperties(function<void(JsonObject const&)> broadcastCallback)
  : m_broadcastCallback(std::move(broadcastCallback)) {}

Json WorldServerProperties::getProperty(String const& propertyName, Json const& def) const {
  return m_worldProperties.value(propertyName, def);
}

void WorldServerProperties::setProperty(String const& propertyName, Json const& property) {
  auto entry = m_worldProperties.find(propertyName);
  bool missing = entry == m_worldProperties.end();
  if (missing ? !property.isNull() : property != entry->second) {
    if (missing)
      m_worldProperties.emplace(propertyName, property);
    else if (property.isNull())
      m_worldProperties.erase(entry);
    else
      entry->second = property;
    if (m_broadcastCallback)
      m_broadcastCallback(JsonObject{{propertyName, property}});
  }
  auto listener = m_worldPropertyListeners.find(propertyName);
  if (listener != m_worldPropertyListeners.end())
    listener->second(property);
}

void WorldServerProperties::setPropertyListener(String const& propertyName, function<void(Json const&)> listener) {
  if (listener)
    m_worldPropertyListeners[propertyName] = listener;
  else
    m_worldPropertyListeners.erase(propertyName);
}

JsonObject& WorldServerProperties::properties() {
  return m_worldProperties;
}

}
