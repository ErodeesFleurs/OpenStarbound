#include "StarDamageManager.hpp"
#include "StarColor.hpp"
#include "StarConfig.hpp"
#include "StarIterator.hpp"
#include "StarLogging.hpp"
#include "StarWorld.hpp"

import std;

namespace Star {

auto RemoteHitRequest::destinationConnection() const -> ConnectionId {
  return connectionForEntity(causingEntityId);
}

auto operator<<(DataStream& ds, RemoteHitRequest const& hitRequest) -> DataStream& {
  ds << hitRequest.causingEntityId;
  ds << hitRequest.targetEntityId;
  ds << hitRequest.damageRequest;
  return ds;
}

auto operator>>(DataStream& ds, RemoteHitRequest& hitRequest) -> DataStream& {
  ds >> hitRequest.causingEntityId;
  ds >> hitRequest.targetEntityId;
  ds >> hitRequest.damageRequest;
  return ds;
}

auto RemoteDamageRequest::destinationConnection() const -> ConnectionId {
  return connectionForEntity(targetEntityId);
}

auto operator<<(DataStream& ds, RemoteDamageRequest const& damageRequest) -> DataStream& {
  ds << damageRequest.causingEntityId;
  ds << damageRequest.targetEntityId;
  ds << damageRequest.damageRequest;
  return ds;
}

auto operator>>(DataStream& ds, RemoteDamageRequest& damageRequest) -> DataStream& {
  ds >> damageRequest.causingEntityId;
  ds >> damageRequest.targetEntityId;
  ds >> damageRequest.damageRequest;
  return ds;
}

auto operator<<(DataStream& ds, RemoteDamageNotification const& damageNotification) -> DataStream& {
  ds << damageNotification.sourceEntityId;
  ds << damageNotification.damageNotification;
  return ds;
}

auto operator>>(DataStream& ds, RemoteDamageNotification& damageNotification) -> DataStream& {
  ds >> damageNotification.sourceEntityId;
  ds >> damageNotification.damageNotification;
  return ds;
}

DamageManager::DamageManager(World* world, ConnectionId connectionId) : m_world(world), m_connectionId(connectionId) {}

void DamageManager::update(float dt) {
  float const DefaultDamageTimeout = 1.0f;

  auto damageIt = makeSMutableMapIterator(m_recentEntityDamages);
  while (damageIt.hasNext()) {
    auto& events = damageIt.next().second;
    auto eventIt = makeSMutableIterator(events);
    while (eventIt.hasNext()) {
      auto& event = eventIt.next();
      event.timeout -= dt;
      auto entityIdTimeoutGroup = event.timeoutGroup.maybe<EntityId>();
      if (event.timeout <= 0.0f || (entityIdTimeoutGroup && !m_world->entity(*entityIdTimeoutGroup)))
        eventIt.remove();
    }
    if (events.empty())
      damageIt.remove();
  }

  m_world->forAllEntities([&](Ptr<Entity> const& causingEntity) -> void {
    for (auto& damageSource : causingEntity->damageSources()) {
      if (damageSource.trackSourceEntity)
        damageSource.translate(causingEntity->position());

      if (auto poly = damageSource.damageArea.ptr<PolyF>())
        SpatialLogger::logPoly("world", *poly, Color::Orange.toRgba());
      else if (auto line = damageSource.damageArea.ptr<Line2F>())
        SpatialLogger::logLine("world", *line, Color::Orange.toRgba());

      for (auto const& hitResultPair : queryHit(damageSource, causingEntity->entityId())) {
        auto targetEntity = m_world->entity(hitResultPair.first);
        if (!isAuthoritative(causingEntity, targetEntity))
          continue;

        auto& eventList = m_recentEntityDamages[hitResultPair.first];
        // Guard against rapidly repeating damages by either the causing
        // entity id, or optionally the repeat group if specified.
        bool allowDamage = true;
        for (auto const& event : eventList) {
          if (damageSource.damageRepeatGroup) {
            if (event.timeoutGroup == *damageSource.damageRepeatGroup)
              allowDamage = false;
          } else {
            if (event.timeoutGroup == causingEntity->entityId())
              allowDamage = false;
          }
        }
        if (allowDamage) {
          float timeout = damageSource.damageRepeatTimeout.value_or(DefaultDamageTimeout);
          if (damageSource.damageRepeatGroup)
            eventList.append({*damageSource.damageRepeatGroup, timeout});
          else
            eventList.append({causingEntity->entityId(), timeout});

          auto damageRequest = DamageRequest(hitResultPair.second, damageSource.damageType, damageSource.damage,
                                             damageSource.knockbackMomentum(m_world->geometry(), targetEntity->position()),
                                             damageSource.sourceEntityId, damageSource.damageSourceKind, damageSource.statusEffects);
          addHitRequest({.causingEntityId = causingEntity->entityId(), .targetEntityId = targetEntity->entityId(), .damageRequest = damageRequest});

          if (damageSource.damageType != NoDamage)
            addDamageRequest({.causingEntityId = causingEntity->entityId(), .targetEntityId = targetEntity->entityId(), .damageRequest = std::move(damageRequest)});
        }
      }
    }

    for (auto const& damageNotification : causingEntity->selfDamageNotifications())
      addDamageNotification({.sourceEntityId = causingEntity->entityId(), .damageNotification = damageNotification});
  });
}

void DamageManager::pushRemoteHitRequest(RemoteHitRequest const& remoteHitRequest) {
  if (remoteHitRequest.destinationConnection() != m_connectionId)
    throw StarException("RemoteDamageRequest routed to wrong DamageManager");

  if (auto causingEntity = m_world->entity(remoteHitRequest.causingEntityId)) {
    causingEntity->hitOther(remoteHitRequest.targetEntityId, remoteHitRequest.damageRequest);
  }
}

void DamageManager::pushRemoteDamageRequest(RemoteDamageRequest const& remoteDamageRequest) {
  if (remoteDamageRequest.destinationConnection() != m_connectionId)
    throw StarException("RemoteDamageRequest routed to wrong DamageManager");

  if (auto targetEntity = m_world->entity(remoteDamageRequest.targetEntityId)) {
    for (auto& damageNotification : targetEntity->applyDamage(remoteDamageRequest.damageRequest))
      addDamageNotification({.sourceEntityId = remoteDamageRequest.damageRequest.sourceEntityId, .damageNotification = std::move(damageNotification)});
  }
}

void DamageManager::pushRemoteDamageNotification(RemoteDamageNotification remoteDamageNotification) {
  if (auto sourceEntity = m_world->entity(remoteDamageNotification.sourceEntityId)) {
    if (sourceEntity->isMaster()
        && sourceEntity->entityId() != remoteDamageNotification.damageNotification.targetEntityId)
      sourceEntity->damagedOther(remoteDamageNotification.damageNotification);
  }

  m_pendingNotifications.append(std::move(remoteDamageNotification.damageNotification));
}

auto DamageManager::pullRemoteHitRequests() -> List<RemoteHitRequest> {
  return take(m_pendingRemoteHitRequests);
}

auto DamageManager::pullRemoteDamageRequests() -> List<RemoteDamageRequest> {
  return take(m_pendingRemoteDamageRequests);
}

auto DamageManager::pullRemoteDamageNotifications() -> List<RemoteDamageNotification> {
  return take(m_pendingRemoteNotifications);
}

auto DamageManager::pullPendingNotifications() -> List<DamageNotification> {
  return take(m_pendingNotifications);
}

auto DamageManager::queryHit(DamageSource const& source, EntityId causingId) const -> SmallList<std::pair<EntityId, HitType>, 4> {
  SmallList<std::pair<EntityId, HitType>, 4> resultList;
  auto doQueryHit = [&source, &resultList, causingId, this](Ptr<Entity> const& targetEntity) -> void {
    if (targetEntity->entityId() == causingId)
      return;

    if (!source.team.canDamage(targetEntity->getTeam(), targetEntity->entityId() == source.sourceEntityId))
      return;

    if (source.rayCheck) {
      if (auto poly = source.damageArea.ptr<PolyF>()) {
        if (auto sourceEntity = m_world->entity(source.sourceEntityId)) {
          auto overlap = m_world->geometry().rectOverlap(targetEntity->metaBoundBox().translated(targetEntity->position()), poly->boundBox());
          if (!overlap.isEmpty() && m_world->lineTileCollision(overlap.center(), sourceEntity->position()))
            return;
        }
      } else if (auto line = source.damageArea.ptr<Line2F>()) {
        if (auto hitPoly = targetEntity->hitPoly()) {
          if (auto intersection = m_world->geometry().lineIntersectsPolyAt(*line, *hitPoly)) {
            if (m_world->lineTileCollision(line->min(), *intersection))
              return;
          }
        }
      }
    }

    if (auto hitResult = targetEntity->queryHit(source))
      resultList.append({targetEntity->entityId(), *hitResult});

    return;
  };

  if (auto poly = source.damageArea.ptr<PolyF>())
    m_world->forEachEntity(poly->boundBox(), doQueryHit);
  else if (auto line = source.damageArea.ptr<Line2F>())
    m_world->forEachEntityLine(line->min(), line->max(), doQueryHit);

  return resultList;
}

auto DamageManager::isAuthoritative(Ptr<Entity> const& causingEntity, Ptr<Entity> const& targetEntity) -> bool {
  // Damage manager is authoritative if either one of the entities is
  // masterOnly, OR the manager is server-side and both entities are
  // server-side master entities, OR the damage manager is server-side and both
  // entities are different clients, OR if the manager is client-side and the
  // source is client-side master and the target is server-side master, OR if
  // the manager is client-side and the target is client-side master.
  //
  // This means that PvE and EvP are both decided on the player doing the
  // hitting or getting hit, and PvP is decided on the server, except for
  // master-only entities whose interactions are always decided on the machine
  // they are residing on.

  auto causingClient = connectionForEntity(causingEntity->entityId());
  auto targetClient = connectionForEntity(targetEntity->entityId());

  if (causingEntity->masterOnly() || targetEntity->masterOnly())
    return true;
  else if (causingClient == ServerConnectionId && targetClient == ServerConnectionId)
    return m_connectionId == ServerConnectionId;
  else if (causingClient != ServerConnectionId && targetClient != ServerConnectionId && causingClient != targetClient)
    return m_connectionId == ServerConnectionId;
  else if (targetClient == ServerConnectionId)
    return causingClient == m_connectionId;
  else
    return targetClient == m_connectionId;
}

void DamageManager::addHitRequest(RemoteHitRequest const& remoteHitRequest) {
  if (remoteHitRequest.destinationConnection() == m_connectionId)
    pushRemoteHitRequest(remoteHitRequest);
  else
    m_pendingRemoteHitRequests.append(remoteHitRequest);
}

void DamageManager::addDamageRequest(RemoteDamageRequest remoteDamageRequest) {
  if (remoteDamageRequest.destinationConnection() == m_connectionId)
    pushRemoteDamageRequest(std::move(remoteDamageRequest));
  else
    m_pendingRemoteDamageRequests.append(std::move(remoteDamageRequest));
}

void DamageManager::addDamageNotification(RemoteDamageNotification remoteDamageNotification) {
  pushRemoteDamageNotification(remoteDamageNotification);
  m_pendingRemoteNotifications.append(std::move(remoteDamageNotification));
}

}// namespace Star
