#include "StarEntityMap.hpp"

#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarInteractiveEntity.hpp"
#include "StarLogging.hpp"
#include "StarTileEntity.hpp"

import std;

namespace Star {

constexpr std::float_t EntityMapSpatialHashSectorSize = 16.0f;
constexpr std::int32_t EntityMap::MaximumEntityBoundBox = 10000;

EntityMap::EntityMap(Vec2U const& worldSize, EntityId beginIdSpace, EntityId endIdSpace)
    : m_geometry(worldSize),
      m_spatialMap(EntityMapSpatialHashSectorSize),
      m_nextId(beginIdSpace),
      m_beginIdSpace(beginIdSpace),
      m_endIdSpace(endIdSpace) {}

auto EntityMap::reserveEntityId() -> EntityId {
  if (m_spatialMap.size() >= (std::size_t)(m_endIdSpace - m_beginIdSpace))
    throw EntityMapException("No more entity id space in EntityMap::reserveEntityId");

  EntityId id = m_nextId;
  while (m_spatialMap.contains(id))
    id = cycleIncrement(id, m_beginIdSpace, m_endIdSpace);
  m_nextId = cycleIncrement(id, m_beginIdSpace, m_endIdSpace);

  return id;
}

auto EntityMap::maybeReserveEntityId(EntityId entityId) -> std::optional<EntityId> {
  if (m_spatialMap.size() >= (std::size_t)(m_endIdSpace - m_beginIdSpace))
    throw EntityMapException("No more entity id space in EntityMap::reserveEntityId");

  if (entityId == NullEntityId || m_spatialMap.contains(entityId))
    return {};
  else
    return entityId;
}

auto EntityMap::reserveEntityId(EntityId entityId) -> EntityId {
  if (entityId == NullEntityId)
    return reserveEntityId();
  if (auto reserved = maybeReserveEntityId(entityId))
    return *reserved;

  m_nextId = entityId;
  return reserveEntityId();
}

void EntityMap::addEntity(Ptr<Entity> entity) {
  auto position = entity->position();
  auto boundBox = entity->metaBoundBox();
  auto entityId = entity->entityId();
  auto uniqueId = entity->uniqueId();

  if (m_spatialMap.contains(entityId))
    throw EntityMapException::format("Duplicate entity id '{}' in EntityMap::addEntity", entityId);

  if (boundBox.isNegative() || boundBox.width() > MaximumEntityBoundBox || boundBox.height() > MaximumEntityBoundBox) {
    throw EntityMapException::format("Entity id: {} type: {} bound box is negative or beyond the maximum entity bound box size in EntityMap::addEntity",
                                     entity->entityId(), (int)entity->entityType());
  }

  if (entityId == NullEntityId)
    throw EntityMapException::format("Null entity id in EntityMap::addEntity");

  if (uniqueId && m_uniqueMap.hasLeftValue(*uniqueId))
    throw EntityMapException::format("Duplicate entity unique id ({}) on entity id ({}) in EntityMap::addEntity", *uniqueId, entityId);

  m_spatialMap.set(entityId, m_geometry.splitRect(boundBox, position), std::move(entity));
  if (uniqueId)
    m_uniqueMap.add(*uniqueId, entityId);
}

auto EntityMap::removeEntity(EntityId entityId) -> Ptr<Entity> {
  if (auto entity = m_spatialMap.remove(entityId)) {
    m_uniqueMap.removeRight(entityId);
    return std::move(*entity);
  }
  return {};
}

auto EntityMap::size() const -> std::size_t {
  return m_spatialMap.size();
}

auto EntityMap::entityIds() const -> List<EntityId> {
  return m_spatialMap.keys();
}

void EntityMap::updateAllEntities(EntityCallback const& callback, std::function<bool(Ptr<Entity> const&, Ptr<Entity> const&)> sortOrder) {
  auto updateEntityInfo = [&](SpatialMap::Entry const& entry) -> void {
    auto const& entity = entry.value;

    auto position = entity->position();
    auto boundBox = entity->metaBoundBox();

    if (boundBox.isNegative() || boundBox.width() > MaximumEntityBoundBox || boundBox.height() > MaximumEntityBoundBox) {
      throw EntityMapException::format("Entity id: {} type: {} bound box is negative or beyond the maximum entity bound box size in EntityMap::addEntity",
                                       entity->entityId(), (int)entity->entityType());
    }

    auto entityId = entity->entityId();
    if (entityId == NullEntityId)
      throw EntityMapException::format("Null entity id in EntityMap::setEntityInfo");

    auto rects = m_geometry.splitRect(boundBox, position);
    if (!containersEqual(rects, entry.rects))
      m_spatialMap.set(entityId, rects);

    auto uniqueId = entity->uniqueId();
    if (uniqueId) {
      if (auto existingEntityId = m_uniqueMap.maybeRight(*uniqueId)) {
        if (entityId != *existingEntityId)
          throw EntityMapException::format("Duplicate entity unique id on entity ids ({}) and ({})", *existingEntityId, entityId);
      } else {
        m_uniqueMap.removeRight(entityId);
        m_uniqueMap.add(*uniqueId, entityId);
      }
    } else {
      m_uniqueMap.removeRight(entityId);
    }
  };

  // Even if there is no sort order, we still copy pointers to a temporary
  // list, so that it is safe to call addEntity from the callback.
  m_entrySortBuffer.clear();
  for (auto const& entry : m_spatialMap.entries())
    m_entrySortBuffer.append(&entry.second);

  if (sortOrder) {
    m_entrySortBuffer.sort([&sortOrder](auto a, auto b) -> auto {
      return sortOrder(a->value, b->value);
    });
  }

  for (auto entry : m_entrySortBuffer) {
    if (callback)
      callback(entry->value);
    updateEntityInfo(*entry);
  }
}

auto EntityMap::uniqueEntityId(String const& uniqueId) const -> EntityId {
  return m_uniqueMap.maybeRight(uniqueId).value_or(NullEntityId);
}

auto EntityMap::entity(EntityId entityId) const -> Ptr<Entity> {
  auto entity = m_spatialMap.value(entityId);
  return entity;
}

auto EntityMap::uniqueEntity(String const& uniqueId) const -> Ptr<Entity> {
  return entity(uniqueEntityId(uniqueId));
}

auto EntityMap::entityQuery(RectF const& boundBox, EntityFilter const& filter) const -> List<Ptr<Entity>> {
  List<Ptr<Entity>> values;
  forEachEntity(boundBox, [&](Ptr<Entity> const& entity) -> void {
    if (!filter || filter(entity))
      values.append(entity);
  });
  return values;
}

auto EntityMap::entitiesAt(Vec2F const& pos, EntityFilter const& filter) const -> List<Ptr<Entity>> {
  auto entityList = entityQuery(RectF::withCenter(pos, {0, 0}), filter);

  sortByComputedValue(entityList, [&](Ptr<Entity> const& entity) -> float {
    return vmagSquared(entity->position() - pos);
  });
  return entityList;
}

auto EntityMap::entitiesAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> const& filter) const -> List<Ptr<TileEntity>> {
  List<Ptr<TileEntity>> values;
  forEachEntityAtTile(pos, [&](Ptr<TileEntity> const& entity) -> void {
    if (!filter || filter(entity))
      values.append(entity);
  });
  return values;
}

