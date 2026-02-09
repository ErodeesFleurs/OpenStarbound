#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLiquidTypes.hpp"
#include "StarLruCache.hpp"
#include "StarMaterialTypes.hpp"
#include "StarRandom.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

using DungeonException = ExceptionDerived<"DungeonException">;

class DungeonDefinition;

class DungeonGeneratorWorldFacade {
public:
  virtual ~DungeonGeneratorWorldFacade() = default;

  // Hint that the given rectangular region is dungeon generated, and thus
  // would not receive the normal entity generation steps.
  virtual void markRegion(RectI const& region) = 0;
  // Mark the region as needing terrain to properly integrate with the dungeon
  virtual void markTerrain(PolyF const& region) = 0;
  // Mark the region as needing space to properly integrate with the dungeon
  virtual void markSpace(PolyF const& region) = 0;

  virtual void setForegroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant) = 0;
  virtual void setBackgroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant) = 0;
  virtual void setForegroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift) = 0;
  virtual void setBackgroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift) = 0;
  virtual void placeObject(Vec2I const& pos, String const& objectName, Star::Direction direction, Json const& parameters) = 0;
  virtual void placeVehicle(Vec2F const& pos, String const& vehicleName, Json const& parameters) = 0;
  virtual void placeSurfaceBiomeItems(Vec2I const& pos) = 0;
  virtual void placeBiomeTree(Vec2I const& pos) = 0;
  virtual void addDrop(Vec2F const& position, ItemDescriptor const& item) = 0;
  virtual void spawnNpc(Vec2F const& position, Json const& parameters) = 0;
  virtual void spawnStagehand(Vec2F const& position, Json const& definition) = 0;
  virtual void setLiquid(Vec2I const& pos, LiquidStore const& liquid) = 0;
  virtual void connectWireGroup(List<Vec2I> const& wireGroup) = 0;
  virtual void setTileProtection(DungeonId dungeonId, bool isProtected) = 0;
  virtual auto checkSolid(Vec2I const& position, TileLayer layer) -> bool = 0;
  virtual auto checkOpen(Vec2I const& position, TileLayer layer) -> bool = 0;
  virtual auto checkOceanLiquid(Vec2I const& position) -> bool = 0;
  virtual auto getDungeonIdAt(Vec2I const& position) -> DungeonId = 0;
  virtual void setDungeonIdAt(Vec2I const& position, DungeonId dungeonId) = 0;
  virtual void clearTileEntities(RectI const& bounds, Set<Vec2I> const& positions, bool clearAnchoredObjects) = 0;

  [[nodiscard]] virtual auto getWorldGeometry() const -> WorldGeometry = 0;

  virtual void setPlayerStart(Vec2F const& startPosition) = 0;
};

namespace Dungeon {

class Part;
struct Tile;

class DungeonGeneratorWriter {
public:
  DungeonGeneratorWriter(Ptr<DungeonGeneratorWorldFacade> facade, std::optional<int> terrainMarkingSurfaceLevel, std::optional<int> terrainSurfaceSpaceExtends);

  [[nodiscard]] auto wrapPosition(Vec2I const& pos) const -> Vec2I;

  void setMarkDungeonId(std::optional<DungeonId> markDungeonId = {});

  void requestLiquid(Vec2I const& pos, LiquidStore const& liquid);
  void setLiquid(Vec2I const& pos, LiquidStore const& liquid);
  void setForegroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant);
  void setBackgroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant);
  void setForegroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift);
  void setBackgroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift);
  auto needsForegroundBiomeMod(Vec2I const& position) -> bool;
  auto needsBackgroundBiomeMod(Vec2I const& position) -> bool;
  void placeObject(Vec2I const& position, String const& objectType, Direction direction, Json const& parameters);
  void placeVehicle(Vec2F const& pos, String const& vehicleName, Json const& parameters);
  void placeSurfaceBiomeItems(Vec2I const& pos);
  void placeBiomeTree(Vec2I const& pos);
  void addDrop(Vec2F const& position, ItemDescriptor const& item);
  void requestWire(Vec2I const& position, String const& wireGroup, bool partLocal);
  void spawnNpc(Vec2F const& pos, Json const& definition);
  void spawnStagehand(Vec2F const& pos, Json const& definition);
  void setPlayerStart(Vec2F const& startPosition);
  auto checkSolid(Vec2I position, TileLayer layer) -> bool;
  auto checkOpen(Vec2I position, TileLayer layer) -> bool;
  auto checkLiquid(Vec2I const& position) -> bool;
  auto otherDungeonPresent(Vec2I position) -> bool;
  void setDungeonId(Vec2I const& pos, DungeonId dungeonId);
  void markPosition(Vec2F const& pos);
  void markPosition(Vec2I const& pos);
  void finishPart();

  void clearTileEntities(RectI const& bounds, Set<Vec2I> const& positions, bool clearAnchoredObjects);

  void flushLiquid();
  void flush();

  [[nodiscard]] auto boundingBoxes() const -> List<RectI>;

  void reset();

