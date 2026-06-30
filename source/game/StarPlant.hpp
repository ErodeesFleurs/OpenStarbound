#pragma once

#include "StarSet.hpp"
#include "StarNetElementSystem.hpp"
#include "StarTileEntity.hpp"
#include "StarPlantDatabase.hpp"
#include "StarInspectableEntity.hpp"
#include "StarAssetPath.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

class RenderCallback;
class Plant;

struct PlantExceptionTag { static constexpr char const* typeName = "PlantException"; };
using PlantException = TypedException<StarException, PlantExceptionTag>;

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
    PlantPiece() = default;
    AssetPath imagePath;
    String image;
    Vec2U imageSize;
    Vec2F offset;
    int segmentIdx = 0;
    bool structuralSegment = false;
    PlantPieceKind kind = PlantPieceKind::None;
    RotationType rotationType = RotationType::DontRotate;
    float rotationOffset = 0.0f;
    Set<Vec2I> spaces;
    bool flip = false;
    // no need to serialize
    float zLevel = 0.0f;
  };

  Plant(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, TreeVariant const& config, uint64_t seed);
  Plant(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, GrassVariant const& config, uint64_t seed);
  Plant(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, BushVariant const& config, uint64_t seed);
  Plant(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& diskStore);
  Plant(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, ByteArray const& netStore, NetCompatibilityRules rules = {});

  [[nodiscard]] Json diskStore() const;
  [[nodiscard]] ByteArray netStore(NetCompatibilityRules rules = {}) const;

  [[nodiscard]] EntityType entityType() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;

  [[nodiscard]] String description() const override;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;

  [[nodiscard]] Vec2F position() const override;
  [[nodiscard]] RectF metaBoundBox() const override;

  [[nodiscard]] bool ephemeral() const override;

  [[nodiscard]] bool shouldDestroy() const override;

  // Forces the plant to check if it has been invalidly placed in some way, and
  // should die.  shouldDie does not, by default, do this expensive calculation
  [[nodiscard]] bool checkBroken() override;

  // Base tile grid position
  [[nodiscard]] Vec2I tilePosition() const override;
  void setTilePosition(Vec2I const& tilePosition) override;

  // Spaces this plant currently occupies
  [[nodiscard]] List<Vec2I> spaces() const override;

  // Root blocks for this plant.
  [[nodiscard]] List<Vec2I> roots() const override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  [[nodiscard]] bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;

  // Central root position
  [[nodiscard]] Vec2I primaryRoot() const;
  // Plant hangs from the ceiling
  [[nodiscard]] bool ceiling() const;

  [[nodiscard]] List<PlantPiece> pieces() const;
  [[nodiscard]] RectF interactiveBoundBox() const override;

private:
  Plant(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  void breakAtPosition(Vec2I const& position, Vec2F const& sourcePosition);
  [[nodiscard]] Vec2I baseDamagePosition(List<Vec2I> const& positions) const;
  [[nodiscard]] bool damagable() const;

  void scanSpacesAndRoots();
  [[nodiscard]] List<PlantPiece> spawnFolliage(String const& key, String const& type);
  [[nodiscard]] float branchRotation(float xPos, float rotoffset) const;
  void calcBoundBox();

  void readPieces(ByteArray pieces);
  [[nodiscard]] ByteArray writePieces() const;

  void readPiecesFromJson(Json const& pieces);
  [[nodiscard]] Json writePiecesToJson() const;

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
  AssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
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