void EntityMap::forEachEntity(RectF const& boundBox, EntityCallback const& callback) const {
  m_spatialMap.forEach(m_geometry.splitRect(boundBox), callback);
}

void EntityMap::forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback const& callback) const {
  return m_spatialMap.forEach(m_geometry.splitRect(RectF::boundBoxOf(begin, end)), [&](Ptr<Entity> const& entity) -> void {
    if (m_geometry.lineIntersectsRect({begin, end}, entity->metaBoundBox().translated(entity->position())))
      callback(entity);
  });
}

void EntityMap::forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> const& callback) const {
  RectF rect(Vec2F(pos[0], pos[1]), Vec2F(pos[0] + 1, pos[1] + 1));
  forEachEntity(rect, [&](Ptr<Entity> const& entity) -> void {
    if (auto tileEntity = as<TileEntity>(entity)) {
      for (Vec2I space : tileEntity->spaces()) {
        if (m_geometry.equal(pos, space + tileEntity->tilePosition()))
          callback(tileEntity);
      }
    }
  });
}

void EntityMap::forAllEntities(EntityCallback const& callback, std::function<bool(Ptr<Entity> const&, Ptr<Entity> const&)> sortOrder) const {
  // Even if there is no sort order, we still copy pointers to a temporary
  // list, so that it is safe to call addEntity from the callback.
  List<Ptr<Entity> const*> allEntities;
  allEntities.reserve(m_spatialMap.size());
  for (auto const& entry : m_spatialMap.entries())
    allEntities.append(&entry.second.value);

  if (sortOrder) {
    allEntities.sort([&sortOrder](Ptr<Entity> const* a, Ptr<Entity> const* b) -> bool {
      return sortOrder(*a, *b);
    });
  }

  for (auto ptr : allEntities) {
    auto& entity = *ptr;
    try {
      callback(entity);
    } catch (...) {
      Logger::error("[EntityMap] Exception caught running forAllEntities callback for {} entity {} (named \"{}\")",
                    EntityTypeNames.getRight(entity->entityType()),
                    entity->entityId(),
                    entity->name());
      throw;
    }
  }
}

auto EntityMap::findEntity(RectF const& boundBox, EntityFilter const& filter) const -> Ptr<Entity> {
  Ptr<Entity> res;
  forEachEntity(boundBox, [&filter, &res](Ptr<Entity> const& entity) -> void {
    if (res)
      return;
    if (filter(entity))
      res = entity;
  });
  return res;
}

