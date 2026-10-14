module;

import star.vector;
import star.game_types;
import star.world_geometry;
import std;
import star.list;
import star.map;
import star.data_stream;
import star.wiring;

namespace Star {

class WireEntity;
using WireEntityPtr = std::shared_ptr<WireEntity>;
using WireEntityConstPtr = std::shared_ptr<const WireEntity>;
using WireEntityWeakPtr = std::weak_ptr<WireEntity>;
using WireEntityConstWeakPtr = std::weak_ptr<const WireEntity>;
using WireEntityUPtr = std::unique_ptr<WireEntity>;
using WireEntityConstUPtr = std::unique_ptr<const WireEntity>;
class WorldStorage;
using WorldStoragePtr = std::shared_ptr<WorldStorage>;
using WorldStorageConstPtr = std::shared_ptr<const WorldStorage>;
using WorldStorageWeakPtr = std::weak_ptr<WorldStorage>;
using WorldStorageConstWeakPtr = std::weak_ptr<const WorldStorage>;
using WorldStorageUPtr = std::unique_ptr<WorldStorage>;
using WorldStorageConstUPtr = std::unique_ptr<const WorldStorage>;

class WireProcessor;
using WireProcessorPtr = std::shared_ptr<WireProcessor>;
using WireProcessorConstPtr = std::shared_ptr<const WireProcessor>;
using WireProcessorWeakPtr = std::weak_ptr<WireProcessor>;
using WireProcessorConstWeakPtr = std::weak_ptr<const WireProcessor>;
using WireProcessorUPtr = std::unique_ptr<WireProcessor>;
using WireProcessorConstUPtr = std::unique_ptr<const WireProcessor>;

// Propogates WireEntity signals, and keeps networks of WireEntities alive
// together.
class WireProcessor : public WireCoordinator {
public:
  WireProcessor(WorldStoragePtr worldStorage);

  void process();

  bool readInputConnection(WireConnection const& connection) override;

private:
  struct WireEntityState {
    WireEntity* wireEntity;
    List<bool> outputStates;
    bool networkLoaded;
  };

  // Add the given WireEntity to the working entities set, populating inbound /
  // outbound nodes and states.
  void populateWorking(WireEntity* wireEntity);
  // Scans a wire network, starting at an entity at the given position, while
  // also loading any unloaded entries in the network and marking each entry as
  // now having been 'networkLoaded'.
  void loadNetwork(Vec2I tilePosition);

  WorldStoragePtr m_worldStorage;
  StableHashMap<Vec2I, WireEntityState> m_workingWireEntities;
};

}

export module star.wire_processor;

export namespace Star {
using ::Star::WireProcessor;
using ::Star::WireProcessorPtr;
using ::Star::WireProcessorConstPtr;
using ::Star::WireProcessorWeakPtr;
using ::Star::WireProcessorConstWeakPtr;
using ::Star::WireProcessorUPtr;
using ::Star::WireProcessorConstUPtr;
using ::Star::WireEntity;
using ::Star::WireEntityPtr;
using ::Star::WireEntityConstPtr;
using ::Star::WireEntityWeakPtr;
using ::Star::WireEntityConstWeakPtr;
using ::Star::WireEntityUPtr;
using ::Star::WireEntityConstUPtr;
using ::Star::WorldStorage;
using ::Star::WorldStoragePtr;
using ::Star::WorldStorageConstPtr;
using ::Star::WorldStorageWeakPtr;
using ::Star::WorldStorageConstWeakPtr;
using ::Star::WorldStorageUPtr;
using ::Star::WorldStorageConstUPtr;
using ::Star::WireDirection;
using ::Star::otherWireDirection;
using ::Star::WireNode;
using ::Star::WireConnection;
using ::Star::WireCoordinator;
using ::Star::WireCoordinatorPtr;
using ::Star::WireCoordinatorConstPtr;
using ::Star::WireCoordinatorWeakPtr;
using ::Star::WireCoordinatorConstWeakPtr;
using ::Star::WireCoordinatorUPtr;
using ::Star::WireCoordinatorConstUPtr;
using ::Star::WireConnector;
using ::Star::WireConnectorPtr;
using ::Star::WireConnectorConstPtr;
using ::Star::WireConnectorWeakPtr;
using ::Star::WireConnectorConstWeakPtr;
using ::Star::WireConnectorUPtr;
using ::Star::WireConnectorConstUPtr;
}
