module;
#include "StarConfig.hpp"
#include "StarGameTypes.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"

import star.uuid;

namespace Star {

STAR_CLASS(Universe);
STAR_CLASS(CelestialDatabase);

// Shared base class for UniverseClient and UniverseServer
class Universe {
public:
  struct Message {
    String message;
    JsonArray args;
    Variant<pair<Uuid,ConnectionId>,RpcPromiseKeeper<Json>> keeper;
  };

  virtual ~Universe() = default;
  
  virtual CelestialDatabasePtr celestialDatabase() = 0;
  
  virtual void passMessage(Message&& message) = 0;
  // Sends a universe message to the target universe's main universe script context.
  virtual RpcPromise<Json> sendUniverseMessage(ConnectionId const& connectionId, String const& message, JsonArray const& args = {}) = 0;
  
  virtual Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool const& local, JsonArray const& args) = 0;
};

}

export module star.universe;

export namespace Star {
  using ::Star::Universe;
  using ::Star::UniversePtr;
  using ::Star::UniverseConstPtr;
  using ::Star::UniverseWeakPtr;
  using ::Star::UniverseConstWeakPtr;
  using ::Star::UniverseUPtr;
  using ::Star::UniverseConstUPtr;
  using ::Star::CelestialDatabase;
  using ::Star::CelestialDatabasePtr;
  using ::Star::CelestialDatabaseConstPtr;
  using ::Star::CelestialDatabaseWeakPtr;
  using ::Star::CelestialDatabaseConstWeakPtr;
  using ::Star::CelestialDatabaseUPtr;
  using ::Star::CelestialDatabaseConstUPtr;
}
