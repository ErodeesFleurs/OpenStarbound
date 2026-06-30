#pragma once

#include "StarVersioningDatabase.hpp"
#include "StarEntity.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarItemDatabase.hpp"

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
class VehicleDatabase;
using VehicleDatabaseConstPtr = SharedPtr<VehicleDatabase const>;

class EntityFactory;
using EntityFactoryPtr = SharedPtr<EntityFactory>;
using EntityFactoryConstPtr = SharedPtr<EntityFactory const>;

struct EntityFactoryExceptionTag { static constexpr char const* typeName = "EntityFactoryException"; };
using EntityFactoryException = TypedException<StarException, EntityFactoryExceptionTag>;

class EntityFactory {
public:
  EntityFactory(
      AssetsConstPtr assets,
      PlayerFactoryConstPtr playerFactory,
      MonsterDatabaseConstPtr monsterDatabase,
      ObjectDatabaseConstPtr objectDatabase,
      ProjectileDatabaseConstPtr projectileDatabase,
      NpcDatabaseConstPtr npcDatabase,
      VehicleDatabaseConstPtr vehicleDatabase,
      VersioningDatabaseConstPtr versioningDatabase,
      ItemDatabaseConstPtr itemDatabase,
      ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] EntityPtr create(String const& entityName, Json const& extraParams = {}) const;

  [[nodiscard]] ByteArray netStoreEntity(EntityPtr const& entity, NetCompatibilityRules rules = {}) const;
  [[nodiscard]] EntityPtr netLoadEntity(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

  [[nodiscard]] Json diskStoreEntity(EntityPtr const& entity) const;
  [[nodiscard]] EntityPtr diskLoadEntity(EntityType type, Json const& diskStore) const;

  [[nodiscard]] Json loadVersionedJson(VersionedJson const& versionedJson, EntityType expectedType) const;
  [[nodiscard]] VersionedJson storeVersionedJson(EntityType type, Json const& store) const;

  // Wraps the normal Json based Entity store / load in a VersionedJson, and
  // uses sripts in the VersionedingDatabase to bring the version of the store
  // forward to match the current version.
  [[nodiscard]] EntityPtr loadVersionedEntity(VersionedJson const& versionedJson) const;
  [[nodiscard]] VersionedJson storeVersionedEntity(EntityPtr const& entityPtr) const;

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
  AssetsConstPtr m_assets;
  ItemDatabaseConstPtr m_itemDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
};

}
