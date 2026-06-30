#pragma once

#include "StarRect.hpp"
#include "StarNetElementSystem.hpp"
#include "StarGameTypes.hpp"
#include "StarAssets.hpp"

namespace Star {

class WorldClientState;

// Class to aid in network syncronization of client state such as viewing area
// and player entity id.
class WorldClientState {
public:
  WorldClientState(AssetsConstPtr assets);

  // Actual area of the client visible screen (rounded to nearest block)
  [[nodiscard]] RectI window() const;
  void setWindow(RectI const& window);

  // Shortcut to find the window center of the client.
  [[nodiscard]] Vec2F windowCenter() const;

  // Entity of the unique main Player for this client
  [[nodiscard]] EntityId playerId() const;
  void setPlayer(EntityId playerId);

  // Entities that should contribute to the monitoring regions of the client.
  [[nodiscard]] List<EntityId> const& clientPresenceEntities() const;
  void setClientPresenceEntities(List<EntityId> entities);

  // All areas of the server monitored by the client, takes a function to
  // resolve an entity id to its bound box.
  [[nodiscard]] List<RectI> monitoringRegions(function<Maybe<RectI>(EntityId)> entityBounds) const;

  [[nodiscard]] ByteArray writeDelta();
  void readDelta(ByteArray delta);

  void setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules);
  [[nodiscard]] NetCompatibilityRules netCompatibilityRules() const;

  void reset();

private:
  int m_windowMonitoringBorder;
  int m_presenceEntityMonitoringBorder;

  NetElementTopGroup m_netGroup;
  uint64_t m_netVersion = 0;

  NetElementInt m_windowXMin;
  NetElementInt m_windowYMin;
  NetElementInt m_windowWidth;
  NetElementInt m_windowHeight;

  NetElementInt m_playerId;
  NetElementData<List<EntityId>> m_clientPresenceEntities;

  NetCompatibilityRules m_netCompatibilityRules;
};

}
