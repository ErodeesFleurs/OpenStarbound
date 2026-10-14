module;
import std;
import star.format;
import star.json;
import star.id_map;
import star.thread;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;

import star.net_compatibility;
import star.byte_array;
namespace Star {

struct VersionedJson;
using VersionedJsonPtr = std::shared_ptr<VersionedJson>;
using VersionedJsonConstPtr = std::shared_ptr<const VersionedJson>;
using VersionedJsonWeakPtr = std::weak_ptr<VersionedJson>;
using VersionedJsonConstWeakPtr = std::weak_ptr<const VersionedJson>;
using VersionedJsonUPtr = std::unique_ptr<VersionedJson>;
using VersionedJsonConstUPtr = std::unique_ptr<const VersionedJson>;
class VehicleDatabase;
using VehicleDatabasePtr = std::shared_ptr<VehicleDatabase>;
using VehicleDatabaseConstPtr = std::shared_ptr<const VehicleDatabase>;
using VehicleDatabaseWeakPtr = std::weak_ptr<VehicleDatabase>;
using VehicleDatabaseConstWeakPtr = std::weak_ptr<const VehicleDatabase>;
using VehicleDatabaseUPtr = std::unique_ptr<VehicleDatabase>;
using VehicleDatabaseConstUPtr = std::unique_ptr<const VehicleDatabase>;
class VersioningDatabase;
using VersioningDatabasePtr = std::shared_ptr<VersioningDatabase>;
using VersioningDatabaseConstPtr = std::shared_ptr<const VersioningDatabase>;
using VersioningDatabaseWeakPtr = std::weak_ptr<VersioningDatabase>;
using VersioningDatabaseConstWeakPtr = std::weak_ptr<const VersioningDatabase>;
using VersioningDatabaseUPtr = std::unique_ptr<VersioningDatabase>;
using VersioningDatabaseConstUPtr = std::unique_ptr<const VersioningDatabase>;
class PlayerFactory;
using PlayerFactoryPtr = std::shared_ptr<PlayerFactory>;
using PlayerFactoryConstPtr = std::shared_ptr<const PlayerFactory>;
using PlayerFactoryWeakPtr = std::weak_ptr<PlayerFactory>;
using PlayerFactoryConstWeakPtr = std::weak_ptr<const PlayerFactory>;
using PlayerFactoryUPtr = std::unique_ptr<PlayerFactory>;
using PlayerFactoryConstUPtr = std::unique_ptr<const PlayerFactory>;
class MonsterDatabase;
using MonsterDatabasePtr = std::shared_ptr<MonsterDatabase>;
using MonsterDatabaseConstPtr = std::shared_ptr<const MonsterDatabase>;
using MonsterDatabaseWeakPtr = std::weak_ptr<MonsterDatabase>;
using MonsterDatabaseConstWeakPtr = std::weak_ptr<const MonsterDatabase>;
using MonsterDatabaseUPtr = std::unique_ptr<MonsterDatabase>;
using MonsterDatabaseConstUPtr = std::unique_ptr<const MonsterDatabase>;
class ObjectDatabase;
using ObjectDatabasePtr = std::shared_ptr<ObjectDatabase>;
using ObjectDatabaseConstPtr = std::shared_ptr<const ObjectDatabase>;
using ObjectDatabaseWeakPtr = std::weak_ptr<ObjectDatabase>;
using ObjectDatabaseConstWeakPtr = std::weak_ptr<const ObjectDatabase>;
using ObjectDatabaseUPtr = std::unique_ptr<ObjectDatabase>;
using ObjectDatabaseConstUPtr = std::unique_ptr<const ObjectDatabase>;
class ProjectileDatabase;
using ProjectileDatabasePtr = std::shared_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstPtr = std::shared_ptr<const ProjectileDatabase>;
using ProjectileDatabaseWeakPtr = std::weak_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstWeakPtr = std::weak_ptr<const ProjectileDatabase>;
using ProjectileDatabaseUPtr = std::unique_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstUPtr = std::unique_ptr<const ProjectileDatabase>;
class NpcDatabase;
using NpcDatabasePtr = std::shared_ptr<NpcDatabase>;
using NpcDatabaseConstPtr = std::shared_ptr<const NpcDatabase>;
using NpcDatabaseWeakPtr = std::weak_ptr<NpcDatabase>;
using NpcDatabaseConstWeakPtr = std::weak_ptr<const NpcDatabase>;
using NpcDatabaseUPtr = std::unique_ptr<NpcDatabase>;
using NpcDatabaseConstUPtr = std::unique_ptr<const NpcDatabase>;

class EntityFactory;
using EntityFactoryPtr = std::shared_ptr<EntityFactory>;
using EntityFactoryConstPtr = std::shared_ptr<const EntityFactory>;
using EntityFactoryWeakPtr = std::weak_ptr<EntityFactory>;
using EntityFactoryConstWeakPtr = std::weak_ptr<const EntityFactory>;
using EntityFactoryUPtr = std::unique_ptr<EntityFactory>;
using EntityFactoryConstUPtr = std::unique_ptr<const EntityFactory>;

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
