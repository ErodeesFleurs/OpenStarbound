#pragma once

#include "StarImage.hpp"
#include "StarItemDescriptor.hpp"
#include "StarWorldGeometry.hpp"
#include "StarGameTypes.hpp"
#include "StarRandom.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"
#include "StarLruCache.hpp"
#include "StarAssets.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarLiquidsDatabase.hpp"

namespace Star {

struct DungeonExceptionTag { static constexpr char const* typeName = "DungeonException"; };
using DungeonException = TypedException<StarException, DungeonExceptionTag>;

class DungeonGeneratorWorldFacade;
using DungeonGeneratorWorldFacadePtr = SharedPtr<DungeonGeneratorWorldFacade>;
class DungeonDefinition;
using DungeonDefinitionPtr = SharedPtr<DungeonDefinition>;
using DungeonDefinitionConstPtr = SharedPtr<DungeonDefinition const>;
class DungeonDefinitions;
class TilesetDatabase;
using TilesetDatabaseConstPtr = SharedPtr<TilesetDatabase const>;
using DungeonDefinitionsPtr = SharedPtr<DungeonDefinitions>;
using DungeonDefinitionsConstPtr = SharedPtr<DungeonDefinitions const>;

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
  [[nodiscard]] virtual bool checkSolid(Vec2I const& position, TileLayer layer) = 0;
  [[nodiscard]] virtual bool checkOpen(Vec2I const& position, TileLayer layer) = 0;
  [[nodiscard]] virtual bool checkOceanLiquid(Vec2I const& position) = 0;
  [[nodiscard]] virtual DungeonId getDungeonIdAt(Vec2I const& position) = 0;
  virtual void setDungeonIdAt(Vec2I const& position, DungeonId dungeonId) = 0;
  virtual void clearTileEntities(RectI const& bounds, Set<Vec2I> const& positions, bool clearAnchoredObjects) = 0;

  [[nodiscard]] virtual WorldGeometry getWorldGeometry() const = 0;
  [[nodiscard]] virtual MaterialDatabaseConstPtr materialDatabase() const = 0;
  [[nodiscard]] virtual LiquidsDatabaseConstPtr liquidsDatabase() const = 0;

  virtual void setPlayerStart(Vec2F const& startPosition) = 0;
};

namespace Dungeon {
  class DungeonGeneratorWriter;
  class PartReader;
  using PartReaderPtr = SharedPtr<PartReader>;
  using PartReaderConstPtr = SharedPtr<PartReader const>;
  class Part;
  using PartConstPtr = SharedPtr<Part const>;
  class Rule;
  using RuleConstPtr = SharedPtr<Rule const>;
  class Brush;
  using BrushConstPtr = SharedPtr<Brush const>;
  struct Tile;
  class Connector;
  using ConnectorConstPtr = SharedPtr<Connector const>;

  class DungeonGeneratorWriter {
  public:
    DungeonGeneratorWriter(DungeonGeneratorWorldFacadePtr facade, Maybe<int> terrainMarkingSurfaceLevel, Maybe<int> terrainSurfaceSpaceExtends);

    [[nodiscard]] Vec2I wrapPosition(Vec2I const& pos) const;
    [[nodiscard]] MaterialId materialId(String const& materialName) const;
    [[nodiscard]] ModId modId(String const& modName) const;
    [[nodiscard]] CollisionKind materialCollisionKind(MaterialId material) const;
    [[nodiscard]] LiquidId liquidId(String const& liquidName) const;

    void setMarkDungeonId(Maybe<DungeonId> markDungeonId = {});

