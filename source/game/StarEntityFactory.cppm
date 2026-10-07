module;
#include "StarIdMap.hpp"
#include "StarThread.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;

namespace Star {

STAR_STRUCT(VersionedJson);
STAR_CLASS(VehicleDatabase);
STAR_CLASS(VersioningDatabase);
STAR_CLASS(PlayerFactory);
STAR_CLASS(MonsterDatabase);
STAR_CLASS(ObjectDatabase);
STAR_CLASS(ProjectileDatabase);
STAR_CLASS(NpcDatabase);

STAR_CLASS(EntityFactory);

struct EntityFactoryExceptionTag {
  static constexpr char const* name() { return "EntityFactoryException"; }
};
using EntityFactoryException = StarError<EntityFactoryExceptionTag, StarException>;

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

export module star.entity_factory;

export namespace Star {
  using ::Star::VersionedJson;
  using ::Star::VersionedJsonPtr;
  using ::Star::VersionedJsonConstPtr;
  using ::Star::VersionedJsonWeakPtr;
  using ::Star::VersionedJsonConstWeakPtr;
  using ::Star::VersionedJsonUPtr;
  using ::Star::VersionedJsonConstUPtr;
  using ::Star::VehicleDatabase;
  using ::Star::VehicleDatabasePtr;
  using ::Star::VehicleDatabaseConstPtr;
  using ::Star::VehicleDatabaseWeakPtr;
  using ::Star::VehicleDatabaseConstWeakPtr;
  using ::Star::VehicleDatabaseUPtr;
  using ::Star::VehicleDatabaseConstUPtr;
  using ::Star::VersioningDatabase;
  using ::Star::VersioningDatabasePtr;
  using ::Star::VersioningDatabaseConstPtr;
  using ::Star::VersioningDatabaseWeakPtr;
  using ::Star::VersioningDatabaseConstWeakPtr;
  using ::Star::VersioningDatabaseUPtr;
  using ::Star::VersioningDatabaseConstUPtr;
  using ::Star::PlayerFactory;
  using ::Star::PlayerFactoryPtr;
  using ::Star::PlayerFactoryConstPtr;
  using ::Star::PlayerFactoryWeakPtr;
  using ::Star::PlayerFactoryConstWeakPtr;
  using ::Star::PlayerFactoryUPtr;
  using ::Star::PlayerFactoryConstUPtr;
  using ::Star::MonsterDatabase;
  using ::Star::MonsterDatabasePtr;
  using ::Star::MonsterDatabaseConstPtr;
  using ::Star::MonsterDatabaseWeakPtr;
  using ::Star::MonsterDatabaseConstWeakPtr;
  using ::Star::MonsterDatabaseUPtr;
  using ::Star::MonsterDatabaseConstUPtr;
  using ::Star::ObjectDatabase;
  using ::Star::ObjectDatabasePtr;
  using ::Star::ObjectDatabaseConstPtr;
  using ::Star::ObjectDatabaseWeakPtr;
  using ::Star::ObjectDatabaseConstWeakPtr;
  using ::Star::ObjectDatabaseUPtr;
  using ::Star::ObjectDatabaseConstUPtr;
  using ::Star::ProjectileDatabase;
  using ::Star::ProjectileDatabasePtr;
  using ::Star::ProjectileDatabaseConstPtr;
  using ::Star::ProjectileDatabaseWeakPtr;
  using ::Star::ProjectileDatabaseConstWeakPtr;
  using ::Star::ProjectileDatabaseUPtr;
  using ::Star::ProjectileDatabaseConstUPtr;
  using ::Star::NpcDatabase;
  using ::Star::NpcDatabasePtr;
  using ::Star::NpcDatabaseConstPtr;
  using ::Star::NpcDatabaseWeakPtr;
  using ::Star::NpcDatabaseConstWeakPtr;
  using ::Star::NpcDatabaseUPtr;
  using ::Star::NpcDatabaseConstUPtr;
  using ::Star::EntityFactory;
  using ::Star::EntityFactoryPtr;
  using ::Star::EntityFactoryConstPtr;
  using ::Star::EntityFactoryWeakPtr;
  using ::Star::EntityFactoryConstWeakPtr;
  using ::Star::EntityFactoryUPtr;
  using ::Star::EntityFactoryConstUPtr;
  using ::Star::EntityFactoryExceptionTag;
  using ::Star::EntityFactoryException;
}