private:
  struct Material {
    MaterialId material;
    MaterialHue hueshift;
    MaterialColorVariant colorVariant;
  };

  struct Mod {
    ModId mod;
    MaterialHue hueshift;
  };

  struct ObjectSettings {
    ObjectSettings() : direction() {}
    ObjectSettings(String objectName, Direction direction, Json parameters)
        : objectName(std::move(objectName)), direction(direction), parameters(std::move(parameters)) {}

    String objectName;
    Direction direction;
    Json parameters;
  };

  Ptr<DungeonGeneratorWorldFacade> m_facade;
  std::optional<int> m_terrainMarkingSurfaceLevel;
  std::optional<int> m_terrainSurfaceSpaceExtends;

  Map<Vec2I, LiquidStore> m_pendingLiquids;

  Map<Vec2I, Material> m_foregroundMaterial;
  Map<Vec2I, Material> m_backgroundMaterial;
  Map<Vec2I, Mod> m_foregroundMod;
  Map<Vec2I, Mod> m_backgroundMod;

  Map<Vec2I, ObjectSettings> m_objects;
  Map<Vec2F, std::pair<String, Json>> m_vehicles;
  Set<Vec2I> m_biomeTrees;
  Set<Vec2I> m_biomeItems;
  Map<Vec2F, ItemDescriptor> m_drops;
  Map<Vec2F, Json> m_npcs;
  Map<Vec2F, Json> m_stagehands;
  Map<Vec2I, DungeonId> m_dungeonIds;

  Map<Vec2I, LiquidStore> m_liquids;

  StringMap<Set<Vec2I>> m_globalWires;
  List<Set<Vec2I>> m_localWires;
  StringMap<Set<Vec2I>> m_openLocalWires;

  std::optional<DungeonId> m_markDungeonId;
  RectI m_currentBounds;
  List<RectI> m_boundingBoxes;
};

class Rule {
public:
  static auto parse(Json const& config) -> std::optional<ConstPtr<Rule>>;
  static auto readRules(Json const& rules) -> List<ConstPtr<Rule>>;

  virtual ~Rule() = default;

  virtual auto checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool;

  [[nodiscard]] virtual auto overdrawable() const -> bool;
  [[nodiscard]] virtual auto ignorePartMaximum() const -> bool;
  [[nodiscard]] virtual auto allowSpawnCount(int currentCount) const -> bool;

  [[nodiscard]] virtual auto doesNotConnectToPart(String const& name) const -> bool;
  [[nodiscard]] virtual auto checkPartCombinationsAllowed(StringMap<int> const& placementCounter) const -> bool;

  [[nodiscard]] virtual auto requiresOpen() const -> bool;
  [[nodiscard]] virtual auto requiresSolid() const -> bool;
  [[nodiscard]] virtual auto requiresLiquid() const -> bool;

protected:
  Rule() = default;
};

class WorldGenMustContainAirRule : public Rule {
public:
  WorldGenMustContainAirRule(TileLayer layer) : layer(layer) {}

  auto checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool override;

  [[nodiscard]] auto requiresOpen() const -> bool override {
    return true;
  }

  TileLayer layer;
};

class WorldGenMustContainSolidRule : public Rule {
public:
  WorldGenMustContainSolidRule(TileLayer layer) : layer(layer) {}

  auto checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool override;

  [[nodiscard]] auto requiresSolid() const -> bool override {
    return true;
  }

  TileLayer layer;
};

class WorldGenMustContainLiquidRule : public Rule {
public:
  WorldGenMustContainLiquidRule() = default;

  auto checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool override;

  [[nodiscard]] auto requiresLiquid() const -> bool override {
    return true;
  }
};

class WorldGenMustNotContainLiquidRule : public Rule {
public:
  WorldGenMustNotContainLiquidRule() = default;

  auto checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool override;
};

class AllowOverdrawingRule : public Rule {
public:
  AllowOverdrawingRule() = default;

