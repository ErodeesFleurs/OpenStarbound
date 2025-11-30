#pragma once

// ECS PlantDrop Adapter for OpenStarbound
// This adapter implements the PlantDrop entity using ECS components

#include "StarEntityAdapter.hpp"
#include "StarPlant.hpp"
#include "StarAssetPath.hpp"
#include "StarDrawable.hpp"

namespace Star {
namespace ECS {

// PlantDrop piece data for rendering
struct PlantDropPiece {
  AssetPath image;
  Vec2F offset;
  int segmentIdx = 0;
  Plant::PlantPieceKind kind = Plant::PlantPieceKind::None;
  bool flip = false;
};

// PlantDrop-specific component
struct PlantDropDataComponent {
  List<PlantDropPiece> pieces;
  String description;
  
  // Physics parameters
  float rotationRate = 0.0f;
  float rotationFallThreshold = 0.0f;
  float rotationCap = 0.0f;
  
  // State
  float time = 5000.0f;
  bool master = false;
  bool firstTick = true;
  bool spawnedDrops = false;
  bool spawnedDropEffects = false;
  
  // Configuration
  Json stemConfig;
  Json foliageConfig;
  Json saplingConfig;
  
  // Calculated bounds
  RectF boundingBox;
  RectF collisionRect;
};

// PlantDrop adapter that wraps ECS entity
class PlantDropAdapter : public EntityAdapter {
public:
  // Create from plant pieces
  static shared_ptr<PlantDropAdapter> create(
    World* ecsWorld,
    List<Plant::PlantPiece> pieces,
    Vec2F const& position,
    Vec2F const& strikeVector,
    String const& description,
    bool upsideDown,
    Json stemConfig,
    Json foliageConfig,
    Json saplingConfig,
    bool master,
    float random);
  
  // Create from network data
  static shared_ptr<PlantDropAdapter> createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules rules = {});
  
  // Construct from existing ECS entity
  PlantDropAdapter(World* ecsWorld, Entity ecsEntity);
  
  // Serialization
  ByteArray netStore(NetCompatibilityRules rules = {}) const;
  
  // Entity interface
  EntityType entityType() const override;
  
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;
  
  String description() const override;
  
  Vec2F position() const override;
  RectF metaBoundBox() const override;
  RectF collisionArea() const override;
  
  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;
  
  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;
  
  // PlantDrop-specific methods
  void setPosition(Vec2F const& position);
  void setVelocity(Vec2F const& velocity);
  
  RectF collisionRect() const;
  float rotation() const;

private:
  void setupComponents(
    List<Plant::PlantPiece> const& pieces,
    Vec2F const& position,
    Vec2F const& strikeVector,
    String const& description,
    bool upsideDown,
    Json stemConfig,
    Json foliageConfig,
    Json saplingConfig,
    bool master,
    float random);
  
  void particleForPlantPart(
    PlantDropPiece const& piece,
    String const& mode,
    Json const& mainConfig,
    RenderCallback* renderCallback);
  
  // Rotation stored separately (not in transform as it's physics-driven)
  float m_rotation = 0.0f;
};

using PlantDropAdapterPtr = shared_ptr<PlantDropAdapter>;

} // namespace ECS
} // namespace Star
