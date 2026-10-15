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

class CelestialDatabase;

// Shared base class for UniverseClient and UniverseServer
class Universe {
public:
  struct Message {
    String message;
    JsonArray args;
    Variant<std::pair<Uuid,ConnectionId>,RpcPromiseKeeper<Json>> keeper;
  };

  virtual ~Universe() = default;
  
  virtual std::shared_ptr<CelestialDatabase> celestialDatabase() = 0;
  
  virtual void passMessage(Message&& message) = 0;
  // Sends a universe message to the target universe's main universe script context.
  virtual RpcPromise<Json> sendUniverseMessage(ConnectionId const& connectionId, String const& message, JsonArray const& args = {}) = 0;
  
  virtual Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool const& local, JsonArray const& args) = 0;
};

}

export module star.universe;

export namespace Star {
  using ::Star::Universe;
  using ::Star::CelestialDatabase;
}