    void requestLiquid(Vec2I const& pos, LiquidStore const& liquid);
    void setLiquid(Vec2I const& pos, LiquidStore const& liquid);
    void setForegroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant);
    void setBackgroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant);
    void setForegroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift);
    void setBackgroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift);
    [[nodiscard]] bool needsForegroundBiomeMod(Vec2I const& position);
    [[nodiscard]] bool needsBackgroundBiomeMod(Vec2I const& position);
    void placeObject(Vec2I const& position, String const& objectType, Direction direction, Json const& parameters);
    void placeVehicle(Vec2F const& pos, String const& vehicleName, Json const& parameters);
    void placeSurfaceBiomeItems(Vec2I const& pos);
    void placeBiomeTree(Vec2I const& pos);
    void addDrop(Vec2F const& position, ItemDescriptor const& item);
    void requestWire(Vec2I const& position, String const& wireGroup, bool partLocal);
    void spawnNpc(Vec2F const& pos, Json const& definition);
    void spawnStagehand(Vec2F const& pos, Json const& definition);
    void setPlayerStart(Vec2F const& startPosition);
    [[nodiscard]] bool checkSolid(Vec2I position, TileLayer layer);
    [[nodiscard]] bool checkOpen(Vec2I position, TileLayer layer);
    [[nodiscard]] bool checkLiquid(Vec2I const& position);
    [[nodiscard]] bool otherDungeonPresent(Vec2I position);
    void setDungeonId(Vec2I const& pos, DungeonId dungeonId);
    void markPosition(Vec2F const& pos);
    void markPosition(Vec2I const& pos);
    void finishPart();

    void clearTileEntities(RectI const& bounds, Set<Vec2I> const& positions, bool clearAnchoredObjects);

    void flushLiquid();
    void flush();

    [[nodiscard]] List<RectI> boundingBoxes() const;

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
      ObjectSettings() = default;
      ObjectSettings(String const& objectName, Direction direction, Json const& parameters)
        : objectName(objectName), direction(direction), parameters(parameters) {}

      String objectName;
      Direction direction{};
      Json parameters;
    };

    DungeonGeneratorWorldFacadePtr m_facade;
    MaterialDatabaseConstPtr m_materialDatabase;
    LiquidsDatabaseConstPtr m_liquidsDatabase;
    Maybe<int> m_terrainMarkingSurfaceLevel;
    Maybe<int> m_terrainSurfaceSpaceExtends;

    Map<Vec2I, LiquidStore> m_pendingLiquids;

    Map<Vec2I, Material> m_foregroundMaterial;
    Map<Vec2I, Material> m_backgroundMaterial;
    Map<Vec2I, Mod> m_foregroundMod;
    Map<Vec2I, Mod> m_backgroundMod;

    Map<Vec2I, ObjectSettings> m_objects;
    Map<Vec2F, pair<String, Json>> m_vehicles;
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

    Maybe<DungeonId> m_markDungeonId;
    RectI m_currentBounds;
    List<RectI> m_boundingBoxes;
  };

  class Rule {
  public:
    [[nodiscard]] static Maybe<RuleConstPtr> parse(Json const& rule);
    [[nodiscard]] static List<RuleConstPtr> readRules(Json const& rules);

    virtual ~Rule() = default;

    [[nodiscard]] virtual bool checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const;

    [[nodiscard]] virtual bool overdrawable() const;
    [[nodiscard]] virtual bool ignorePartMaximum() const;
    [[nodiscard]] virtual bool allowSpawnCount(int currentCount) const;

    [[nodiscard]] virtual bool doesNotConnectToPart(String const& name) const;
    [[nodiscard]] virtual bool checkPartCombinationsAllowed(StringMap<int> const& placementCounter) const;

    [[nodiscard]] virtual bool requiresOpen() const;
    [[nodiscard]] virtual bool requiresSolid() const;
    [[nodiscard]] virtual bool requiresLiquid() const;

  protected:
    Rule() = default;
  };

  class WorldGenMustContainAirRule : public Rule {
  public:
    WorldGenMustContainAirRule(TileLayer layer) : layer(layer) {}

    [[nodiscard]] bool checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const override;

    bool requiresOpen() const override {
      return true;
    }

    TileLayer layer;
  };

  class WorldGenMustContainSolidRule : public Rule {
  public:
    WorldGenMustContainSolidRule(TileLayer layer) : layer(layer) {}

    [[nodiscard]] bool checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const override;

    bool requiresSolid() const override {
      return true;
    }

    TileLayer layer;
  };

  class WorldGenMustContainLiquidRule : public Rule {
  public:
    WorldGenMustContainLiquidRule() = default;

    [[nodiscard]] bool checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const override;
    
    bool requiresLiquid() const override {
      return true;
    }
  };

  class WorldGenMustNotContainLiquidRule : public Rule {
  public:
    WorldGenMustNotContainLiquidRule() = default;

    [[nodiscard]] bool checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const override;
  };

  class AllowOverdrawingRule : public Rule {
  public:
    AllowOverdrawingRule() = default;

    bool overdrawable() const override {
      return true;
    }
  };

  class IgnorePartMaximumRule : public Rule {
  public:
    IgnorePartMaximumRule() = default;

    bool ignorePartMaximum() const override {
      return true;
    }
  };

  class MaxSpawnCountRule : public Rule {
  public:
    MaxSpawnCountRule(Json const& rule) {
      m_maxCount = rule.toArray()[1].toArray()[0].toInt();
    }

    bool allowSpawnCount(int currentCount) const override {
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

    bool doesNotConnectToPart(String const& name) const override {
      return m_partNames.contains(name);
    }

  private:
    StringSet m_partNames;
  };

  class DoNotCombineWithRule : public Rule {
  public:
    DoNotCombineWithRule(Json const& rule) {
      for (auto part : rule.toArray()[1].toArray())
        m_parts.add(part.toString());
    }

    bool checkPartCombinationsAllowed(StringMap<int> const& placementCounter) const override {
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
    [[nodiscard]] static BrushConstPtr parse(Json const& brush);
    [[nodiscard]] static List<BrushConstPtr> readBrushes(Json const& brushes);

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
    List<BrushConstPtr> m_brushes;
    int64_t m_seed;
  };

  class ClearBrush : public Brush {
  public:
    ClearBrush() = default;

    void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;
  };

  class FrontBrush : public Brush {
  public:
    FrontBrush(String const& material, Maybe<String> mod, Maybe<float> hueshift, Maybe<float> modhueshift, Maybe<MaterialColorVariant> colorVariant);

    void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

  private:
    String m_material;
    MaterialHue m_materialHue;
    MaterialColorVariant m_materialColorVariant;
    Maybe<String> m_mod;
    MaterialHue m_modHue;
  };

  class BackBrush : public Brush {
  public:
    BackBrush(String const& material, Maybe<String> mod, Maybe<float> hueshift, Maybe<float> modhueshift, Maybe<MaterialColorVariant> colorVariant);

    void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

  private:
    String m_material;
    MaterialHue m_materialHue;
    MaterialColorVariant m_materialColorVariant;
    Maybe<String> m_mod;
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
    SurfaceBrush(Maybe<int> variant, Maybe<String> mod);

    void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

  private:
    int m_variant;
    Maybe<String> m_mod;
  };

  class SurfaceBackgroundBrush : public Brush {
  public:
    SurfaceBackgroundBrush(Maybe<int> variant, Maybe<String> mod);

    void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

  private:
    int m_variant;
    Maybe<String> m_mod;
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
    WireBrush(String wireGroup, bool partLocal) : m_wireGroup(wireGroup), m_partLocal(partLocal) {}

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
    InvalidBrush(Maybe<String> nameHint);

    void paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const override;

  private:
    Maybe<String> m_nameHint;
  };

  enum class Direction : uint8_t {
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
    Connector(Part& part, String value, bool forwardOnly, Direction direction, Vec2I offset);

    [[nodiscard]] bool connectsTo(Connector const& connector) const;

    [[nodiscard]] String value() const;
    [[nodiscard]] Vec2I positionAdjustment() const;
    [[nodiscard]] Part const& part() const;
    [[nodiscard]] Vec2I offset() const;

  private:
    String m_value;
    bool m_forwardOnly;
    Direction m_direction;
    Vec2I m_offset;
    Part& m_part;
  };

  using TileCallback = function<bool(Vec2I, Tile const&)>;

  class PartReader {
  public:
    virtual ~PartReader() = default;

    virtual void readAsset(String const& asset) = 0;

    // Returns the dimensions of the part
    [[nodiscard]] virtual Vec2U size() const = 0;

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
    Part(DungeonDefinition& dungeon, Json const& part, PartReaderPtr reader);

    [[nodiscard]] String const& name() const;
    [[nodiscard]] Vec2U size() const;
    [[nodiscard]] Vec2I anchorPoint() const;
    [[nodiscard]] float chance() const;
    [[nodiscard]] bool markDungeonId() const;
    [[nodiscard]] Maybe<float> minimumThreatLevel() const;
    [[nodiscard]] Maybe<float> maximumThreatLevel() const;
    [[nodiscard]] bool clearAnchoredObjects() const;
    [[nodiscard]] int placementLevelConstraint() const;
    [[nodiscard]] bool ignoresPartMaximum() const;
    [[nodiscard]] bool allowsPlacement(int currentPlacementCount) const;
    [[nodiscard]] List<ConnectorConstPtr> const& connections() const;
    [[nodiscard]] bool doesNotConnectTo(Part const& part) const;
    [[nodiscard]] bool checkPartCombinationsAllowed(StringMap<int> const& placementCounts) const;
    [[nodiscard]] bool collidesWithPlaces(Vec2I pos, Set<Vec2I>& places) const;

    [[nodiscard]] bool canPlace(Vec2I pos, DungeonGeneratorWriter* writer) const;

    void place(Vec2I pos, Set<Vec2I> const& places, DungeonGeneratorWriter* writer) const;

    void forEachTile(TileCallback const& callback) const;

  private:
    void placePhase(Vec2I pos, Phase phase, Set<Vec2I> const& places, DungeonGeneratorWriter* writer) const;

    [[nodiscard]] bool tileUsesPlaces(Vec2I pos) const;
    [[nodiscard]] Direction pickByEdge(Vec2I position, Vec2U size) const;
    [[nodiscard]] Direction pickByNeighbours(Vec2I pos) const;
    void scanConnectors();
    void scanAnchor();

    PartReaderConstPtr m_reader;

    String m_name;
    List<RuleConstPtr> m_rules;
    List<ConnectorConstPtr> m_connections;
    Vec2I m_anchorPoint;
    bool m_overrideAllowAlways;
    Maybe<float> m_minimumThreatLevel;
    Maybe<float> m_maximumThreatLevel;
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
    [[nodiscard]] bool canPlace(Vec2I position, DungeonGeneratorWriter* writer) const;
    void place(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const;
    [[nodiscard]] bool usesPlaces() const;
    [[nodiscard]] bool modifiesPlaces() const;
    [[nodiscard]] bool collidesWithPlaces() const;
    [[nodiscard]] bool requiresOpen() const;
    [[nodiscard]] bool requiresSolid() const;
    [[nodiscard]] bool requiresLiquid() const;

    List<BrushConstPtr> brushes;
    List<RuleConstPtr> rules;
    Maybe<TileConnector> connector;
  };
}

class DungeonDefinition {
public:
  DungeonDefinition(AssetsConstPtr assets, TilesetDatabaseConstPtr tilesetDatabase, JsonObject const& definition, String const& directory);

  [[nodiscard]] JsonObject metadata() const;
  [[nodiscard]] String directory() const;
  [[nodiscard]] String name() const;
  [[nodiscard]] String displayName() const;
  [[nodiscard]] bool isProtected() const;
  [[nodiscard]] Maybe<float> gravity() const;
  [[nodiscard]] Maybe<bool> breathable() const;
  StringMap<Dungeon::PartConstPtr> const& parts() const;

  [[nodiscard]] List<String> const& anchors() const;
  [[nodiscard]] Maybe<Json> const& optTileset() const;
  [[nodiscard]] int maxParts() const;
  [[nodiscard]] int maxRadius() const;
  [[nodiscard]] int extendSurfaceFreeSpace() const;

  [[nodiscard]] JsonObject metaData() const;

private:
  JsonObject m_metadata;
  String m_directory;
  String m_name;
  String m_displayName;
  String m_species;
  bool m_isProtected;
  List<Dungeon::RuleConstPtr> m_rules;
  StringMap<Dungeon::PartConstPtr> m_parts;
  List<String> m_anchors;
  Maybe<Json> m_tileset;

  int m_maxRadius;
  int m_maxParts;
  int m_extendSurfaceFreeSpace;

  Maybe<float> m_gravity;
  Maybe<bool> m_breathable;
};

class DungeonDefinitions {
public:
  DungeonDefinitions(AssetsConstPtr assets, TilesetDatabaseConstPtr tilesetDatabase);

  [[nodiscard]] DungeonDefinitionConstPtr get(String const& name) const;
  [[nodiscard]] JsonObject getMetadata(String const& name) const;

private:
  [[nodiscard]] DungeonDefinitionPtr readDefinition(String const& path) const;

  StringMap<String> m_paths;
  AssetsConstPtr m_assets;
  TilesetDatabaseConstPtr m_tilesetDatabase;
  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, DungeonDefinitionPtr> m_definitionCache;
};

class DungeonGenerator {
public:
  DungeonGenerator(DungeonDefinitionsConstPtr dungeonDefinitions, String const& dungeonName, uint64_t seed, float threatLevel, Maybe<DungeonId> dungeonId);

  [[nodiscard]] Maybe<pair<List<RectI>, Set<Vec2I>>> generate(DungeonGeneratorWorldFacadePtr facade, Vec2I position, bool markSurfaceAndTerrain, bool forcePlacement);

  [[nodiscard]] pair<List<RectI>, Set<Vec2I>> buildDungeon(Dungeon::PartConstPtr anchor, Vec2I pos, Dungeon::DungeonGeneratorWriter* writer, bool forcePlacement);
  Dungeon::PartConstPtr pickAnchor();
  List<Dungeon::ConnectorConstPtr> findConnectablePart(Dungeon::ConnectorConstPtr connector) const;

  [[nodiscard]] DungeonDefinitionConstPtr definition() const;

private:
  DungeonDefinitionConstPtr m_def;

  RandomSource m_rand;
  float m_threatLevel;
  Maybe<DungeonId> m_dungeonId;
};

}
