#pragma once

#include "StarAssetPath.hpp"
#include "StarException.hpp"
#include "StarNetElementSystem.hpp"
#include "StarPlantDatabase.hpp"
#include "StarSet.hpp"
#include "StarTileEntity.hpp"

import std;

namespace Star {

using PlantException = ExceptionDerived<"PlantException">;

class Plant : public virtual TileEntity {
public:
  // TODO: For right now the space scan threshold is hard-coded, but should be
  // configurable in the future
  static float const PlantScanThreshold;

  enum RotationType {
    DontRotate,
    RotateBranch,
    RotateLeaves,
    RotateCrownBranch,
    RotateCrownLeaves
  };

  static EnumMap<RotationType> const RotationTypeNames;

  enum PlantPieceKind {
    None,
    Stem,
    Foliage
  };

  struct PlantPiece {
    PlantPiece();
    AssetPath imagePath;
    String image;
    Vec2U imageSize;
    Vec2F offset;
    int segmentIdx;
    bool structuralSegment;
    PlantPieceKind kind;
    RotationType rotationType;
    float rotationOffset;
    Set<Vec2I> spaces;
    bool flip;
    // no need to serialize
    float zLevel;
  };

  Plant(TreeVariant const& config, std::uint64_t seed);
  Plant(GrassVariant const& config, std::uint64_t seed);
  Plant(BushVariant const& config, std::uint64_t seed);
  Plant(Json const& diskStore);
  Plant(ByteArray const& netStore, NetCompatibilityRules rules = {});

  auto diskStore() const -> Json;
  auto netStore(NetCompatibilityRules rules = {}) const -> ByteArray;

  auto entityType() const -> EntityType override;

  void init(World* world, EntityId entityId, EntityMode mode) override;

  auto description() const -> String override;

  auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  auto position() const -> Vec2F override;
  auto metaBoundBox() const -> RectF override;

  auto ephemeral() const -> bool override;

  auto shouldDestroy() const -> bool override;

  // Forces the plant to check if it has been invalidly placed in some way, and
  // should die.  shouldDie does not, by default, do this expensive calculation
  auto checkBroken() -> bool override;

  // Base tile grid position
  auto tilePosition() const -> Vec2I override;
  void setTilePosition(Vec2I const& tilePosition) override;

  // Spaces this plant currently occupies
  auto spaces() const -> List<Vec2I> override;

  // Root blocks for this plant.
  auto roots() const -> List<Vec2I> override;

  void update(float dt, std::uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  auto damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) -> bool override;

  // Central root position
  auto primaryRoot() const -> Vec2I;
  // Plant hangs from the ceiling
  auto ceiling() const -> bool;

  auto pieces() const -> List<PlantPiece>;
  auto interactiveBoundBox() const -> RectF override;

private:
  Plant();

  void breakAtPosition(Vec2I const& position, Vec2F const& sourcePosition);
  auto baseDamagePosition(List<Vec2I> const& positions) const -> Vec2I;
  auto damagable() const -> bool;

  void scanSpacesAndRoots();
  auto spawnFolliage(String const& key, String const& type) -> List<PlantPiece>;
  auto branchRotation(float xPos, float rotoffset) const -> float;
  void calcBoundBox();

  void readPieces(ByteArray pieces);
  auto writePieces() const -> ByteArray;

  void readPiecesFromJson(Json const& pieces);
  auto writePiecesToJson() const -> Json;

  void validatePieces();

  void setupNetStates();
  void getNetStates();
  void setNetStates();

  Vec2I m_tilePosition;
  List<Vec2I> m_spaces;
  List<Vec2I> m_roots;
  RectI m_boundBox;

  Json m_descriptions;

  bool m_ephemeral;

  Json m_stemDropConfig;
  Json m_foliageDropConfig;
  Json m_saplingDropConfig;

  List<PlantPiece> m_pieces;
  bool m_piecesUpdated;

  bool m_ceiling;
  bool m_broken;
  bool m_fallsWhenDead;

  float m_windTime;
  float m_windLevel;

  RectF m_metaBoundBox;

  bool m_piecesScanned;

  TileDamageParameters m_tileDamageParameters;
  EntityTileDamageStatus m_tileDamageStatus;
  float m_tileDamageX;
  float m_tileDamageY;
  bool m_tileDamageEventTrigger;
  bool m_tileDamageEvent;

  NetElementTopGroup m_netGroup;
  NetElementBytes m_piecesNetState;
  NetElementFloat m_tileDamageXNetState;
  NetElementFloat m_tileDamageYNetState;
  NetElementEvent m_tileDamageEventNetState;
};

}// namespace Star
