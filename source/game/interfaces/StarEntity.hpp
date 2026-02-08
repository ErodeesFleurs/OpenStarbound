#pragma once

#include "StarDamage.hpp"
#include "StarException.hpp"
#include "StarLightSource.hpp"

import std;

namespace Star {

class World;
class RenderCallback;

using EntityException = ExceptionDerived<"EntityException">;

// Specifies how the client should treat an entity created on the client,
// whether it should always be sent to the server and be a slave on the client,
// whether it is allowed to be master on the client, and whether client master
// entities should contribute to client presence.
enum class ClientEntityMode {
  // Always a slave on the client
  ClientSlaveOnly,
  // Can be a master on the client
  ClientMasterAllowed,
  // Can be a master on the client, and when it is contributes to client
  // presence.
  ClientPresenceMaster
};
extern EnumMap<ClientEntityMode> const ClientEntityModeNames;

// The top-level entity type.  The enum order is intended to be in the order in
// which entities should be updated every tick
enum class EntityType : std::uint8_t {
  Plant,
  Object,
  Vehicle,
  ItemDrop,
  PlantDrop,
  Projectile,
  Stagehand,
  Monster,
  Npc,
  Player
};
extern EnumMap<EntityType> const EntityTypeNames;

class Entity {
public:
  virtual ~Entity();

  [[nodiscard]] virtual auto entityType() const -> EntityType = 0;

  // Called when an entity is first inserted into a World.  Calling base class
  // init sets the world pointer, entityId, and entityMode.
  virtual void init(World* world, EntityId entityId, EntityMode mode);

  // Should do whatever steps necessary to take an entity out of a world,
  // default implementation clears the world pointer, entityMode, and entityId.
  virtual void uninit();

  // Write state data that changes over time, and is used to keep slaves in
  // sync.  Can return empty and this is the default.  May be called
  // uninitalized.  Should return the delta to be written to the slave, along
  // with the version to pass into writeDeltaState on the next call.  The first
  // delta written to a slave entity will always be the delta starting with 0.
  virtual auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t>;
  // Will be called with deltas written by writeDeltaState, including if the
  // delta is empty.  interpolationTime will be provided if interpolation is
  // enabled.
  virtual void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {});

  virtual void enableInterpolation(float extrapolationHint);
  virtual void disableInterpolation();

  // Base position of this entity, bound boxes, drawables, and other entity
  // positions are relative to this.
  [[nodiscard]] virtual auto position() const -> Vec2F = 0;

  // Largest bounding-box of this entity.  Any damage boxes / drawables / light
  // or sound *sources* must be contained within this bounding box.  Used for
  // all top-level spatial queries.
  [[nodiscard]] virtual auto metaBoundBox() const -> RectF = 0;

  // By default returns a null rect, if non-null, it defines the area around
  // this entity where it is likely for the entity to physically collide with
  // collision geometry.
  [[nodiscard]] virtual auto collisionArea() const -> RectF;

  // Should this entity allow object / block placement over it, and can the
  // entity immediately be despawned without terribly bad effects?
  [[nodiscard]] virtual auto ephemeral() const -> bool;

  // How should this entity be treated if created on the client?  Defaults to
  // ClientSlave.
  [[nodiscard]] virtual auto clientEntityMode() const -> ClientEntityMode;
  // Should this entity only exist on the master side?
  [[nodiscard]] virtual auto masterOnly() const -> bool;

  [[nodiscard]] virtual auto name() const -> String;
  [[nodiscard]] virtual auto description() const -> String;

  // Gameplay affecting light sources (separate from light sources added during
  // rendering)
  [[nodiscard]] virtual auto lightSources() const -> List<LightSource>;

  // All damage sources for this frame.
  [[nodiscard]] virtual auto damageSources() const -> List<DamageSource>;

  // Return the damage that would result from being hit by the given damage
  // source.  Will be called on master and slave entities.  Culling based on
  // team damage and self damage will be done outside of this query.
  [[nodiscard]] virtual auto queryHit(DamageSource const& source) const -> std::optional<HitType>;

  // Return the polygonal area in which the entity can be hit. Not used for
  // actual hit computation, only for determining more precisely where a
  // hit intersection occurred (e.g. by projectiles)
  [[nodiscard]] virtual auto hitPoly() const -> std::optional<PolyF>;