auto EntityMap::findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter const& filter) const -> Ptr<Entity> {
  return findEntity(RectF::boundBoxOf(begin, end), [&](Ptr<Entity> const& entity) -> bool {
    if (m_geometry.lineIntersectsRect({begin, end}, entity->metaBoundBox().translated(entity->position()))) {
      if (filter(entity))
        return true;
    }
    return false;
  });
}

auto EntityMap::findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> const& filter) const -> Ptr<Entity> {
  RectF rect(Vec2F(pos[0], pos[1]), Vec2F(pos[0] + 1, pos[1] + 1));
  return findEntity(rect, [&](Ptr<Entity> const& entity) -> bool {
    if (auto tileEntity = as<TileEntity>(entity)) {
      for (Vec2I space : tileEntity->spaces()) {
        if (m_geometry.equal(pos, space + tileEntity->tilePosition())) {
          if (filter(tileEntity))
            return true;
        }
      }
    }
    return false;
  });
}

auto EntityMap::entityLineQuery(Vec2F const& begin, Vec2F const& end, EntityFilter const& filter) const -> List<Ptr<Entity>> {
  List<Ptr<Entity>> values;
  forEachEntityLine(begin, end, [&](Ptr<Entity> const& entity) -> void {
    if (!filter || filter(entity))
      values.append(entity);
  });
  return values;
}

auto EntityMap::closestEntity(Vec2F const& center, float radius, EntityFilter const& filter) const -> Ptr<Entity> {
  Ptr<Entity> closest;
  float distSquared = square(radius);
  RectF boundBox(center[0] - radius, center[1] - radius, center[0] + radius, center[1] + radius);

  m_spatialMap.forEach(m_geometry.splitRect(boundBox), [&](Ptr<Entity> const& entity) -> void {
    Vec2F pos = entity->position();
    float thisDistSquared = m_geometry.diff(center, pos).magnitudeSquared();
    if (distSquared > thisDistSquared) {
      if (!filter || filter(entity)) {
        distSquared = thisDistSquared;
        closest = entity;
      }
    }
  });

  return closest;
}

auto EntityMap::interactiveEntityNear(Vec2F const& pos, float maxRadius) const -> Ptr<InteractiveEntity> {
  auto rect = RectF::withCenter(pos, Vec2F::filled(maxRadius));
  Ptr<InteractiveEntity> interactiveEntity;
  double bestDistance = maxRadius + 100;
  double bestCenterDistance = maxRadius + 100;
  m_spatialMap.forEach(m_geometry.splitRect(rect), [&](Ptr<Entity> const& entity) -> void {
    if (auto ie = as<InteractiveEntity>(entity)) {
      if (ie->isInteractive()) {
        if (auto tileEntity = as<TileEntity>(entity)) {
          for (Vec2I space : tileEntity->interactiveSpaces()) {
            auto dist = m_geometry.diff(pos, centerOfTile(space + tileEntity->tilePosition())).magnitude();
            auto centerDist = m_geometry.diff(tileEntity->metaBoundBox().center() + tileEntity->position(), pos).magnitude();
            if ((dist < bestDistance) || ((dist == bestDistance) && (centerDist < bestCenterDistance))) {
              interactiveEntity = ie;
              bestDistance = dist;
              bestCenterDistance = centerDist;
            }
          }
        } else {
          auto box = ie->interactiveBoundBox().translated(entity->position());
          auto dist = m_geometry.diffToNearestCoordInBox(box, pos).magnitude();
          auto centerDist = m_geometry.diff(box.center(), pos).magnitude();
          if ((dist < bestDistance) || ((dist == bestDistance) && (centerDist < bestCenterDistance))) {
            interactiveEntity = ie;
            bestDistance = dist;
            bestCenterDistance = centerDist;
          }
        }
      }
    }
  });
  if (bestDistance <= maxRadius)
    return interactiveEntity;
  return {};
}

auto EntityMap::tileIsOccupied(Vec2I const& pos, bool includeEphemeral) const -> bool {
  RectF rect(Vec2F(pos[0], pos[1]), Vec2F(pos[0] + 1, pos[1] + 1));
  return (bool)findEntity(rect, [&](Ptr<Entity> const& entity) -> bool {
    if (auto tileEntity = as<TileEntity>(entity)) {
      if (includeEphemeral || !tileEntity->ephemeral()) {
        for (Vec2I space : tileEntity->spaces()) {
          if (m_geometry.equal(pos, space + tileEntity->tilePosition())) {
            return true;
          }
        }
      }
    }
    return false;
  });
}

auto EntityMap::spaceIsOccupied(RectF const& rect, bool includesEphemeral) const -> bool {
  for (auto const& entity : entityQuery(rect)) {
    if (!includesEphemeral && entity->ephemeral())
      continue;

    for (RectF const& c : m_geometry.splitRect(entity->collisionArea(), entity->position())) {
      if (!c.isNull() && rect.intersects(c))
        return true;
    }
  }
  return false;
}

}// namespace Star
