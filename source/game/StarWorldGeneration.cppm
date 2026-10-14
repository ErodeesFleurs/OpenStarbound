module;
import star.json;
import star.xx_hash;
import star.vector;
import star.set;
import star.map;
import star.random;
import star.game_types;
import star.material_types;
import star.poly;
import star.list;
import star.bi_map;
import std;
import star.rect;
import star.thread;
import star.worker_pool;

import star.sector_array_2d;
import star.maybe;
import star.color;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.variant;
import star.string;
import star.version;
import star.image;
import star.ordered_map;
import star.lru_cache;
import star.id_map;
#include "StarBTree.hpp"
#include "StarDataStreamDevices.hpp"
import star.btree_database;
#include "StarCasting.hpp"
import star.ordered_set;
import star.rpc_promise;
import star.perlin;
import star.weighted_pool;
#include "StarStrongTypedef.hpp"

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;

import star.item_descriptor;
import star.world_geometry;
import star.dungeon_generator;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
#include "StarLuaRoot.hpp"
import star.versioning_database;
import star.world_storage;
import star.cellular_liquid;
import star.plant_database;
import star.biome_placement;

import star.falling_blocks_agent;

namespace Star {
class WorldServer;
using WorldServerPtr = std::shared_ptr<WorldServer>;
using WorldServerConstPtr = std::shared_ptr<const WorldServer>;
using WorldServerWeakPtr = std::weak_ptr<WorldServer>;
using WorldServerConstWeakPtr = std::weak_ptr<const WorldServer>;
using WorldServerUPtr = std::unique_ptr<WorldServer>;
using WorldServerConstUPtr = std::unique_ptr<const WorldServer>;
class Plant;
using PlantPtr = std::shared_ptr<Plant>;
using PlantConstPtr = std::shared_ptr<const Plant>;
using PlantWeakPtr = std::weak_ptr<Plant>;
using PlantConstWeakPtr = std::weak_ptr<const Plant>;
using PlantUPtr = std::unique_ptr<Plant>;
using PlantConstUPtr = std::unique_ptr<const Plant>;
class MaterialDatabase;
using MaterialDatabasePtr = std::shared_ptr<MaterialDatabase>;
using MaterialDatabaseConstPtr = std::shared_ptr<const MaterialDatabase>;
using MaterialDatabaseWeakPtr = std::weak_ptr<MaterialDatabase>;
using MaterialDatabaseConstWeakPtr = std::weak_ptr<const MaterialDatabase>;
using MaterialDatabaseUPtr = std::unique_ptr<MaterialDatabase>;
using MaterialDatabaseConstUPtr = std::unique_ptr<const MaterialDatabase>;
class LiquidsDatabase;
using LiquidsDatabasePtr = std::shared_ptr<LiquidsDatabase>;
using LiquidsDatabaseConstPtr = std::shared_ptr<const LiquidsDatabase>;
using LiquidsDatabaseWeakPtr = std::weak_ptr<LiquidsDatabase>;
using LiquidsDatabaseConstWeakPtr = std::weak_ptr<const LiquidsDatabase>;
using LiquidsDatabaseUPtr = std::unique_ptr<LiquidsDatabase>;
using LiquidsDatabaseConstUPtr = std::unique_ptr<const LiquidsDatabase>;
}

import star.micro_dungeon;
import star.spawn_type_database;
import star.spawner;

namespace Star {

class FallingBlocksWorld;
using FallingBlocksWorldPtr = std::shared_ptr<FallingBlocksWorld>;
using FallingBlocksWorldConstPtr = std::shared_ptr<const FallingBlocksWorld>;
using FallingBlocksWorldWeakPtr = std::weak_ptr<FallingBlocksWorld>;
using FallingBlocksWorldConstWeakPtr = std::weak_ptr<const FallingBlocksWorld>;
using FallingBlocksWorldUPtr = std::unique_ptr<FallingBlocksWorld>;
using FallingBlocksWorldConstUPtr = std::unique_ptr<const FallingBlocksWorld>;
class DungeonGeneratorWorld;
using DungeonGeneratorWorldPtr = std::shared_ptr<DungeonGeneratorWorld>;
using DungeonGeneratorWorldConstPtr = std::shared_ptr<const DungeonGeneratorWorld>;
using DungeonGeneratorWorldWeakPtr = std::weak_ptr<DungeonGeneratorWorld>;
using DungeonGeneratorWorldConstWeakPtr = std::weak_ptr<const DungeonGeneratorWorld>;
using DungeonGeneratorWorldUPtr = std::unique_ptr<DungeonGeneratorWorld>;
using DungeonGeneratorWorldConstUPtr = std::unique_ptr<const DungeonGeneratorWorld>;
class SpawnerWorld;
using SpawnerWorldPtr = std::shared_ptr<SpawnerWorld>;
using SpawnerWorldConstPtr = std::shared_ptr<const SpawnerWorld>;
using SpawnerWorldWeakPtr = std::weak_ptr<SpawnerWorld>;
using SpawnerWorldConstWeakPtr = std::weak_ptr<const SpawnerWorld>;
using SpawnerWorldUPtr = std::unique_ptr<SpawnerWorld>;
using SpawnerWorldConstUPtr = std::unique_ptr<const SpawnerWorld>;
class WorldGenerator;
using WorldGeneratorPtr = std::shared_ptr<WorldGenerator>;
using WorldGeneratorConstPtr = std::shared_ptr<const WorldGenerator>;
using WorldGeneratorWeakPtr = std::weak_ptr<WorldGenerator>;
using WorldGeneratorConstWeakPtr = std::weak_ptr<const WorldGenerator>;
using WorldGeneratorUPtr = std::unique_ptr<WorldGenerator>;
using WorldGeneratorConstUPtr = std::unique_ptr<const WorldGenerator>;

class LiquidWorld : public CellularLiquidWorld<LiquidId> {
public:
  LiquidWorld(WorldServer* world);

