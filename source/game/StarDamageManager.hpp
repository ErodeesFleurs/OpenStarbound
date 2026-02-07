#pragma once

#include "StarConfig.hpp"
#include "StarDamage.hpp"

import std;

namespace Star {

class World;
class Entity;
// STAR_CLASS(DamageManager);

struct RemoteHitRequest {
  [[nodiscard]] auto destinationConnection() const -> ConnectionId;

  EntityId causingEntityId;
  EntityId targetEntityId;
  DamageRequest damageRequest;
};

auto operator<<(DataStream& ds, RemoteHitRequest const& hitRequest) -> DataStream&;
auto operator>>(DataStream& ds, RemoteHitRequest& hitRequest) -> DataStream&;

struct RemoteDamageRequest {
  [[nodiscard]] auto destinationConnection() const -> ConnectionId;

  EntityId causingEntityId;
  EntityId targetEntityId;
  DamageRequest damageRequest;
};

auto operator<<(DataStream& ds, RemoteDamageRequest const& damageRequest) -> DataStream&;
auto operator>>(DataStream& ds, RemoteDamageRequest& damageRequest) -> DataStream&;

struct RemoteDamageNotification {
  EntityId sourceEntityId;
  DamageNotification damageNotification;
};

auto operator<<(DataStream& ds, RemoteDamageNotification const& damageNotification) -> DataStream&;
auto operator>>(DataStream& ds, RemoteDamageNotification& damageNotification) -> DataStream&;

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

  auto pullRemoteHitRequests() -> List<RemoteHitRequest>;
  auto pullRemoteDamageRequests() -> List<RemoteDamageRequest>;
  auto pullRemoteDamageNotifications() -> List<RemoteDamageNotification>;

  // Pending *local* notifications.  Sum of all notifications either generated
  // locally or recieved.
  auto pullPendingNotifications() -> List<DamageNotification>;

private:
  struct EntityDamageEvent {
    Variant<EntityId, String> timeoutGroup;
    float timeout;
  };

  // Searches for and queries for hit to any entity within range of the
  // damage source.  Skips over source.sourceEntityId, if set.
  [[nodiscard]] auto queryHit(DamageSource const& source, EntityId causingId) const -> SmallList<std::pair<EntityId, HitType>, 4>;

  auto isAuthoritative(Ptr<Entity> const& causingEntity, Ptr<Entity> const& targetEntity) -> bool;

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

}// namespace Star
