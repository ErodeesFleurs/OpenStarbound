#pragma once

// ECS Plant Adapter for OpenStarbound
// This adapter implements the Plant entity using ECS components
// Plants are TileEntities that represent trees, grass, bushes with wind animation

#include "StarEntityAdapter.hpp"
#include "StarPlantDatabase.hpp"
#include "StarTileEntity.hpp"
#include "StarSet.hpp"
#include "StarNetElementSystem.hpp"
#include "StarAssetPath.hpp"

namespace Star {
namespace ECS {

// Plant piece definition (same as original Plant::PlantPiece)
enum class PlantRotationType {
  DontRotate,
  RotateBranch,
  RotateLeaves,
  RotateCrownBranch,
  RotateCrownLeaves
};

enum class PlantPieceKind {
  None,
  Stem,
  Foliage
};

struct PlantPieceData {
  PlantPieceData();
  
  AssetPath imagePath;
  String image;
  Vec2U imageSize;
  Vec2F offset;
  int segmentIdx = 0;
  bool structuralSegment = false;
  PlantPieceKind kind = PlantPieceKind::None;
  PlantRotationType rotationType = PlantRotationType::DontRotate;
  float rotationOffset = 0.0f;
  Set<Vec2I> spaces;
  bool flip = false;
  float zLevel = 0.0f;
};

// Plant-specific component storing all plant state
struct PlantDataComponent {
  // Position and spaces
  Vec2I tilePosition;
  List<Vec2I> spaces;
  List<Vec2I> roots;
  RectI boundBox;
  
  // Configuration
  Json descriptions;
  bool ephemeral = false;
  bool ceiling = false;
  bool fallsWhenDead = false;
  
  // Drop configs
  Json stemDropConfig;
  Json foliageDropConfig;
  Json saplingDropConfig;
  
  // Visual pieces
  List<PlantPieceData> pieces;
  bool piecesUpdated = true;
  bool piecesScanned = false;
  
  // Animation state
  float windTime = 0.0f;
  float windLevel = 0.0f;
  
  // Damage state
  TileDamageParameters tileDamageParameters;
  EntityTileDamageStatus tileDamageStatus;
  float tileDamageX = 0.0f;
  float tileDamageY = 0.0f;
  bool tileDamageEventTrigger = false;
  bool tileDamageEvent = false;
  
  // State
  bool broken = false;
  RectF metaBoundBox;
};

// Tag component for identifying plants
struct PlantTag {};

// Plant adapter that wraps ECS entity to implement TileEntity interface
class PlantAdapter : public EntityAdapter, public virtual TileEntity {
public:
  static float const PlantScanThreshold;
  
  // Create from TreeVariant
  static shared_ptr<PlantAdapter> createTree(
    World* ecsWorld,
    TreeVariant const& config,
    uint64_t seed);
  
  // Create from GrassVariant
  static shared_ptr<PlantAdapter> createGrass(
    World* ecsWorld,
    GrassVariant const& config,
    uint64_t seed);
  
  // Create from BushVariant
  static shared_ptr<PlantAdapter> createBush(
    World* ecsWorld,
    BushVariant const& config,
    uint64_t seed);
  
  // Create from disk store
  static shared_ptr<PlantAdapter> createFromDiskStore(
    World* ecsWorld,
    Json const& diskStore);
  
  // Create from network data
  static shared_ptr<PlantAdapter> createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules rules = {});
  
  // Construct from existing ECS entity
  PlantAdapter(World* ecsWorld, Entity ecsEntity);
  
  // Serialization
  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {}) const;
  
  // Entity interface
  EntityType entityType() const override;
  
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  String description() const override;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;
  
  Vec2F position() const override;
  RectF metaBoundBox() const override;
  
  bool ephemeral() const override;
  
  bool shouldDestroy() const override;
  
  // TileEntity interface
  Vec2I tilePosition() const override;
  void setTilePosition(Vec2I const& tilePosition) override;
  List<Vec2I> spaces() const override;
  List<Vec2I> roots() const override;
  bool checkBroken() override;
  bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;
  RectF interactiveBoundBox() const override;
  
  // Plant-specific methods
  Vec2I primaryRoot() const;
  bool ceiling() const;
  List<PlantPieceData> pieces() const;
  
  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;

private:
  void setupTreePieces(TreeVariant const& config, uint64_t seed);
  void setupGrassPieces(GrassVariant const& config, uint64_t seed);
  void setupBushPieces(BushVariant const& config, uint64_t seed);
  
  void breakAtPosition(Vec2I const& position, Vec2F const& sourcePosition);
  Vec2I baseDamagePosition(List<Vec2I> const& positions) const;
  bool damagable() const;
  
  void scanSpacesAndRoots();
  float branchRotation(float xPos, float rotoffset) const;
  void calcBoundBox();
  
  void readPieces(ByteArray pieces);
  ByteArray writePieces() const;
  void readPiecesFromJson(Json const& pieces);
  Json writePiecesToJson() const;
  void validatePieces();
  
  void setupNetStates();
  void getNetStates();
  void setNetStates();
  
  // Network state
  NetElementTopGroup m_netGroup;
  NetElementBytes m_piecesNetState;
  NetElementFloat m_tileDamageXNetState;
  NetElementFloat m_tileDamageYNetState;
  NetElementEvent m_tileDamageEventNetState;
};

using PlantAdapterPtr = shared_ptr<PlantAdapter>;

} // namespace ECS
} // namespace Star
