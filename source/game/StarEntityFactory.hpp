#pragma once

#include "StarVersioningDatabase.hpp"
#include "StarEntity.hpp"

namespace Star {

class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;
class PlayerFactory;
using PlayerFactoryConstPtr = SharedPtr<PlayerFactory const>;
class MonsterDatabase;
using MonsterDatabaseConstPtr = SharedPtr<MonsterDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class ProjectileDatabase;
using ProjectileDatabaseConstPtr = SharedPtr<ProjectileDatabase const>;
class NpcDatabase;
using NpcDatabaseConstPtr = SharedPtr<NpcDatabase const>;

class EntityFactory;
using EntityFactoryPtr = SharedPtr<EntityFactory>;
using EntityFactoryConstPtr = SharedPtr<EntityFactory const>;

struct EntityFactoryExceptionTag { static constexpr char const* typeName = "EntityFactoryException"; };
using EntityFactoryException = TypedException<StarException, EntityFactoryExceptionTag>;

class EntityFactory {
public:
  EntityFactory();

  ByteArray netStoreEntity(EntityPtr const& entity, NetCompatibilityRules rules = {}) const;
  EntityPtr netLoadEntity(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

  Json diskStoreEntity(EntityPtr const& entity) const;
  EntityPtr diskLoadEntity(EntityType type, Json const& diskStore) const;

  Json loadVersionedJson(VersionedJson const& versionedJson, EntityType expectedType) const;
  VersionedJson storeVersionedJson(EntityType type, Json const& store) const;

  // Wraps the normal Json based Entity store / load in a VersionedJson, and
  // uses sripts in the VersionedingDatabase to bring the version of the store
  // forward to match the current version.
  EntityPtr loadVersionedEntity(VersionedJson const& versionedJson) const;
  VersionedJson storeVersionedEntity(EntityPtr const& entityPtr) const;

private:
  static EnumMap<EntityType> const EntityStorageIdentifiers;

  mutable RecursiveMutex m_mutex;

  PlayerFactoryConstPtr m_playerFactory;
  MonsterDatabaseConstPtr m_monsterDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  ProjectileDatabaseConstPtr m_projectileDatabase;
  NpcDatabaseConstPtr m_npcDatabase;
  VehicleDatabaseConstPtr m_vehicleDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;
};

}
