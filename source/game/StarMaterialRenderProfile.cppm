module;

#include "StarRect.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"
import star.tile_damage;
#include "StarDirectives.hpp"

namespace Star {

struct MaterialRenderProfileExceptionTag {
  static constexpr char const* name() { return "MaterialRenderProfileException"; }
};
using MaterialRenderProfileException = StarError<MaterialRenderProfileExceptionTag, StarException>;

enum class MaterialJoinType : uint8_t { All, Any };
extern EnumMap<MaterialJoinType> const MaterialJoinTypeNames;

STAR_STRUCT(MaterialRule);

struct MaterialRule {
  struct RuleEmpty {};
  struct RuleConnects {};
  struct RuleShadows {};
  struct RuleEqualsSelf {
    bool matchHue;
  };

  struct RuleEqualsId {
    uint16_t id;
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

STAR_STRUCT(MaterialRenderPiece);

struct MaterialRenderPiece {
  size_t pieceId;
  String texture;
  // Maps each MaterialColorVariant to a list of texture coordinates for each
  // random variant
  HashMap<MaterialColorVariant, List<RectF>> variants;
};

STAR_STRUCT(MaterialRenderMatch);
typedef List<MaterialRenderMatchConstPtr> MaterialRenderMatchList;

struct MaterialRenderMatch {
  List<MaterialMatchPoint> matchPoints;
  MaterialJoinType matchJoin;

  // Positions here are in TilePixels
  List<pair<MaterialRenderPieceConstPtr, Vec2F>> resultingPieces;
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

STAR_STRUCT(MaterialRenderProfile);

struct MaterialRenderProfile {
  RuleMap rules;
  PieceMap pieces;
  MatchMap matches;

  String representativePiece;

  MaterialRenderMatchList mainMatchList;
  List<pair<String, Vec2F>> crackingFrames;
  List<pair<String, Vec2F>> protectedFrames;
  List<Directives> colorDirectives;
  Json ruleProperties;

  bool foregroundLightTransparent;
  bool backgroundLightTransparent;
  uint8_t colorVariants;
  bool occludesBehind;
  uint32_t zLevel;
  Vec3F radiantLight;

  // Get a single asset path for just a single piece of a material, with the
  // image cropped to the piece itself.
  String pieceImage(String const& pieceName,
      unsigned variant,
      MaterialColorVariant colorVariant = DefaultMaterialColorVariant,
      MaterialHue hueShift = MaterialHue()) const;

  // Get an overlay image for rendering damaged tiles, as well as the offset
  // for it in world coordinates.
  pair<String, Vec2F> const& damageImage(float damageLevel, TileDamageType damageType) const;
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
