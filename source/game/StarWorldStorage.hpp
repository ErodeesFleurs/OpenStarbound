#pragma once

#include "StarBTreeDatabase.hpp"
#include "StarBiomePlacement.hpp"
#include "StarConfig.hpp"
#include "StarEntity.hpp"
#include "StarException.hpp"
#include "StarRpcPromise.hpp"
#include "StarVersioningDatabase.hpp"
#include "StarWorldTiles.hpp"

import std;

namespace Star {

using WorldStorageException = ExceptionDerived<"WorldStorageException">;

class WorldStorage;
class EntityMap;
// STAR_STRUCT(WorldGeneratorFacade);

using WorldChunks = HashMap<ByteArray, std::optional<ByteArray>>;

enum class SectorLoadLevel : uint8_t {
  None = 0,
  Tiles = 1,
  Entities = 2,

  Loaded = 2
};

enum class SectorGenerationLevel : uint8_t {
  None = 0,
  BaseTiles = 1,
  MicroDungeons = 2,
  CaveLiquid = 3,
  Finalize = 4,

  Complete = 4,

  Terraform = 5
};

struct WorldGeneratorFacade {
  using Sector = ServerTileSectorArray::Sector;

  WorldGeneratorFacade() = default;
  virtual ~WorldGeneratorFacade() = default;

  // Should bring a given sector from generationLevel - 1 to generationLevel.
  virtual void generateSectorLevel(WorldStorage* storage, Sector const& sector, SectorGenerationLevel generationLevel) = 0;

  virtual void sectorLoadLevelChanged(WorldStorage* storage, Sector const& sector, SectorLoadLevel loadLevel) = 0;

  // Perform terraforming operations (biome reapplication) on the given sector
  virtual void terraformSector(WorldStorage* storage, Sector const& sector) = 0;

  // Called after an entity is loaded, but before the entity is added to the
  // EntityMap.
  virtual void initEntity(WorldStorage* storage, EntityId newEntityId, Ptr<Entity> const& entity) = 0;

  // Called after the entity is removed from the entity map but before it is
  // stored.
  virtual void destructEntity(WorldStorage* storage, Ptr<Entity> const& entity) = 0;

  // Should return true if this entity should maintain the sector, false
  // otherwise.
  virtual auto entityKeepAlive(WorldStorage* storage, Ptr<Entity> const& entity) const -> bool = 0;

  // Should return true if this entity should be stored along with the world,
  // false otherwise.
  virtual auto entityPersistent(WorldStorage* storage, Ptr<Entity> const& entity) const -> bool = 0;

  // Queues up a microdungeon. Fulfills the rpc promise with the position the
  // microdungeon was placed at
  virtual auto enqueuePlacement(List<BiomeItemDistribution> placements, std::optional<DungeonId> id) -> RpcPromise<Vec2I> = 0;
};

// Handles paging entity and tile data in / out of disk backed storage for
// WorldServer and triggers initial generation.  Ties tile sectors to entity
// sectors, and allows for multiple stage generation of those sectors.  Sector
// generation is done in stages, so that lower generation stages are done in a
// one sector border around the higher generation stages.
//
// WorldStorage is designed so that once constructed, any exceptions triggered
// during loading, unloading, or generation that would result in an
// indeterminate world state cause the underlying database to be rolled back
// and then immediately closed.  The underlying database committed only when
// destructed without error, or a manual call to sync().
class WorldStorage {
public:
  using Sector = ServerTileSectorArray::Sector;
  using TileArray = ServerTileSectorArray::Array;
  using TileArrayPtr = ServerTileSectorArray::ArrayPtr;

  static auto getWorldChunksUpdate(WorldChunks const& oldChunks, WorldChunks const& newChunks) -> WorldChunks;
  static void applyWorldChunksUpdateToFile(String const& file, WorldChunks const& update);
  static auto getWorldChunksFromFile(String const& file) -> WorldChunks;

  // Create a new world of the given size.
  WorldStorage(Vec2U const& worldSize, Ptr<IODevice> const& device, Ptr<WorldGeneratorFacade> const& generatorFacade);
  // Read an existing world.
  WorldStorage(Ptr<IODevice> const& device, Ptr<WorldGeneratorFacade> const& generatorFacade);
  // Read an in-memory world.
  WorldStorage(WorldChunks const& chunks, Ptr<WorldGeneratorFacade> const& generatorFacade);
  ~WorldStorage();

  auto worldMetadata() -> VersionedJson;
  void setWorldMetadata(VersionedJson const& metadata);

