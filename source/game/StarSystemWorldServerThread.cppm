module;
import std;
import star.string;
import star.map;
import star.set;
import star.json;
import star.vector;
import star.variant;
import star.game_types;
import star.color;
import star.bi_map;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarStrongTypedef.hpp"
import star.either;
import star.net_compatibility;


import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.uuid;
import star.warping;
import star.sky_parameters;
import star.system_world;

namespace Star {

struct Packet;
using PacketPtr = std::shared_ptr<Packet>;
using PacketConstPtr = std::shared_ptr<const Packet>;
using PacketWeakPtr = std::weak_ptr<Packet>;
using PacketConstWeakPtr = std::weak_ptr<const Packet>;
using PacketUPtr = std::unique_ptr<Packet>;
using PacketConstUPtr = std::unique_ptr<const Packet>;

class SystemWorldServerThread;
using SystemWorldServerThreadPtr = std::shared_ptr<SystemWorldServerThread>;
using SystemWorldServerThreadConstPtr = std::shared_ptr<const SystemWorldServerThread>;
using SystemWorldServerThreadWeakPtr = std::weak_ptr<SystemWorldServerThread>;
using SystemWorldServerThreadConstWeakPtr = std::weak_ptr<const SystemWorldServerThread>;
using SystemWorldServerThreadUPtr = std::unique_ptr<SystemWorldServerThread>;
using SystemWorldServerThreadConstUPtr = std::unique_ptr<const SystemWorldServerThread>;

typedef std::function<void(SystemClientShip*)> ClientShipAction;

class SystemWorldServerThread : public Thread {
public:
  SystemWorldServerThread(Vec3I const& location, SystemWorldServerPtr systemWorld, String storageFile);
  ~SystemWorldServerThread();

  Vec3I location() const;

  List<ConnectionId> clients();
  void addClient(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation const& location);
  void removeClient(ConnectionId clientId);

  void setPause(std::shared_ptr<const std::atomic<bool>> pause);
  void run() override;
  void stop();

  void update();

  void setClientDestination(ConnectionId clientId, SystemLocation const& location);
  void executeClientShipAction(ConnectionId clientId, ClientShipAction action);

  SystemLocation clientShipLocation(ConnectionId clientId);
  Maybe<std::pair<WarpAction, WarpMode>> clientWarpAction(ConnectionId clientId);
  SkyParameters clientSkyParameters(ConnectionId clientId);

  List<InstanceWorldId> activeInstanceWorlds() const;

  // callback to be run after update in the server thread
  void setUpdateAction(std::function<void(SystemWorldServerThread*)> updateAction);
  void pushIncomingPacket(ConnectionId clientId, PacketPtr packet);
  List<PacketPtr> pullOutgoingPackets(ConnectionId clientId);

  void store();

private:
  Vec3I m_systemLocation;
  SystemWorldServerPtr m_systemWorld;

  std::atomic<bool> m_stop{false};
  float m_periodicStorage{300.0f};
  bool m_triggerStorage{ false};
  String m_storageFile;

  std::shared_ptr<const std::atomic<bool>> m_pause;
  std::function<void(SystemWorldServerThread*)> m_updateAction;

  ReadersWriterMutex m_mutex;
  ReadersWriterMutex m_queueMutex;

  HashSet<ConnectionId> m_clients;
  HashMap<ConnectionId, SystemLocation> m_clientShipDestinations;
  HashMap<ConnectionId, std::pair<SystemLocation, SkyParameters>> m_clientShipLocations;
  HashMap<ConnectionId, std::pair<WarpAction, WarpMode>> m_clientWarpActions;
  List<std::pair<ConnectionId, ClientShipAction>> m_clientShipActions;
  List<InstanceWorldId> m_activeInstanceWorlds;
  Map<ConnectionId, List<PacketPtr>> m_outgoingPacketQueue;
  List<std::pair<ConnectionId, PacketPtr>> m_incomingPacketQueue;
};


}

export module star.system_world_server_thread;

export namespace Star {
  using ::Star::ClientShipAction;
  using ::Star::SystemWorldServerThread;
  using ::Star::SystemWorldServerThreadPtr;
  using ::Star::SystemWorldServerThreadConstPtr;
  using ::Star::SystemWorldServerThreadWeakPtr;
  using ::Star::SystemWorldServerThreadConstWeakPtr;
  using ::Star::SystemWorldServerThreadUPtr;
  using ::Star::SystemWorldServerThreadConstUPtr;
}
