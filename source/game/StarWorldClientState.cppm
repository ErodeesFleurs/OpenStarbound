module;
import std;
import star.list;
import star.maybe;

import star.rect;
import star.vector;
import star.net_compatibility;
import star.game_types;

import star.byte_array;
import star.net_element_basic_fields;
import star.net_element_system;

namespace Star {

class WorldClientState;
using WorldClientStatePtr = std::shared_ptr<WorldClientState>;
using WorldClientStateConstPtr = std::shared_ptr<const WorldClientState>;
using WorldClientStateWeakPtr = std::weak_ptr<WorldClientState>;
using WorldClientStateConstWeakPtr = std::weak_ptr<const WorldClientState>;
using WorldClientStateUPtr = std::unique_ptr<WorldClientState>;
using WorldClientStateConstUPtr = std::unique_ptr<const WorldClientState>;

// Class to aid in network syncronization of client state such as viewing area
// and player entity id.
class WorldClientState {
public:
  WorldClientState();

  // Actual area of the client visible screen (rounded to nearest block)
  RectI window() const;
  void setWindow(RectI const& window);

  // Shortcut to find the window center of the client.
  Vec2F windowCenter() const;

  // Entity of the unique main Player for this client
  EntityId playerId() const;
  void setPlayer(EntityId playerId);

  // Entities that should contribute to the monitoring regions of the client.
  List<EntityId> const& clientPresenceEntities() const;
  void setClientPresenceEntities(List<EntityId> entities);

  // All areas of the server monitored by the client, takes a function to
  // resolve an entity id to its bound box.
  List<RectI> monitoringRegions(std::function<Maybe<RectI>(EntityId)> entityBounds) const;

  ByteArray writeDelta();
  void readDelta(ByteArray delta);

  void setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules);
  NetCompatibilityRules netCompatibilityRules() const;

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

}

export module star.world_client_state;

export namespace Star {
using ::Star::WorldClientState;
using ::Star::WorldClientStatePtr;
using ::Star::WorldClientStateConstPtr;
using ::Star::WorldClientStateWeakPtr;
using ::Star::WorldClientStateConstWeakPtr;
using ::Star::WorldClientStateUPtr;
using ::Star::WorldClientStateConstUPtr;
}
