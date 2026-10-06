module;

#include "StarNetElementSystem.hpp"
#include "StarJsonRpc.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
import star.damage_types;
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
import star.uuid;
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
#include "StarGameTypes.hpp"
import star.warping;
#include "StarWorldStorage.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarEither.hpp"
import star.player_types;

namespace Star {

STAR_CLASS(CelestialLog);
STAR_CLASS(ClientContext);

class ClientContext {
public:
  ClientContext(Uuid serverUuid, Uuid playerUuid);

  Uuid serverUuid() const;
  // The player Uuid can differ from the mainPlayer's Uuid
  //  if the player has swapped character - use this for ship saving.
  Uuid playerUuid() const;

  // The coordinate for the world which the player's ship is currently
  // orbiting.
  CelestialCoordinate shipCoordinate() const;

  Maybe<pair<WarpAction, WarpMode>> orbitWarpAction() const;

  // The current world id of the player
  WorldId playerWorldId() const;

  bool isAdmin() const;
  EntityDamageTeam team() const;

  JsonRpcInterfacePtr rpcInterface() const;

  WorldChunks newShipUpdates();
  ShipUpgrades shipUpgrades() const;
  
  StringMap<WorldChunks> newCustomWorldUpdates();

  void readUpdate(ByteArray data, NetCompatibilityRules rules);
  ByteArray writeUpdate(NetCompatibilityRules rules);

  void setConnectionId(ConnectionId connectionId);
  ConnectionId connectionId() const;

  void setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules);
  NetCompatibilityRules netCompatibilityRules() const;

private:
  Uuid m_serverUuid;
  Uuid m_playerUuid;
  ConnectionId m_connectionId = 0;
  NetCompatibilityRules m_netCompatibilityRules;

  JsonRpcPtr m_rpc;

  NetElementTopGroup m_netGroup;
  NetElementData<Maybe<pair<WarpAction, WarpMode>>> m_orbitWarpActionNetState;
  NetElementData<WorldId> m_playerWorldIdNetState;
  NetElementBool m_isAdminNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementData<ShipUpgrades> m_shipUpgrades;
  NetElementData<CelestialCoordinate> m_shipCoordinate;
  WorldChunks m_newShipUpdates;
  StringMap<WorldChunks> m_newCustomWorldUpdates;
};

}

export module star.client_context;

export namespace Star {
using ::Star::CelestialLog;
using ::Star::CelestialLogPtr;
using ::Star::CelestialLogConstPtr;
using ::Star::CelestialLogWeakPtr;
using ::Star::CelestialLogConstWeakPtr;
using ::Star::CelestialLogUPtr;
using ::Star::CelestialLogConstUPtr;
using ::Star::ClientContext;
using ::Star::ClientContextPtr;
using ::Star::ClientContextConstPtr;
using ::Star::ClientContextWeakPtr;
using ::Star::ClientContextConstWeakPtr;
using ::Star::ClientContextUPtr;
using ::Star::ClientContextConstUPtr;
}