  [[nodiscard]] auto overdrawable() const -> bool override {
    return true;
  }
};

class IgnorePartMaximumRule : public Rule {
public:
  IgnorePartMaximumRule() = default;

  [[nodiscard]] auto ignorePartMaximum() const -> bool override {
    return true;
  }
};

class MaxSpawnCountRule : public Rule {
public:
  MaxSpawnCountRule(Json const& rule) {
    m_maxCount = rule.toArray()[1].toArray()[0].toInt();
  }

  [[nodiscard]] auto allowSpawnCount(int currentCount) const -> bool override {
    return currentCount < m_maxCount;
  }

private:
  int m_maxCount;
};

class DoNotConnectToPartRule : public Rule {
public:
  DoNotConnectToPartRule(Json const& rule) {
    for (auto entry : rule.toArray()[1].toArray())
      m_partNames.add(entry.toString());
  }

  [[nodiscard]] auto doesNotConnectToPart(String const& name) const -> bool override;

private:
  StringSet m_partNames;
};

class DoNotCombineWithRule : public Rule {
public:
  DoNotCombineWithRule(Json const& rule) {
    for (auto part : rule.toArray()[1].toArray())
      m_parts.add(part.toString());
  }

  [[nodiscard]] auto checkPartCombinationsAllowed(StringMap<int> const& placementCounter) const -> bool override {
    for (auto part : m_parts) {
      if (placementCounter.contains(part) && (placementCounter.get(part) > 0))
        return false;
    }
    return true;
  }

private:
  StringSet m_parts;
};

enum class Phase {
  ClearPhase,
  WallPhase,
  ModsPhase,
  ObjectPhase,
  BiomeTreesPhase,
  BiomeItemsPhase,
  WirePhase,
  ItemPhase,
  NpcPhase,
  DungeonIdPhase
};

class Brush {
public:
  static auto parse(Json const& brush) -> ConstPtr<Brush>;
  static auto readBrushes(Json const& brushes) -> List<ConstPtr<Brush>>;

  virtual ~Brush() = default;

  virtual void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const = 0;

protected:
  Brush() = default;
};

class RandomBrush : public Brush {
public:
  RandomBrush(Json const& brush);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  List<ConstPtr<Brush>> m_brushes;
  std::int64_t m_seed;
};

class ClearBrush : public Brush {
public:
  ClearBrush() = default;

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;
};

class FrontBrush : public Brush {
public:
  FrontBrush(String const& material, std::optional<String> mod, std::optional<float> hueshift, std::optional<float> modhueshift, std::optional<MaterialColorVariant> colorVariant);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  String m_material;
  MaterialHue m_materialHue;
  MaterialColorVariant m_materialColorVariant;
  std::optional<String> m_mod;
  MaterialHue m_modHue;
};

class BackBrush : public Brush {
public:
  BackBrush(String const& material, std::optional<String> mod, std::optional<float> hueshift, std::optional<float> modhueshift, std::optional<MaterialColorVariant> colorVariant);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  String m_material;
  MaterialHue m_materialHue;
  MaterialColorVariant m_materialColorVariant;
  std::optional<String> m_mod;
  MaterialHue m_modHue;
};

class ObjectBrush : public Brush {
public:
  ObjectBrush(String const& object, Star::Direction direction, Json const& parameters);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  String m_object;
  Star::Direction m_direction;
  Json m_parameters;
};

class VehicleBrush : public Brush {
public:
  VehicleBrush(String const& vehicle, Json const& parameters);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  String m_vehicle;
  Json m_parameters;
};

class BiomeItemsBrush : public Brush {
public:
  BiomeItemsBrush() = default;

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;
};

class BiomeTreeBrush : public Brush {
public:
  BiomeTreeBrush() = default;

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;
};

class ItemBrush : public Brush {
public:
  ItemBrush(ItemDescriptor const& item);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  ItemDescriptor m_item;
};

class NpcBrush : public Brush {
public:
  NpcBrush(Json const& brush);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  Json m_npc;
};

class StagehandBrush : public Brush {
public:
  StagehandBrush(Json const& definition);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  Json m_definition;
};

class DungeonIdBrush : public Brush {
public:
  DungeonIdBrush(DungeonId dungeonId);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  DungeonId m_dungeonId;
};

class SurfaceBrush : public Brush {
public:
  SurfaceBrush(std::optional<int> variant, std::optional<String> mod);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  int m_variant;
  std::optional<String> m_mod;
};

