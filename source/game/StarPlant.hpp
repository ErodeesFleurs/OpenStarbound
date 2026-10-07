#pragma once
#include "StarIdMap.hpp"
#include "StarSet.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarList.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"
#include "StarAssetPath.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_damage;
import star.inspectable_entity;

namespace Star {

STAR_CLASS(RenderCallback);
STAR_CLASS(Plant);
struct TreeVariant;
struct GrassVariant;
struct BushVariant;

struct PlantExceptionTag {
  static constexpr char const* name() { return "PlantException"; }
};
using PlantException = StarError<PlantExceptionTag, StarException>;

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

  Plant(TreeVariant const& config, uint64_t seed);
  Plant(GrassVariant const& config, uint64_t seed);
  Plant(BushVariant const& config, uint64_t seed);
  Plant(Json const& diskStore);
  Plant(ByteArray const& netStore, NetCompatibilityRules rules = {});

  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {}) const;

  EntityType entityType() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;

  virtual String description() const override;

  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  Vec2F position() const override;
  RectF metaBoundBox() const override;

  bool ephemeral() const override;

  bool shouldDestroy() const override;

  // Forces the plant to check if it has been invalidly placed in some way, and
  // should die.  shouldDie does not, by default, do this expensive calculation
  bool checkBroken() override;

  // Base tile grid position
  Vec2I tilePosition() const override;
  void setTilePosition(Vec2I const& tilePosition) override;

  // Spaces this plant currently occupies
  List<Vec2I> spaces() const override;

  // Root blocks for this plant.
  List<Vec2I> roots() const override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;

  // Central root position
  Vec2I primaryRoot() const;
  // Plant hangs from the ceiling
  bool ceiling() const;

  List<PlantPiece> pieces() const;
  RectF interactiveBoundBox() const override;

private:
  Plant();

  void breakAtPosition(Vec2I const& position, Vec2F const& sourcePosition);
  Vec2I baseDamagePosition(List<Vec2I> const& positions) const;
  bool damagable() const;

  void scanSpacesAndRoots();
  List<PlantPiece> spawnFolliage(String const& key, String const& type);
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

}