  // Apply a request to damage this entity. Will only be called on Master
  // entities. DamageRequest might be adjusted based on protection and other
  // effects
  virtual auto applyDamage(DamageRequest const& damage) -> List<DamageNotification>;

  // Pull any pending damage notifications applied internally, only called on
  // Master entities.
  virtual auto selfDamageNotifications() -> List<DamageNotification>;

  // Called on master entities when a DamageRequest has been generated due to a
  // DamageSource from this entity being applied to another entity.  Will be
  // called on the *causing* entity of the damage.
  virtual void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest);

  // Called on master entities when this entity has damaged another entity.
  // Only called on the *source entity* of the damage, which may be different
  // than the causing entity.
  virtual void damagedOther(DamageNotification const& damage);

  // Returning true here indicates that this entity should be removed from the
  // world, default returns false.
  [[nodiscard]] virtual auto shouldDestroy() const -> bool;
  // Will be called once before removing the entity from the World on both
  // master and slave entities.
  virtual void destroy(RenderCallback* renderCallback);

  // Entities can send other entities potentially remote messages and get
  // responses back from them, and should implement this to receive and respond
  // to messages.  If the message is NOT handled, should return Nothing,
  // otherwise should return some Json value.
  // This will only ever be called on master entities.
  virtual auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json>;

  virtual void update(float dt, std::uint64_t currentStep);

  virtual void render(RenderCallback* renderer);

  virtual void renderLightSources(RenderCallback* renderer);

  [[nodiscard]] auto entityId() const -> EntityId;

  [[nodiscard]] auto getTeam() const -> EntityDamageTeam;

  // Returns true if an entity is initialized in a world, and thus has a valid
  // world pointer, entity id, and entity mode.
  [[nodiscard]] auto inWorld() const -> bool;

  // Throws an exception if not currently in a world.
  [[nodiscard]] auto world() const -> World*;
  // Returns nullptr if not currently in a world.
  [[nodiscard]] auto worldPtr() const -> World*;

  // Specifies if the entity is to be saved to disk alongside the sector or
  // despawned.
  [[nodiscard]] auto persistent() const -> bool;

  // Entity should keep any sector it is in alive.  Default implementation
  // returns false.
  [[nodiscard]] auto keepAlive() const -> bool;

  // If set, then the entity will be discoverable by its unique id and will be
  // indexed in the stored world.  Unique ids must be different across all
  // entities in a single world.
  [[nodiscard]] auto uniqueId() const -> std::optional<String>;

  // EntityMode will only be set if the entity is initialized, if the entity is
  // uninitialized then isMaster and isSlave will both return false.
  [[nodiscard]] auto entityMode() const -> std::optional<EntityMode>;
  [[nodiscard]] auto isMaster() const -> bool;
  [[nodiscard]] auto isSlave() const -> bool;

protected:
  Entity();

  void setPersistent(bool persistent);
  void setKeepAlive(bool keepAlive);
  void setUniqueId(std::optional<String> uniqueId);
  void setTeam(EntityDamageTeam newTeam);

private:
  EntityId m_entityId;
  std::optional<EntityMode> m_entityMode;
  bool m_persistent;
  bool m_keepAlive;
  std::optional<String> m_uniqueId;
  World* m_world;
  EntityDamageTeam m_team;
};

template <typename EntityT>
using EntityCallbackOf = std::function<void(std::shared_ptr<EntityT> const&)>;

template <typename EntityT>
using EntityFilterOf = std::function<bool(std::shared_ptr<EntityT> const&)>;

using EntityCallback = EntityCallbackOf<Entity>;
using EntityFilter = EntityFilterOf<Entity>;

// Filters based first on dynamic casting to the given type, then optionally on
// the given derived type filter.
template <typename EntityT>
auto entityTypeFilter(std::function<bool(std::shared_ptr<EntityT> const&)> filter = {}) -> EntityFilter {
  return [filter](Ptr<Entity> const& e) -> bool {
    if (auto entity = as<EntityT>(e)) {
      return !filter || filter(entity);
    } else {
      return false;
    }
  };
}
}// namespace Star