class SurfaceBackgroundBrush : public Brush {
public:
  SurfaceBackgroundBrush(std::optional<int> variant, std::optional<String> mod);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  int m_variant;
  std::optional<String> m_mod;
};

class LiquidBrush : public Brush {
public:
  LiquidBrush(String const& liquidName, float quantity, bool source);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  String m_liquid;
  float m_quantity;
  bool m_source;
};

class WireBrush : public Brush {
public:
  WireBrush(String wireGroup, bool partLocal) : m_wireGroup(std::move(wireGroup)), m_partLocal(partLocal) {}

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  String m_wireGroup;
  bool m_partLocal;
};

class PlayerStartBrush : public Brush {
public:
  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;
};

// InvalidBrush reports an error when it is painted. This brush is used on
// tiles
// that represent objects that have been removed from the game.
class InvalidBrush : public Brush {
public:
  InvalidBrush(std::optional<String> nameHint);

  void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

private:
  std::optional<String> m_nameHint;
};

enum class Direction : std::uint8_t {
  Left = 0,
  Right = 1,
  Up = 2,
  Down = 3,
  Unknown = 4,
  Any = 5
};
extern EnumMap<Dungeon::Direction> const DungeonDirectionNames;

class Connector {
public:
  Connector(Part* part, String value, bool forwardOnly, Direction direction, Vec2I offset);

  [[nodiscard]] auto connectsTo(ConstPtr<Connector> connector) const -> bool;

  [[nodiscard]] auto value() const -> String;
  [[nodiscard]] auto positionAdjustment() const -> Vec2I;
  [[nodiscard]] auto part() const -> Part*;
  [[nodiscard]] auto offset() const -> Vec2I;

private:
  String m_value;
  bool m_forwardOnly;
  Direction m_direction;
  Vec2I m_offset;
  Part* m_part;
};

using TileCallback = std::function<bool(Vec2I, Tile const&)>;

class PartReader {
public:
  virtual ~PartReader() = default;

  virtual void readAsset(String const& asset) = 0;

  // Returns the dimensions of the part
  [[nodiscard]] virtual auto size() const -> Vec2U = 0;

  // Iterate over every tile in every layer of the part.
  // The callback receives the position of the tile (within the part), and
  // the tile at that position.
  // The callback can return true to exit from the loop early.
  virtual void forEachTile(TileCallback const& callback) const = 0;

  // Calls the callback with only the tiles at the given position.
  virtual void forEachTileAt(Vec2I pos, TileCallback const& callback) const = 0;

protected:
  PartReader() = default;
};

class Part {
public:
  Part(DungeonDefinition* dungeon, Json const& part, Ptr<PartReader> reader);

  [[nodiscard]] auto name() const -> String const&;
  [[nodiscard]] auto size() const -> Vec2U;
  [[nodiscard]] auto anchorPoint() const -> Vec2I;
  [[nodiscard]] auto chance() const -> float;
  [[nodiscard]] auto markDungeonId() const -> bool;
  [[nodiscard]] auto minimumThreatLevel() const -> std::optional<float>;
  [[nodiscard]] auto maximumThreatLevel() const -> std::optional<float>;
  [[nodiscard]] auto clearAnchoredObjects() const -> bool;
  [[nodiscard]] auto placementLevelConstraint() const -> int;
  [[nodiscard]] auto ignoresPartMaximum() const -> bool;
  [[nodiscard]] auto allowsPlacement(int currentPlacementCount) const -> bool;
  [[nodiscard]] auto connections() const -> List<ConstPtr<Connector>> const&;
  auto doesNotConnectTo(Part* part) const -> bool;
  [[nodiscard]] auto checkPartCombinationsAllowed(StringMap<int> const& placementCounts) const -> bool;
  auto collidesWithPlaces(Vec2I pos, Set<Vec2I>& places) const -> bool;

  auto canPlace(Vec2I pos, DungeonGeneratorWriter* writer) const -> bool;

  void place(Vec2I pos, Set<Vec2I> const& places, DungeonGeneratorWriter* writer) const;

  void forEachTile(TileCallback const& callback) const;

private:
  void placePhase(Vec2I pos, Phase phase, Set<Vec2I> const& places, DungeonGeneratorWriter* writer) const;

  [[nodiscard]] auto tileUsesPlaces(Vec2I pos) const -> bool;
  [[nodiscard]] auto pickByEdge(Vec2I position, Vec2U size) const -> Direction;
  [[nodiscard]] auto pickByNeighbours(Vec2I pos) const -> Direction;
  void scanConnectors();
  void scanAnchor();

