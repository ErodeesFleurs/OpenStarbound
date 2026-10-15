module;

import star.random;
import star.periodic;
import star.id_map;
import std;
import star.map;
import star.set;
import star.maybe;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.ttl_cache;
import star.world_geometry;
import star.vector;
import star.game_types;
import star.liquid_types;
import star.list;
import star.bi_map;
import star.collision_block;
import star.weighted_pool;

import star.spawn_type_database;

import star.rect;
namespace Star {

class Entity;

class SpawnerFacade {
public:
  virtual ~SpawnerFacade(){};

  virtual WorldGeometry geometry() const = 0;
  virtual List<RectF> clientWindows() const = 0;
  // Should return false if the given region is not ready yet for spawning
  virtual bool signalRegion(RectF const& region) const = 0;

  virtual bool isFreeSpace(RectF const& area) const = 0;
  virtual CollisionKind collision(Vec2I const& position) const = 0;
  virtual bool isBackgroundEmpty(Vec2I const& position) const = 0;
  virtual LiquidLevel liquidLevel(Vec2I const& pos) const = 0;
  virtual bool spawningProhibited(RectF const& area) const = 0;

  virtual std::uint64_t spawnSeed() const = 0;
  virtual SpawnProfile spawnProfile(Vec2F const& position) const = 0;
  virtual float dayLevel() const = 0;
  virtual float threatLevel() const = 0;

  // May return NullEntityId if spawning fails for some reason.
  virtual EntityId spawnEntity(std::shared_ptr<Entity> entity) const = 0;
  virtual std::shared_ptr<Entity> getEntity(EntityId entityId) const = 0;
  virtual void despawnEntity(EntityId entityId) = 0;
};

class Spawner {
public:
  Spawner();

  void init(std::shared_ptr<SpawnerFacade> facade);
  // Despawns all spawned entities before shutting down
  void uninit();

  // An inactive spawner will not spawn new entities into newly visited
  // regions.
  bool active() const;
  void setActive(bool active);

  // Activates the given spawn cells, spawning monsters in them if necessary.
  void activateRegion(RectF region);
  // Activates the given spawn cells *without* spawning monsters in them, does
  // nothing if they are already active.
  void activateEmptyRegion(RectF region);

  void update(float dt);

private:
  struct SpawnCellDebugInfo {
    SpawnParameters spawnParameters;
    int spawns;
    int spawnAttempts;
  };

  Vec2I cellIndexForPosition(Vec2F const& position) const;
  List<Vec2I> cellIndexesForRange(RectF const& range) const;
  RectF cellRegion(Vec2I const& cellIndex) const;

  // Is the cell spawnable, and if so, what are the valid spawn parameters for it?
  Maybe<SpawnParameters> spawnParametersForCell(Vec2I const& cellIndex) const;

  // Finds a position for the given bounding box inside the given spawn cell
  // which matches the given spawn parameters.
  Maybe<Vec2F> adjustSpawnRegion(RectF const& spawnRegion, RectF const& boundBox, SpawnParameters const& spawnParameters) const;

  // Spawns monsters in a newly active cell
  void spawnInCell(Vec2I const& cell);

  void debugShowSpawnCells();

  unsigned m_spawnCellSize;
  unsigned m_spawnCellMinimumEmptyTiles;
  unsigned m_spawnCellMinimumLiquidTiles;
  unsigned m_spawnCellMinimumNearSurfaceTiles;
  unsigned m_spawnCellMinimumNearCeilingTiles;
  unsigned m_spawnCellMinimumAirTiles;
  unsigned m_spawnCellMinimumExposedTiles;
  unsigned m_spawnCellNearSurfaceDistance;
  unsigned m_spawnCellNearCeilingDistance;

  float m_minimumDayLevel;
  float m_minimumLiquidLevel;
  float m_spawnCheckResolution;
  int m_spawnSurfaceCheckDistance;
  int m_spawnCeilingCheckDistance;
  float m_spawnProhibitedCheckPadding;

  float m_spawnCellLifetime;
  unsigned m_windowActivationBorder;

  bool m_active;
  std::shared_ptr<SpawnerFacade> m_facade;
  HashSet<EntityId> m_spawnedEntities;
  HashMap<Vec2I, float> m_activeSpawnCells;

  bool m_debug;
  HashMap<Vec2I, SpawnCellDebugInfo> m_debugSpawnInfo;
};

}

export module star.spawner;

export namespace Star {
using ::Star::SpawnerFacade;
using ::Star::Spawner;
using ::Star::Entity;
}
