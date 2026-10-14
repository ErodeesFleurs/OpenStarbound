module;
import std;
import star.string;
import star.map;
import star.variant;
import star.maybe;
import star.format;
import star.json;

import star.rect;
import star.bi_map;
import star.game_types;
import star.material_types;
import star.vector;
import star.tile_damage;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;

namespace Star {

struct MaterialRenderProfileExceptionTag {
  static constexpr char const* name() { return "MaterialRenderProfileException"; }
};
using MaterialRenderProfileException = StarError<MaterialRenderProfileExceptionTag, StarException>;

enum class MaterialJoinType : std::uint8_t { All, Any };
extern EnumMap<MaterialJoinType> const MaterialJoinTypeNames;

struct MaterialRule;
using MaterialRulePtr = std::shared_ptr<MaterialRule>;
using MaterialRuleConstPtr = std::shared_ptr<const MaterialRule>;
using MaterialRuleWeakPtr = std::weak_ptr<MaterialRule>;
using MaterialRuleConstWeakPtr = std::weak_ptr<const MaterialRule>;
using MaterialRuleUPtr = std::unique_ptr<MaterialRule>;
using MaterialRuleConstUPtr = std::unique_ptr<const MaterialRule>;

struct MaterialRule {
  struct RuleEmpty {};
  struct RuleConnects {};
  struct RuleShadows {};
  struct RuleEqualsSelf {
    bool matchHue;
  };

  struct RuleEqualsId {
    std::uint16_t id;
  };

  struct RulePropertyEquals {
    String propertyName;
    Json compare;
  };

  struct RuleEntry {
    MVariant<RuleEmpty, RuleConnects, RuleShadows, RuleEqualsSelf, RuleEqualsSelf, RuleEqualsId, RulePropertyEquals> rule;
    bool inverse;
  };

  MaterialJoinType join;
  List<RuleEntry> entries;
};
typedef StringMap<MaterialRuleConstPtr> RuleMap;

struct MaterialMatchPoint {
  Vec2I position;
  MaterialRuleConstPtr rule;
};

struct MaterialRenderPiece;
using MaterialRenderPiecePtr = std::shared_ptr<MaterialRenderPiece>;
using MaterialRenderPieceConstPtr = std::shared_ptr<const MaterialRenderPiece>;
using MaterialRenderPieceWeakPtr = std::weak_ptr<MaterialRenderPiece>;
using MaterialRenderPieceConstWeakPtr = std::weak_ptr<const MaterialRenderPiece>;
using MaterialRenderPieceUPtr = std::unique_ptr<MaterialRenderPiece>;
using MaterialRenderPieceConstUPtr = std::unique_ptr<const MaterialRenderPiece>;

struct MaterialRenderPiece {
  std::size_t pieceId;
  String texture;
  // Maps each MaterialColorVariant to a list of texture coordinates for each
  // random variant
  HashMap<MaterialColorVariant, List<RectF>> variants;
};

struct MaterialRenderMatch;
using MaterialRenderMatchPtr = std::shared_ptr<MaterialRenderMatch>;
using MaterialRenderMatchConstPtr = std::shared_ptr<const MaterialRenderMatch>;
using MaterialRenderMatchWeakPtr = std::weak_ptr<MaterialRenderMatch>;
using MaterialRenderMatchConstWeakPtr = std::weak_ptr<const MaterialRenderMatch>;
using MaterialRenderMatchUPtr = std::unique_ptr<MaterialRenderMatch>;
using MaterialRenderMatchConstUPtr = std::unique_ptr<const MaterialRenderMatch>;
typedef List<MaterialRenderMatchConstPtr> MaterialRenderMatchList;

struct MaterialRenderMatch {
  List<MaterialMatchPoint> matchPoints;
  MaterialJoinType matchJoin;

