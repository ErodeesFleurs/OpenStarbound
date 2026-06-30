#pragma once

#include "StarJson.hpp"
#include "StarMap.hpp"

namespace Star {

class WorldServer;

class WorldServerProperties {
public:
  friend class WorldServer;

  WorldServerProperties() = default;
  explicit WorldServerProperties(function<void(JsonObject const&)> broadcastCallback);

  [[nodiscard]] Json getProperty(String const& propertyName, Json const& def = Json()) const;
  void setProperty(String const& propertyName, Json const& property);
  void setPropertyListener(String const& propertyName, function<void(Json const&)> listener);
  [[nodiscard]] JsonObject& properties();

private:
  function<void(JsonObject const&)> m_broadcastCallback;
  JsonObject m_worldProperties;
  StringMap<function<void(Json const&)>> m_worldPropertyListeners;
};

}
