#pragma once

#include "StarCollisionBlock.hpp"
#include "StarConfig.hpp"
#include "StarEntity.hpp"
#include "StarGameTypes.hpp"
#include "StarLiquidTypes.hpp"
#include "StarSpawnTypeDatabase.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

class SpawnerFacade {
public:
  virtual ~SpawnerFacade() = default;

  [[nodiscard]] virtual auto geometry() const -> WorldGeometry = 0;
  [[nodiscard]] virtual auto clientWindows() const -> List<RectF> = 0;
  // Should return false if the given region is not ready yet for spawning
  [[nodiscard]] virtual auto signalRegion(RectF const& region) const -> bool = 0;

  [[nodiscard]] virtual auto isFreeSpace(RectF const& area) const -> bool = 0;
  [[nodiscard]] virtual auto collision(Vec2I const& position) const -> CollisionKind = 0;
  [[nodiscard]] virtual auto isBackgroundEmpty(Vec2I const& position) const -> bool = 0;
  [[nodiscard]] virtual auto liquidLevel(Vec2I const& pos) const -> LiquidLevel = 0;
  [[nodiscard]] virtual auto spawningProhibited(RectF const& area) const -> bool = 0;

  [[nodiscard]] virtual auto spawnSeed() const -> uint64_t = 0;
  [[nodiscard]] virtual auto spawnProfile(Vec2F const& position) const -> SpawnProfile = 0;
  [[nodiscard]] virtual auto dayLevel() const -> float = 0;
  [[nodiscard]] virtual auto threatLevel() const -> float = 0;

  // May return NullEntityId if spawning fails for some reason.
  [[nodiscard]] virtual auto spawnEntity(Ptr<Entity> entity) const -> EntityId = 0;
  [[nodiscard]] virtual auto getEntity(EntityId entityId) const -> Ptr<Entity> = 0;
  virtual void despawnEntity(EntityId entityId) = 0;
};

class Spawner {
public:
  Spawner();

  void init(Ptr<SpawnerFacade> facade);
  // Despawns all spawned entities before shutting down
  void uninit();

  // An inactive spawner will not spawn new entities into newly visited
  // regions.
  [[nodiscard]] auto active() const -> bool;
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

  [[nodiscard]] auto cellIndexForPosition(Vec2F const& position) const -> Vec2I;
  [[nodiscard]] auto cellIndexesForRange(RectF const& range) const -> List<Vec2I>;
  [[nodiscard]] auto cellRegion(Vec2I const& cellIndex) const -> RectF;

  // Is the cell spawnable, and if so, what are the valid spawn parameters for it?
  [[nodiscard]] auto spawnParametersForCell(Vec2I const& cellIndex) const -> std::optional<SpawnParameters>;

  // Finds a position for the given bounding box inside the given spawn cell
  // which matches the given spawn parameters.
  [[nodiscard]] auto adjustSpawnRegion(RectF const& spawnRegion, RectF const& boundBox, SpawnParameters const& spawnParameters) const -> std::optional<Vec2F>;

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
  Ptr<SpawnerFacade> m_facade;
  HashSet<EntityId> m_spawnedEntities;
  HashMap<Vec2I, float> m_activeSpawnCells;

  bool m_debug;
  HashMap<Vec2I, SpawnCellDebugInfo> m_debugSpawnInfo;
};

}// namespace Star
