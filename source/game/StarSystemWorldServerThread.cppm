module;
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarStrongTypedef.hpp"
#include "StarEither.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarThread.hpp"
#include "StarNetCompatibility.hpp"


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

STAR_STRUCT(Packet);

STAR_CLASS(SystemWorldServerThread);

typedef function<void(SystemClientShip*)> ClientShipAction;

class SystemWorldServerThread : public Thread {
public:
  SystemWorldServerThread(Vec3I const& location, SystemWorldServerPtr systemWorld, String storageFile);
  ~SystemWorldServerThread();

  Vec3I location() const;

  List<ConnectionId> clients();
  void addClient(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation const& location);
  void removeClient(ConnectionId clientId);

  void setPause(shared_ptr<const atomic<bool>> pause);
  void run() override;
  void stop();

  void update();

  void setClientDestination(ConnectionId clientId, SystemLocation const& location);
  void executeClientShipAction(ConnectionId clientId, ClientShipAction action);

  SystemLocation clientShipLocation(ConnectionId clientId);
  Maybe<pair<WarpAction, WarpMode>> clientWarpAction(ConnectionId clientId);
  SkyParameters clientSkyParameters(ConnectionId clientId);

  List<InstanceWorldId> activeInstanceWorlds() const;

  // callback to be run after update in the server thread
  void setUpdateAction(function<void(SystemWorldServerThread*)> updateAction);
  void pushIncomingPacket(ConnectionId clientId, PacketPtr packet);
  List<PacketPtr> pullOutgoingPackets(ConnectionId clientId);

  void store();

private:
  Vec3I m_systemLocation;
  SystemWorldServerPtr m_systemWorld;

  atomic<bool> m_stop{false};
  float m_periodicStorage{300.0f};
  bool m_triggerStorage{ false};
  String m_storageFile;

  shared_ptr<const atomic<bool>> m_pause;
  function<void(SystemWorldServerThread*)> m_updateAction;

  ReadersWriterMutex m_mutex;
  ReadersWriterMutex m_queueMutex;

  HashSet<ConnectionId> m_clients;
  HashMap<ConnectionId, SystemLocation> m_clientShipDestinations;
  HashMap<ConnectionId, pair<SystemLocation, SkyParameters>> m_clientShipLocations;
  HashMap<ConnectionId, pair<WarpAction, WarpMode>> m_clientWarpActions;
  List<pair<ConnectionId, ClientShipAction>> m_clientShipActions;
  List<InstanceWorldId> m_activeInstanceWorlds;
  Map<ConnectionId, List<PacketPtr>> m_outgoingPacketQueue;
  List<pair<ConnectionId, PacketPtr>> m_incomingPacketQueue;
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
