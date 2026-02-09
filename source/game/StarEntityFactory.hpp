#pragma once

#include "StarConfig.hpp"
#include "StarEntity.hpp"
#include "StarException.hpp"
#include "StarRoot.hpp"
#include "StarVersioningDatabase.hpp"

namespace Star {

using EntityFactoryException = ExceptionDerived<"EntityFactoryException">;

class EntityFactory {
public:
  EntityFactory();

  auto netStoreEntity(Ptr<Entity> const& entity, NetCompatibilityRules rules = {}) const -> ByteArray;
  auto netLoadEntity(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules = {}) const -> Ptr<Entity>;

  auto diskStoreEntity(Ptr<Entity> const& entity) const -> Json;
  auto diskLoadEntity(EntityType type, Json const& diskStore) const -> Ptr<Entity>;

  auto loadVersionedJson(VersionedJson const& versionedJson, EntityType expectedType) const -> Json;
  auto storeVersionedJson(EntityType type, Json const& store) const -> VersionedJson;

  // Wraps the normal Json based Entity store / load in a VersionedJson, and
  // uses sripts in the VersionedingDatabase to bring the version of the store
  // forward to match the current version.
  auto loadVersionedEntity(VersionedJson const& versionedJson) const -> Ptr<Entity>;
  auto storeVersionedEntity(Ptr<Entity> const& entityPtr) const -> VersionedJson;

private:
  static EnumMap<EntityType> const EntityStorageIdentifiers;

  mutable RecursiveMutex m_mutex;

  ConstPtr<PlayerFactory> m_playerFactory;
  ConstPtr<MonsterDatabase> m_monsterDatabase;
  ConstPtr<ObjectDatabase> m_objectDatabase;
  ConstPtr<ProjectileDatabase> m_projectileDatabase;
  ConstPtr<NpcDatabase> m_npcDatabase;
  ConstPtr<VehicleDatabase> m_vehicleDatabase;
  ConstPtr<VersioningDatabase> m_versioningDatabase;
};

}// namespace Star