  ConstPtr<PartReader> m_reader;

  String m_name;
  List<ConstPtr<Rule>> m_rules;
  DungeonDefinition* m_dungeon;
  List<ConstPtr<Connector>> m_connections;
  Vec2I m_anchorPoint;
  bool m_overrideAllowAlways;
  std::optional<float> m_minimumThreatLevel;
  std::optional<float> m_maximumThreatLevel;
  bool m_clearAnchoredObjects;
  Vec2U m_size;
  float m_chance;
  bool m_markDungeonId;
};

struct TileConnector {
  String value;
  bool forwardOnly;
  Direction direction = Direction::Unknown;
};

struct Tile {
  auto canPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool;
  void place(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const;
  [[nodiscard]] auto usesPlaces() const -> bool;
  [[nodiscard]] auto modifiesPlaces() const -> bool;
  [[nodiscard]] auto collidesWithPlaces() const -> bool;
  [[nodiscard]] auto requiresOpen() const -> bool;
  [[nodiscard]] auto requiresSolid() const -> bool;
  [[nodiscard]] auto requiresLiquid() const -> bool;

  List<ConstPtr<Brush>> brushes;
  List<ConstPtr<Rule>> rules;
  std::optional<TileConnector> connector;
};
}// namespace Dungeon

class DungeonDefinition {
public:
  DungeonDefinition(JsonObject const& definition, String const& directory);

  [[nodiscard]] auto metadata() const -> JsonObject;
  [[nodiscard]] auto directory() const -> String;
  [[nodiscard]] auto name() const -> String;
  [[nodiscard]] auto displayName() const -> String;
  [[nodiscard]] auto isProtected() const -> bool;
  [[nodiscard]] auto gravity() const -> std::optional<float>;
  [[nodiscard]] auto breathable() const -> std::optional<bool>;
  [[nodiscard]] auto parts() const -> StringMap<ConstPtr<Dungeon::Part>> const&;

  [[nodiscard]] auto anchors() const -> List<String> const&;
  [[nodiscard]] auto optTileset() const -> std::optional<Json> const&;
  [[nodiscard]] auto maxParts() const -> int;
  [[nodiscard]] auto maxRadius() const -> int;
  [[nodiscard]] auto extendSurfaceFreeSpace() const -> int;

  [[nodiscard]] auto metaData() const -> JsonObject;

private:
  JsonObject m_metadata;
  String m_directory;
  String m_name;
  String m_displayName;
  String m_species;
  bool m_isProtected;
  List<ConstPtr<Dungeon::Rule>> m_rules;
  StringMap<ConstPtr<Dungeon::Part>> m_parts;
  List<String> m_anchors;
  std::optional<Json> m_tileset;

  int m_maxRadius;
  int m_maxParts;
  int m_extendSurfaceFreeSpace;

  std::optional<float> m_gravity;
  std::optional<bool> m_breathable;
};

class DungeonDefinitions {
public:
  DungeonDefinitions();

  auto get(String const& name) const -> ConstPtr<DungeonDefinition>;
  auto getMetadata(String const& name) const -> JsonObject;

private:
  static auto readDefinition(String const& path) -> Ptr<DungeonDefinition>;

  StringMap<String> m_paths;
  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, Ptr<DungeonDefinition>> m_definitionCache;
};

class DungeonGenerator {
public:
  DungeonGenerator(String const& dungeonName, std::uint64_t seed, float threatLevel, std::optional<DungeonId> dungeonId);

  auto generate(Ptr<DungeonGeneratorWorldFacade> facade, Vec2I position, bool markSurfaceAndTerrain, bool forcePlacement) -> std::optional<std::pair<List<RectI>, Set<Vec2I>>>;

  auto buildDungeon(ConstPtr<Dungeon::Part> anchor, Vec2I pos, Dungeon::DungeonGeneratorWriter* writer, bool forcePlacement) -> std::pair<List<RectI>, Set<Vec2I>>;
  auto pickAnchor() -> ConstPtr<Dungeon::Part>;
  [[nodiscard]] auto findConnectablePart(ConstPtr<Dungeon::Connector> connector) const -> List<ConstPtr<Dungeon::Connector>>;

  [[nodiscard]] auto definition() const -> ConstPtr<DungeonDefinition>;

private:
  ConstPtr<DungeonDefinition> m_def;

  RandomSource m_rand;
  float m_threatLevel;
  std::optional<DungeonId> m_dungeonId;
};

}// namespace Star
