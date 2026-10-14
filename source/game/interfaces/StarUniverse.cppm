module;
import star.json;
import std;
import star.string;
import star.variant;
import star.maybe;
import star.rpc_promise;
import star.game_types;
import star.data_stream;

import star.uuid;

namespace Star {

class Universe;
using UniversePtr = std::shared_ptr<Universe>;
using UniverseConstPtr = std::shared_ptr<const Universe>;
using UniverseWeakPtr = std::weak_ptr<Universe>;
using UniverseConstWeakPtr = std::weak_ptr<const Universe>;
using UniverseUPtr = std::unique_ptr<Universe>;
using UniverseConstUPtr = std::unique_ptr<const Universe>;
class CelestialDatabase;
using CelestialDatabasePtr = std::shared_ptr<CelestialDatabase>;
using CelestialDatabaseConstPtr = std::shared_ptr<const CelestialDatabase>;
using CelestialDatabaseWeakPtr = std::weak_ptr<CelestialDatabase>;
using CelestialDatabaseConstWeakPtr = std::weak_ptr<const CelestialDatabase>;
using CelestialDatabaseUPtr = std::unique_ptr<CelestialDatabase>;
using CelestialDatabaseConstUPtr = std::unique_ptr<const CelestialDatabase>;

// Shared base class for UniverseClient and UniverseServer
class Universe {
public:
  struct Message {
    String message;
    JsonArray args;
    Variant<std::pair<Uuid,ConnectionId>,RpcPromiseKeeper<Json>> keeper;
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
