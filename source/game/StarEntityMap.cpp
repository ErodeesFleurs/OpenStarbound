import std;
import star.optional;
import star.string;
import star.logging;
import star.format;
import star.json;
import star.id_map;
import star.rect;
import star.map;
import star.set;
import star.spatial_hash_2d;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.list;
import star.ordered_map;
import star.rpc_promise;
import star.lua;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.lua_root;
import star.force_regions;
import star.world;
import star.root;
import star.physics_entity;
import star.mobile_entity;
import star.scripted_entity;
import star.status_effect_entity;
import star.movement_controller;
import star.animation;
import star.particle;
import star.lua_components;
import star.effect_emitter;
import star.projectile;

import star.entity_map;

import star.math_common;
import star.algorithm;
namespace Star {

float const EntityMapSpatialHashSectorSize = 16.0f;
int const EntityMap::MaximumEntityBoundBox = 10000;

EntityMap::EntityMap(Vec2U const& worldSize, EntityId beginIdSpace, EntityId endIdSpace)
  : m_geometry(worldSize),
    m_spatialMap(EntityMapSpatialHashSectorSize),
    m_nextId(beginIdSpace),
    m_beginIdSpace(beginIdSpace),
    m_endIdSpace(endIdSpace) {}

EntityId EntityMap::reserveEntityId() {
  if (m_spatialMap.size() >= (std::size_t)(m_endIdSpace - m_beginIdSpace))
    throw EntityMapException("No more entity id space in EntityMap::reserveEntityId");

  EntityId id = m_nextId;
  while (m_spatialMap.contains(id))
    id = cycleIncrement(id, m_beginIdSpace, m_endIdSpace);
  m_nextId = cycleIncrement(id, m_beginIdSpace, m_endIdSpace);

  return id;
}

std::optional<EntityId> EntityMap::maybeReserveEntityId(EntityId entityId) {
  if (m_spatialMap.size() >= (std::size_t)(m_endIdSpace - m_beginIdSpace))
    throw EntityMapException("No more entity id space in EntityMap::reserveEntityId");

  if (entityId == NullEntityId || m_spatialMap.contains(entityId))
    return {};
  else
    return entityId;
}

EntityId EntityMap::reserveEntityId(EntityId entityId) {
  if (entityId == NullEntityId)
    return reserveEntityId();
  if (auto reserved = maybeReserveEntityId(entityId))
    return *reserved;

  m_nextId = entityId;
  return reserveEntityId();
}


void EntityMap::addEntity(std::shared_ptr<Entity> entity) {
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

std::shared_ptr<Entity> EntityMap::removeEntity(EntityId entityId) {
  if (auto entity = m_spatialMap.remove(entityId)) {
    m_uniqueMap.removeRight(entityId);
    return takeOptional(entity);
  }
  return {};
}

std::size_t EntityMap::size() const {
  return m_spatialMap.size();
}

List<EntityId> EntityMap::entityIds() const {
  return m_spatialMap.keys();
}

void EntityMap::updateAllEntities(EntityCallback const& callback, std::function<bool(std::shared_ptr<Entity> const&, std::shared_ptr<Entity> const&)> sortOrder) {
  auto updateEntityInfo = [&](SpatialMap::Entry const& entry) {
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
    if (!std::ranges::equal(rects, entry.rects))
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
    m_entrySortBuffer.sort([&sortOrder](auto a, auto b) {
        return sortOrder(a->value, b->value);
      });
  }

  for (auto entry : m_entrySortBuffer) {
    if (callback)
      callback(entry->value);
    updateEntityInfo(*entry);
  }
}

EntityId EntityMap::uniqueEntityId(String const& uniqueId) const {
  return m_uniqueMap.maybeRight(uniqueId).value_or(NullEntityId);
}

std::shared_ptr<Entity> EntityMap::entity(EntityId entityId) const {
  auto entity = m_spatialMap.value(entityId);
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((!entity || entity->entityId() == entityId));
  }
  return entity;
}

std::shared_ptr<Entity> EntityMap::uniqueEntity(String const& uniqueId) const {
  return entity(uniqueEntityId(uniqueId));
}

List<std::shared_ptr<Entity>> EntityMap::entityQuery(RectF const& boundBox, EntityFilter const& filter) const {
  List<std::shared_ptr<Entity>> values;
  forEachEntity(boundBox, [&](std::shared_ptr<Entity> const& entity) {
      if (!filter || filter(entity))
        values.append(entity);
    });
  return values;
}

List<std::shared_ptr<Entity>> EntityMap::entitiesAt(Vec2F const& pos, EntityFilter const& filter) const {
  auto entityList = entityQuery(RectF::withCenter(pos, {0, 0}), filter);

  sortByComputedValue(entityList, [&](std::shared_ptr<Entity> const& entity) -> float {
      return vmagSquared(entity->position() - pos);
    });
  return entityList;
}

List<std::shared_ptr<TileEntity>> EntityMap::entitiesAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> const& filter) const {
  List<std::shared_ptr<TileEntity>> values;
  forEachEntityAtTile(pos, [&](std::shared_ptr<TileEntity> const& entity) {
      if (!filter || filter(entity))
        values.append(entity);
    });
  return values;
}

