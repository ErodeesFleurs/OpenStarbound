module;

#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
import star.damage_types;
#include "StarPoly.hpp"
import star.world_geometry;
#include "StarStrongTypedef.hpp"
#include "StarIdMap.hpp"
import star.status_types;
import star.damage;
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
import star.damage_types;

namespace Star {

STAR_CLASS(World);
STAR_CLASS(Entity);
STAR_CLASS(DamageManager);

struct RemoteHitRequest {
  friend DataStream& operator<<(DataStream& ds, RemoteHitRequest const& hitRequest);
  friend DataStream& operator>>(DataStream& ds, RemoteHitRequest& hitRequest);

  ConnectionId destinationConnection() const;

  EntityId causingEntityId;
  EntityId targetEntityId;
  DamageRequest damageRequest;
};

DataStream& operator<<(DataStream& ds, RemoteHitRequest const& hitRequest);
DataStream& operator>>(DataStream& ds, RemoteHitRequest& hitRequest);

struct RemoteDamageRequest {
  friend DataStream& operator<<(DataStream& ds, RemoteDamageRequest const& damageRequest);
  friend DataStream& operator>>(DataStream& ds, RemoteDamageRequest& damageRequest);

  ConnectionId destinationConnection() const;

  EntityId causingEntityId;
  EntityId targetEntityId;
  DamageRequest damageRequest;
};

DataStream& operator<<(DataStream& ds, RemoteDamageRequest const& damageRequest);
DataStream& operator>>(DataStream& ds, RemoteDamageRequest& damageRequest);

struct RemoteDamageNotification {
  friend DataStream& operator<<(DataStream& ds, RemoteDamageNotification const& damageNotification);
  friend DataStream& operator>>(DataStream& ds, RemoteDamageNotification& damageNotification);

  EntityId sourceEntityId;
  DamageNotification damageNotification;
};

DataStream& operator<<(DataStream& ds, RemoteDamageNotification const& damageNotification);
DataStream& operator>>(DataStream& ds, RemoteDamageNotification& damageNotification);

// Right now, handles entity -> entity damage and ensures that no repeat damage
// is applied within the damage cutoff time from the same causing entity.
class DamageManager {
public:
  DamageManager(World* world, ConnectionId connectionId);

  // Notify entities that they have caused damage, apply damage to master
  // entities, produce damage notifications, and run down damage timeouts.
  void update(float dt);

  // Incoming RemoteHitRequest and RemoteDamageRequest must have the
  // destinationConnection equal to the DamageManager's connectionId

  void pushRemoteHitRequest(RemoteHitRequest const& remoteHitRequest);
  void pushRemoteDamageRequest(RemoteDamageRequest const& remoteDamageRequest);
  void pushRemoteDamageNotification(RemoteDamageNotification remoteDamageNotification);

  List<RemoteHitRequest> pullRemoteHitRequests();
  List<RemoteDamageRequest> pullRemoteDamageRequests();
  List<RemoteDamageNotification> pullRemoteDamageNotifications();

  // Pending *local* notifications.  Sum of all notifications either generated
  // locally or recieved.
  List<DamageNotification> pullPendingNotifications();

private:
  struct EntityDamageEvent {
    Variant<EntityId, String> timeoutGroup;
    float timeout;
  };

  // Searches for and queries for hit to any entity within range of the
  // damage source.  Skips over source.sourceEntityId, if set.
  SmallList<pair<EntityId, HitType>, 4> queryHit(DamageSource const& source, EntityId causingId) const;

  bool isAuthoritative(EntityPtr const& causingEntity, EntityPtr const& targetEntity);

  void addHitRequest(RemoteHitRequest const& remoteHitRequest);
  void addDamageRequest(RemoteDamageRequest remoteDamageRequest);
  void addDamageNotification(RemoteDamageNotification remoteDamageNotification);

  World* m_world;
  ConnectionId m_connectionId;

  // Maps target entity to all of the recent damage events that entity has
  // received, to prevent rapidly repeating damage.
  HashMap<EntityId, List<EntityDamageEvent>> m_recentEntityDamages;

  List<RemoteHitRequest> m_pendingRemoteHitRequests;
  List<RemoteDamageRequest> m_pendingRemoteDamageRequests;
  List<RemoteDamageNotification> m_pendingRemoteNotifications;
  List<DamageNotification> m_pendingNotifications;
};

}

export module star.damage_manager;

export namespace Star {
using ::Star::World;
using ::Star::WorldPtr;
using ::Star::WorldConstPtr;
using ::Star::WorldWeakPtr;
using ::Star::WorldConstWeakPtr;
using ::Star::WorldUPtr;
using ::Star::WorldConstUPtr;
using ::Star::Entity;
using ::Star::EntityPtr;
using ::Star::EntityConstPtr;
using ::Star::EntityWeakPtr;
using ::Star::EntityConstWeakPtr;
using ::Star::EntityUPtr;
using ::Star::EntityConstUPtr;
using ::Star::DamageManager;
using ::Star::DamageManagerPtr;
using ::Star::DamageManagerConstPtr;
using ::Star::DamageManagerWeakPtr;
using ::Star::DamageManagerConstWeakPtr;
using ::Star::DamageManagerUPtr;
using ::Star::DamageManagerConstUPtr;
using ::Star::RemoteHitRequest;
using ::Star::RemoteDamageRequest;
using ::Star::RemoteDamageNotification;
}
