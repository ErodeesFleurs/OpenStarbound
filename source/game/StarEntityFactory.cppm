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
class VehicleDatabase;
class VersioningDatabase;
class PlayerFactory;
class MonsterDatabase;
class ObjectDatabase;
class ProjectileDatabase;
class NpcDatabase;

struct EntityFactoryExceptionTag {
  static constexpr char const* name() { return "EntityFactoryException"; }
};
using EntityFactoryException = StarError<EntityFactoryExceptionTag, StarException>;

class EntityFactory {
public:
  EntityFactory();

  ByteArray netStoreEntity(std::shared_ptr<Entity> const& entity, NetCompatibilityRules rules = {}) const;
  std::shared_ptr<Entity> netLoadEntity(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

  Json diskStoreEntity(std::shared_ptr<Entity> const& entity) const;
  std::shared_ptr<Entity> diskLoadEntity(EntityType type, Json const& diskStore) const;

  Json loadVersionedJson(VersionedJson const& versionedJson, EntityType expectedType) const;
  VersionedJson storeVersionedJson(EntityType type, Json const& store) const;

  // Wraps the normal Json based Entity store / load in a VersionedJson, and
  // uses sripts in the VersionedingDatabase to bring the version of the store
  // forward to match the current version.
  std::shared_ptr<Entity> loadVersionedEntity(VersionedJson const& versionedJson) const;
  VersionedJson storeVersionedEntity(std::shared_ptr<Entity> const& entityPtr) const;

private:
  static EnumMap<EntityType> const EntityStorageIdentifiers;

  mutable RecursiveMutex m_mutex;

  std::shared_ptr<PlayerFactory const> m_playerFactory;
  std::shared_ptr<MonsterDatabase const> m_monsterDatabase;
  std::shared_ptr<ObjectDatabase const> m_objectDatabase;
  std::shared_ptr<ProjectileDatabase const> m_projectileDatabase;
  std::shared_ptr<NpcDatabase const> m_npcDatabase;
  std::shared_ptr<VehicleDatabase const> m_vehicleDatabase;
  std::shared_ptr<VersioningDatabase const> m_versioningDatabase;
};

}

export module star.entity_factory;

export namespace Star {
  using ::Star::VersionedJson;
  using ::Star::VehicleDatabase;
  using ::Star::VersioningDatabase;
  using ::Star::PlayerFactory;
  using ::Star::MonsterDatabase;
  using ::Star::ObjectDatabase;
  using ::Star::ProjectileDatabase;
  using ::Star::NpcDatabase;
  using ::Star::EntityFactory;
  using ::Star::EntityFactoryExceptionTag;
  using ::Star::EntityFactoryException;
}