  // Positions here are in TilePixels
  List<std::pair<MaterialRenderPieceConstPtr, Vec2F>> resultingPieces;
  MaterialRenderMatchList subMatches;
  Maybe<TileLayer> requiredLayer;
  Maybe<bool> occlude;
  bool haltOnMatch;
  bool haltOnSubMatch;
};

typedef StringMap<MaterialRenderPieceConstPtr> PieceMap;
typedef StringMap<MaterialRenderMatchList> MatchMap;

// This is the maximum distance in either X or Y that material neighbor rules
// are limited to.  This can be used as a maximum limit on the "sphere of
// influence" that a tile can have on other tile's rendering.  A value of 1
// here means "1 away", so would be interpreted as a 3x3 block with the
// rendered tile in the center.
inline constexpr int MaterialRenderProfileMaxNeighborDistance = 2;

struct MaterialRenderProfile;
using MaterialRenderProfilePtr = std::shared_ptr<MaterialRenderProfile>;
using MaterialRenderProfileConstPtr = std::shared_ptr<const MaterialRenderProfile>;
using MaterialRenderProfileWeakPtr = std::weak_ptr<MaterialRenderProfile>;
using MaterialRenderProfileConstWeakPtr = std::weak_ptr<const MaterialRenderProfile>;
using MaterialRenderProfileUPtr = std::unique_ptr<MaterialRenderProfile>;
using MaterialRenderProfileConstUPtr = std::unique_ptr<const MaterialRenderProfile>;

struct MaterialRenderProfile {
  RuleMap rules;
  PieceMap pieces;
  MatchMap matches;

  String representativePiece;

  MaterialRenderMatchList mainMatchList;
  List<std::pair<String, Vec2F>> crackingFrames;
  List<std::pair<String, Vec2F>> protectedFrames;
  List<Directives> colorDirectives;
  Json ruleProperties;

  bool foregroundLightTransparent;
  bool backgroundLightTransparent;
  std::uint8_t colorVariants;
  bool occludesBehind;
  std::uint32_t zLevel;
  Vec3F radiantLight;

  // Get a single asset path for just a single piece of a material, with the
  // image cropped to the piece itself.
  String pieceImage(String const& pieceName,
      unsigned variant,
      MaterialColorVariant colorVariant = DefaultMaterialColorVariant,
      MaterialHue hueShift = MaterialHue()) const;

  // Get an overlay image for rendering damaged tiles, as well as the offset
  // for it in world coordinates.
  std::pair<String, Vec2F> const& damageImage(float damageLevel, TileDamageType damageType) const;
};

MaterialRenderProfile parseMaterialRenderProfile(Json const& spec, String const& relativePath = "");

}

export module star.material_render_profile;

export namespace Star {
  using ::Star::MaterialRenderProfileExceptionTag;
  using ::Star::MaterialRenderProfileException;
  using ::Star::MaterialJoinType;
  using ::Star::MaterialJoinTypeNames;
  using ::Star::MaterialRule;
  using ::Star::MaterialRulePtr;
  using ::Star::MaterialRuleConstPtr;
  using ::Star::MaterialRuleWeakPtr;
  using ::Star::MaterialRuleConstWeakPtr;
  using ::Star::MaterialRuleUPtr;
  using ::Star::MaterialRuleConstUPtr;
  using ::Star::RuleMap;
  using ::Star::MaterialMatchPoint;
  using ::Star::MaterialRenderPiece;
  using ::Star::MaterialRenderPiecePtr;
  using ::Star::MaterialRenderPieceConstPtr;
  using ::Star::MaterialRenderPieceWeakPtr;
  using ::Star::MaterialRenderPieceConstWeakPtr;
  using ::Star::MaterialRenderPieceUPtr;
  using ::Star::MaterialRenderPieceConstUPtr;
  using ::Star::MaterialRenderMatch;
  using ::Star::MaterialRenderMatchPtr;
  using ::Star::MaterialRenderMatchConstPtr;
  using ::Star::MaterialRenderMatchWeakPtr;
  using ::Star::MaterialRenderMatchConstWeakPtr;
  using ::Star::MaterialRenderMatchUPtr;
  using ::Star::MaterialRenderMatchConstUPtr;
  using ::Star::MaterialRenderMatchList;
  using ::Star::PieceMap;
  using ::Star::MatchMap;
  using ::Star::MaterialRenderProfileMaxNeighborDistance;
  using ::Star::MaterialRenderProfile;
  using ::Star::MaterialRenderProfilePtr;
  using ::Star::MaterialRenderProfileConstPtr;
  using ::Star::MaterialRenderProfileWeakPtr;
  using ::Star::MaterialRenderProfileConstWeakPtr;
  using ::Star::MaterialRenderProfileUPtr;
  using ::Star::MaterialRenderProfileConstUPtr;
  using ::Star::parseMaterialRenderProfile;
}