  auto tileArray() const -> ServerTileSectorArrayPtr const&;
  auto entityMap() const -> Ptr<EntityMap> const&;

  auto sectorForPosition(Vec2I const& position) const -> std::optional<Sector>;
  auto sectorsForRegion(RectI const& region) const -> List<Sector>;
  auto regionForSector(Sector sector) const -> std::optional<RectI>;

  auto sectorLoadLevel(Sector sector) const -> SectorLoadLevel;
  // Returns the sector generation level if it is currently loaded, nothing
  // otherwise.
  auto sectorGenerationLevel(Sector sector) const -> std::optional<SectorGenerationLevel>;
  // Returns true if the sector is both loaded and fully generated.
  auto sectorActive(Sector) const -> bool;

  // Fully load the given sector and reset its TTL without triggering any
  // generation.
  void loadSector(Sector sector);
  // Fully load, reset the TTL, and if necessary, fully generate the given
  // sector.
  void activateSector(Sector sector);
  // Queue the given sector for activation, if it is not already active.  If
  // the sector is loaded at all, also resets the TTL.
  void queueSectorActivation(Sector sector);

  // Immediately (synchronously) fully generates the sector, then flags it as requiring
  // terraforming (biome reapplication) which will be handled by the normal generation process
  void triggerTerraformSector(Sector sector);

  // Queues up a microdungeon. Fulfills the rpc promise with the position the
  // microdungeon was placed at
  auto enqueuePlacement(List<BiomeItemDistribution> placements, std::optional<DungeonId> id) -> RpcPromise<Vec2I>;

  // Return the remaining time to live for a sector, if loaded.  A sector's
  // time to live is reset when loaded or generated, and when the time to live
  // reaches zero, the sector is automatically unloaded.
  auto sectorTimeToLive(Sector sector) const -> std::optional<float>;
  // Set the given sector's time to live, if it is loaded at all.  Returns
  // false if the sector was not loaded so no action was taken.
  auto setSectorTimeToLive(Sector sector, float newTimeToLive) -> bool;

  // Returns the position for a given unique entity if it exists in this world,
  // loaded or not.
  auto findUniqueEntity(String const& uniqueId) -> std::optional<Vec2F>;

  // If the given unique entity is not loaded, loads its sector and then if the
  // unique entity is found, returns the entity id, otherwise NullEntityId.
  auto loadUniqueEntity(String const& uniqueId) -> EntityId;

  // Does any queued generation work, potentially limiting the total number of
  // increases of SectorGenerationLevel by the sectorGenerationLevelLimit, if
  // given.  If sectorOrdering is given, then it will be used to prioritize the
  // queued sectors.
  void generateQueue(std::optional<size_t> sectorGenerationLevelLimit, std::function<bool(Sector, Sector)> sectorOrdering = {});
  // Ticks down the TTL on sectors and generation queue entries, stores old
  // sectors, expires old generation queue entries, and unloads any zombie
  // entities.
  void tick(float dt, String const* worldId = nullptr);

  // Unload all sectors that can be unloaded (if force is specified, ALWAYS
  // unloads all sectors)
  void unloadAll(bool force = false);

  // Sync all active sectors without unloading them, and commits the underlying
  // database.
  void sync();

  // Syncs all active sectors to disk and stores the full content of the world
  // into memory.
  auto readChunks() -> WorldChunks;

  // if this is set, all terrain generation is assumed to be handled by dungeon placement
  // and steps such as microdungeons, biome objects and grass mods will be skipped
  auto floatingDungeonWorld() const -> bool;
  void setFloatingDungeonWorld(bool floatingDungeonWorld);

private:
  enum class StoreType : uint8_t {
    Metadata = 0,
    TileSector = 1,
    EntitySector = 2,
    UniqueIndex = 3,
    SectorUniques = 4
  };

  using SectorAndPosition = std::pair<Sector, Vec2F>;

  struct WorldMetadataStore {
    Vec2U worldSize;
    VersionedJson userMetadata;
  };

  using EntitySectorStore = List<VersionedJson>;
  // Map of uuid to entity's position and sector they were stored in.
  using UniqueIndexStore = HashMap<String, SectorAndPosition>;
  // Set of unique ids that are stored in a given sector
  using SectorUniqueStore = HashSet<String>;

  struct TileSectorStore {
    TileSectorStore();

    // Also store generation level along with tiles, simply because tiles are
    // the first things to be loaded and the last to be stored.
    SectorGenerationLevel generationLevel;