void EntityMap::forEachEntity(RectF const& boundBox, EntityCallback const& callback) const {
  m_spatialMap.forEach(m_geometry.splitRect(boundBox), callback);
}

void EntityMap::forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback const& callback) const {
  return m_spatialMap.forEach(m_geometry.splitRect(RectF::boundBoxOf(begin, end)), [&](std::shared_ptr<Entity> const& entity) {
      if (m_geometry.lineIntersectsRect({begin, end}, entity->metaBoundBox().translated(entity->position())))
        callback(entity);
    });
}

void EntityMap::forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> const& callback) const {
  RectF rect(Vec2F(pos[0], pos[1]), Vec2F(pos[0] + 1, pos[1] + 1));
  forEachEntity(rect, [&](std::shared_ptr<Entity> const& entity) {
      if (auto tileEntity = as<TileEntity>(entity)) {
        for (Vec2I space : tileEntity->spaces()) {
          if (m_geometry.equal(pos, space + tileEntity->tilePosition()))
            callback(tileEntity);
        }
      }
    });
}

void EntityMap::forAllEntities(EntityCallback const& callback, std::function<bool(std::shared_ptr<Entity> const&, std::shared_ptr<Entity> const&)> sortOrder) const {
  // Even if there is no sort order, we still copy pointers to a temporary
  // list, so that it is safe to call addEntity from the callback.
  List<std::shared_ptr<Entity> const*> allEntities;
  allEntities.reserve(m_spatialMap.size());
  for (auto const& entry : m_spatialMap.entries())
    allEntities.append(&entry.second.value);

  if (sortOrder) {
    allEntities.sort([&sortOrder](std::shared_ptr<Entity> const* a, std::shared_ptr<Entity> const* b) {
        return sortOrder(*a, *b);
      });
  }

  for (auto ptr : allEntities) {
    auto& entity = *ptr;
    try {
      callback(entity);
    } catch (...) {
      entity->root().logger().error("[EntityMap] Exception caught running forAllEntities callback for {} entity {} (named \"{}\")",
        EntityTypeNames.getRight(entity->entityType()),
        entity->entityId(),
        entity->name()
      );
      throw;
    }
  }
}

std::shared_ptr<Entity> EntityMap::findEntity(RectF const& boundBox, EntityFilter const& filter) const {
  std::shared_ptr<Entity> res;
  forEachEntity(boundBox, [&filter, &res](std::shared_ptr<Entity> const& entity) {
      if (res)
        return;
      if (filter(entity))
        res = entity;
    });
  return res;
}

std::shared_ptr<Entity> EntityMap::findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter const& filter) const {
  return findEntity(RectF::boundBoxOf(begin, end), [&](std::shared_ptr<Entity> const& entity) {
      if (m_geometry.lineIntersectsRect({begin, end}, entity->metaBoundBox().translated(entity->position()))) {
        if (filter(entity))
          return true;
      }
      return false;
    });
}

std::shared_ptr<Entity> EntityMap::findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> const& filter) const {
  RectF rect(Vec2F(pos[0], pos[1]), Vec2F(pos[0] + 1, pos[1] + 1));
  return findEntity(rect, [&](std::shared_ptr<Entity> const& entity) {
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

List<std::shared_ptr<Entity>> EntityMap::entityLineQuery(Vec2F const& begin, Vec2F const& end, EntityFilter const& filter) const {
  List<std::shared_ptr<Entity>> values;
  forEachEntityLine(begin, end, [&](std::shared_ptr<Entity> const& entity) {
      if (!filter || filter(entity))
        values.append(entity);
    });
  return values;
}

std::shared_ptr<Entity> EntityMap::closestEntity(Vec2F const& center, float radius, EntityFilter const& filter) const {
  std::shared_ptr<Entity> closest;
  float distSquared = square(radius);
  RectF boundBox(center[0] - radius, center[1] - radius, center[0] + radius, center[1] + radius);

  m_spatialMap.forEach(m_geometry.splitRect(boundBox), [&](std::shared_ptr<Entity> const& entity) {
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

std::shared_ptr<InteractiveEntity> EntityMap::interactiveEntityNear(Vec2F const& pos, float maxRadius) const {
  auto rect = RectF::withCenter(pos, Vec2F::filled(maxRadius));
  std::shared_ptr<InteractiveEntity> interactiveEntity;
  double bestDistance = maxRadius + 100;
  double bestCenterDistance = maxRadius + 100;
  m_spatialMap.forEach(m_geometry.splitRect(rect), [&](std::shared_ptr<Entity> const& entity) {
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

bool EntityMap::tileIsOccupied(Vec2I const& pos, bool includeEphemeral) const {
  RectF rect(Vec2F(pos[0], pos[1]), Vec2F(pos[0] + 1, pos[1] + 1));
  return (bool)findEntity(rect, [&](std::shared_ptr<Entity> const& entity) {
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

bool EntityMap::spaceIsOccupied(RectF const& rect, bool includesEphemeral) const {
  for (auto const& entity : entityQuery(rect, {})) {
    if (!includesEphemeral && entity->ephemeral())
      continue;

    for (RectF const& c : m_geometry.splitRect(entity->collisionArea(), entity->position())) {
      if (!c.isNull() && rect.intersects(c))
        return true;
    }
  }
  return false;
}

}
