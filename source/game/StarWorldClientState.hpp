#pragma once

#include "StarGameTypes.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRect.hpp"

import std;

namespace Star {

// Class to aid in network syncronization of client state such as viewing area
// and player entity id.
class WorldClientState {
public:
  WorldClientState();

  // Actual area of the client visible screen (rounded to nearest block)
  auto window() const -> RectI;
  void setWindow(RectI const& window);

  // Shortcut to find the window center of the client.
  auto windowCenter() const -> Vec2F;

  // Entity of the unique main Player for this client
  auto playerId() const -> EntityId;
  void setPlayer(EntityId playerId);

  // Entities that should contribute to the monitoring regions of the client.
  auto clientPresenceEntities() const -> List<EntityId> const&;
  void setClientPresenceEntities(List<EntityId> entities);

  // All areas of the server monitored by the client, takes a function to
  // resolve an entity id to its bound box.
  auto monitoringRegions(std::function<std::optional<RectI>(EntityId)> entityBounds) const -> List<RectI>;

  auto writeDelta() -> ByteArray;
  void readDelta(ByteArray delta);

  void setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules);
  auto netCompatibilityRules() const -> NetCompatibilityRules;

  void reset();

private:
  int m_windowMonitoringBorder;
  int m_presenceEntityMonitoringBorder;

  NetElementTopGroup m_netGroup;
  std::uint64_t m_netVersion;

  NetElementInt m_windowXMin;
  NetElementInt m_windowYMin;
  NetElementInt m_windowWidth;
  NetElementInt m_windowHeight;

  NetElementInt m_playerId;
  NetElementData<List<EntityId>> m_clientPresenceEntities;

  NetCompatibilityRules m_netCompatibilityRules;
};

}// namespace Star