    VersionNumber tileSerializationVersion;
    TileArrayPtr tiles;
  };

  struct SectorMetadata {
    SectorMetadata();

    SectorLoadLevel loadLevel;
    SectorGenerationLevel generationLevel;
    float timeToLive;
  };

  static auto metadataKey() -> ByteArray;
  static auto readWorldMetadata(ByteArray const& data) -> WorldMetadataStore;
  static auto writeWorldMetadata(WorldMetadataStore const& metadata) -> ByteArray;

  static auto entitySectorKey(Sector const& sector) -> ByteArray;
  static auto readEntitySector(ByteArray const& data) -> EntitySectorStore;
  static auto writeEntitySector(EntitySectorStore const& store) -> ByteArray;

  static auto tileSectorKey(Sector const& sector) -> ByteArray;
  static auto readTileSector(ByteArray const& data) -> TileSectorStore;
  static auto writeTileSector(TileSectorStore const& store) -> ByteArray;

  static auto uniqueIndexKey(String const& uniqueId) -> ByteArray;
  static auto readUniqueIndexStore(ByteArray const& data) -> UniqueIndexStore;
  static auto writeUniqueIndexStore(UniqueIndexStore const& store) -> ByteArray;

  static auto sectorUniqueKey(Sector const& sector) -> ByteArray;
  static auto readSectorUniqueStore(ByteArray const& data) -> SectorUniqueStore;
  static auto writeSectorUniqueStore(SectorUniqueStore const& store) -> ByteArray;

  static void openDatabase(BTreeDatabase& db, Ptr<IODevice> device);

  WorldStorage();

  auto belongsInSector(Sector const& sector, Vec2F const& position) const -> bool;

  // Generate a random TTL value in the configured range
  auto randomizedSectorTTL() const -> float;

  // Generate the given sector to the given generation level.  If
  // sectorGenerationLevelLimit is given, stops work as soon as the given
  // number of generation level changes has occurred.  Returns whether the
  // given sector was fully generated, and the total number of generation
  // levels increased.  If any sector's generation level is brought up at all,
  // it will also reset the TTL for that sector.
  auto generateSectorToLevel(Sector const& sector, SectorGenerationLevel targetGenerationLevel, size_t sectorGenerationLevelLimit = std::numeric_limits<std::size_t>::max()) -> std::pair<bool, size_t>;

  // Bring the sector up to the given load level, and all surrounding sectors
  // as appropriate.  If the load level is brought up, also resets the TTL.
  void loadSectorToLevel(Sector const& sector, SectorLoadLevel targetLoadLevel);

  // Store and unload the given sector to the given level, given the state of
  // the surrounding sectors.  If force is true, will always unload to the
  // given level.
  auto unloadSectorToLevel(Sector const& sector, SectorLoadLevel targetLoadLevel, bool force = false) -> bool;

  // Sync this sector to disk without unloading it.
  void syncSector(Sector const& sector);

  // Returns the sectors within WorldSectorSize of the given sector.  This is
  // *not exactly the same* as the surrounding 9 sectors in a square pattern,
  // because first this does not return invalid sectors, and second, If a world
  // is not evenly divided by the sector size, this may return extra sectors on
  // one side because they are within range.
  auto adjacentSectors(Sector const& sector) const -> List<Sector>;

  // Replace the sector uniques for this sector with the given set
  void updateSectorUniques(Sector const& sector, UniqueIndexStore const& sectorUniques);
  // Merge the stored sector uniques for this sector with the given set
  void mergeSectorUniques(Sector const& sector, UniqueIndexStore const& sectorUniques);

  auto getUniqueIndexEntry(String const& uniqueId) -> std::optional<SectorAndPosition>;
  void setUniqueIndexEntry(String const& uniqueId, SectorAndPosition const& sectorAndPosition);
  // Remove the index entry for this unique id, if the index entry found points
  // to the given sector
  void removeUniqueIndexEntry(String const& uniqueId, Sector const& sector);

  Vec2F m_sectorTimeToLive;
  float m_generationQueueTimeToLive;

  ServerTileSectorArrayPtr m_tileArray;
  Ptr<EntityMap> m_entityMap;
  Ptr<WorldGeneratorFacade> m_generatorFacade;

  bool m_floatingDungeonWorld;

  StableHashMap<Sector, SectorMetadata> m_sectorMetadata;
  OrderedHashMap<Sector, float> m_generationQueue;
  BTreeDatabase m_db;
};

}// namespace Star