  Vec2I uniqueLocation(Vec2I const& location) const override;
  CellularLiquidCell<LiquidId> cell(Vec2I const& location) const override;
  float drainLevel(Vec2I const& location) const override;
  void setFlow(Vec2I const& location, CellularLiquidFlowCell<LiquidId> const& flow) override;
  void liquidInteraction(Vec2I const& a, LiquidId aLiquid, Vec2I const& b, LiquidId bLiquid) override;
  void liquidCollision(Vec2I const& pos, LiquidId liquid, Vec2I const& blockPos) override;

private:
  WorldServer* m_worldServer;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  MaterialDatabaseConstPtr m_materialDatabase;
};

class FallingBlocksWorld : public FallingBlocksFacade {
public:
  FallingBlocksWorld(WorldServer* world);

  FallingBlockType blockType(Vec2I const& pos) override;
  void moveBlock(Vec2I const& from, Vec2I const& to) override;

private:
  WorldServer* m_worldServer;
  MaterialDatabaseConstPtr m_materialDatabase;
};

class DungeonGeneratorWorld : public DungeonGeneratorWorldFacade {
public:
  DungeonGeneratorWorld(WorldServer* worldServer, bool markForActivation);

  void markRegion(RectI const& region) override;
  void markTerrain(PolyF const& region) override;
  void markSpace(PolyF const& region) override;

  void setForegroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant) override;
  void setBackgroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant) override;
  void setForegroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift) override;
  void setBackgroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift) override;
  void placeObject(
      Vec2I const& pos, String const& objectName, Star::Direction direction, Json const& parameters) override;
  void placeVehicle(Vec2F const& pos, String const& vehicleName, Json const& parameters) override;
  void placeSurfaceBiomeItems(Vec2I const& pos) override;
  void placeBiomeTree(Vec2I const& pos) override;
  void addDrop(Vec2F const& position, ItemDescriptor const& item) override;
  void spawnNpc(Vec2F const& position, Json const& parameters) override;
  void spawnStagehand(Vec2F const& position, Json const& definition) override;
  void setLiquid(Vec2I const& pos, LiquidStore const& liquid) override;
  void setPlayerStart(Vec2F const& startPosition) override;
  void connectWireGroup(List<Vec2I> const& wireGroup) override;
  void setTileProtection(DungeonId dungeonId, bool isProtected) override;
  bool checkSolid(Vec2I const& position, TileLayer layer) override;
  bool checkOpen(Vec2I const& position, TileLayer open) override;
  bool checkOceanLiquid(Vec2I const& position) override;
  DungeonId getDungeonIdAt(Vec2I const& position) override;
  void setDungeonIdAt(Vec2I const& position, DungeonId dungeonId) override;
  void clearTileEntities(RectI const& bounds, Set<Vec2I> const& positions, bool clearAnchoredObjects) override;

  WorldGeometry getWorldGeometry() const override;

private:
  void placePlant(PlantPtr const& plant, Vec2I const& position);
  void placeBiomeItems(Vec2I const& pos, List<BiomeItemPlacement>& potentialItems);

  WorldServer* m_worldServer;
  bool m_markForActivation;
};

class SpawnerWorld : public SpawnerFacade {
public:
  SpawnerWorld(WorldServer* worldServer);

