#pragma once

#include "StarBiMap.hpp"
#include "StarConfig.hpp"
#include "StarDirectives.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarMaterialTypes.hpp"
#include "StarRect.hpp"
#include "StarTileDamage.hpp"

import std;

namespace Star {

using MaterialRenderProfileException = ExceptionDerived<"MaterialRenderProfileException">;

struct MaterialRenderMatch;

enum class MaterialJoinType : std::uint8_t { All,
                                             Any };
extern EnumMap<MaterialJoinType> const MaterialJoinTypeNames;

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
using RuleMap = StringMap<ConstPtr<MaterialRule>>;

struct MaterialMatchPoint {
  Vec2I position;
  ConstPtr<MaterialRule> rule;
};

struct MaterialRenderPiece {
  std::size_t pieceId;
  String texture;
  // Maps each MaterialColorVariant to a list of texture coordinates for each
  // random variant
  HashMap<MaterialColorVariant, List<RectF>> variants;
};

using MaterialRenderMatchList = List<ConstPtr<MaterialRenderMatch>>;

struct MaterialRenderMatch {
  List<MaterialMatchPoint> matchPoints;
  MaterialJoinType matchJoin;

  // Positions here are in TilePixels
  List<std::pair<ConstPtr<MaterialRenderPiece>, Vec2F>> resultingPieces;
  MaterialRenderMatchList subMatches;
  std::optional<TileLayer> requiredLayer;
  std::optional<bool> occlude;
  bool haltOnMatch;
  bool haltOnSubMatch;
};

using PieceMap = StringMap<ConstPtr<MaterialRenderPiece>>;
using MatchMap = StringMap<MaterialRenderMatchList>;

// This is the maximum distance in either X or Y that material neighbor rules
// are limited to.  This can be used as a maximum limit on the "sphere of
// influence" that a tile can have on other tile's rendering.  A value of 1
// here means "1 away", so would be interpreted as a 3x3 block with the
// rendered tile in the center.
int const MaterialRenderProfileMaxNeighborDistance = 2;

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
  [[nodiscard]] auto pieceImage(String const& pieceName,
                                unsigned variant,
                                MaterialColorVariant colorVariant = DefaultMaterialColorVariant,
                                MaterialHue hueShift = MaterialHue()) const -> String;

  // Get an overlay image for rendering damaged tiles, as well as the offset
  // for it in world coordinates.
  [[nodiscard]] auto damageImage(float damageLevel, TileDamageType damageType) const -> std::pair<String, Vec2F> const&;
};

auto parseMaterialRenderProfile(Json const& spec, String const& relativePath = "") -> MaterialRenderProfile;

}// namespace Star