  WorldGeometry geometry() const override;
  List<RectF> clientWindows() const override;
  bool signalRegion(RectF const& region) const override;
  CollisionKind collision(Vec2I const& position) const override;
  bool isFreeSpace(RectF const& area) const override;
  bool isBackgroundEmpty(Vec2I const& position) const override;
  LiquidLevel liquidLevel(Vec2I const& position) const override;
  bool spawningProhibited(RectF const& area) const override;
  std::uint64_t spawnSeed() const override;
  SpawnProfile spawnProfile(Vec2F const& position) const override;
  float dayLevel() const override;
  float threatLevel() const override;
  EntityId spawnEntity(EntityPtr entity) const override;
  void despawnEntity(EntityId entityId) override;
  EntityPtr getEntity(EntityId entityId) const override;

private:
  WorldServer* m_worldServer;
};

class WorldGenerator : public WorldGeneratorFacade {
public:
  WorldGenerator(WorldServer* server);

  void generateSectorLevel(WorldStorage* worldStorage, Sector const& sector, SectorGenerationLevel generationLevel) override;
  void sectorLoadLevelChanged(WorldStorage* worldStorage, Sector const& sector, SectorLoadLevel loadLevel) override;
  void terraformSector(WorldStorage* worldStorage, Sector const& sector) override;
  void initEntity(WorldStorage* worldStorage, EntityId entityId, EntityPtr const& entity) override;
  void destructEntity(WorldStorage* worldStorage, EntityPtr const& entity) override;
  bool entityKeepAlive(WorldStorage* worldStorage, EntityPtr const& entity) const override;
  bool entityPersistent(WorldStorage* worldStorage, EntityPtr const& entity) const override;
  RpcPromise<Vec2I> enqueuePlacement(List<BiomeItemDistribution> distributions, Maybe<DungeonId> id) override;

  void replaceBiomeBlocks(ServerTile* tile);

private:
  struct QueuedPlacement {
    List<BiomeItemDistribution> distributions;
    Maybe<DungeonId> dungeonId;
    RpcPromiseKeeper<Vec2I> promise;
    bool fulfilled;
  };

  void prepareTiles(WorldStorage* worldStorage, Sector const& sector);
  void generateMicroDungeons(WorldStorage* worldStorage, Sector const& sector);
  void generateCaveLiquid(WorldStorage* worldStorage, Sector const& sector);
  void prepareSector(WorldStorage* worldStorage, Sector const& sector);
  void prepareSectorBiomeBlocks(WorldStorage* worldStorage, Sector const& sector);

  void placeBiomeGrass(WorldStorage* worldStorage, ServerTile* tile, Vec2I const& position);

  void reapplyBiome(WorldStorage* worldStorage, Sector const& sector);

  Set<Vec2I> caveLiquidSeeds(WorldStorage* worldStorage, Sector const& sector);
  Map<Vec2I, float> determineLiquidLevel(Set<Vec2I> const& spots, Set<Vec2I> const& filled);
  void levelCluster(Set<Vec2I>& cluster, Set<Vec2I> const& filled, Map<Vec2I, float>& results);

  // Special plant placement routine that does slight terrain adjustments to
  // fit plants.
  bool placePlant(WorldStorage* worldStorage, PlantPtr const& plant, Vec2I const& position);

  WorldServer* m_worldServer;
  MicroDungeonFactoryPtr m_microDungeonFactory;
  List<QueuedPlacement> m_queuedPlacements;
};

}

export module star.world_generation;

export namespace Star {
  using ::Star::LiquidWorld;
  using ::Star::FallingBlocksWorld;
  using ::Star::DungeonGeneratorWorld;
  using ::Star::SpawnerWorld;
  using ::Star::WorldGenerator;
  using ::Star::FallingBlocksWorldPtr;
  using ::Star::FallingBlocksWorldConstPtr;
  using ::Star::FallingBlocksWorldWeakPtr;
  using ::Star::FallingBlocksWorldConstWeakPtr;
  using ::Star::FallingBlocksWorldUPtr;
  using ::Star::FallingBlocksWorldConstUPtr;
  using ::Star::DungeonGeneratorWorldPtr;
  using ::Star::DungeonGeneratorWorldConstPtr;
  using ::Star::DungeonGeneratorWorldWeakPtr;
  using ::Star::DungeonGeneratorWorldConstWeakPtr;
  using ::Star::DungeonGeneratorWorldUPtr;
  using ::Star::DungeonGeneratorWorldConstUPtr;
  using ::Star::SpawnerWorldPtr;
  using ::Star::SpawnerWorldConstPtr;
  using ::Star::SpawnerWorldWeakPtr;
  using ::Star::SpawnerWorldConstWeakPtr;
  using ::Star::SpawnerWorldUPtr;
  using ::Star::SpawnerWorldConstUPtr;
  using ::Star::WorldGeneratorPtr;
  using ::Star::WorldGeneratorConstPtr;
  using ::Star::WorldGeneratorWeakPtr;
  using ::Star::WorldGeneratorConstWeakPtr;
  using ::Star::WorldGeneratorUPtr;
  using ::Star::WorldGeneratorConstUPtr;
}
