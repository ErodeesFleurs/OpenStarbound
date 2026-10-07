module;
#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"

import star.drawable;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

import star.anchorable_entity;
import star.entity_rendering_types;

namespace Star {

STAR_CLASS(World);

STAR_STRUCT(LoungeAnchor);
STAR_CLASS(LoungeableEntity);
STAR_CLASS(LoungingEntity);

enum class LoungeOrientation { None, Sit, Lay, Stand };
extern EnumMap<LoungeOrientation> const LoungeOrientationNames;

enum class LoungeControl { Left, Right, Down, Up, Jump, PrimaryFire, AltFire, Special1, Special2, Special3, Walk };
extern EnumMap<LoungeControl> const LoungeControlNames;

struct LoungeAnchor : EntityAnchor {
  LoungeOrientation orientation;
  EntityRenderLayer loungeRenderLayer;
  bool controllable;
  List<PersistentStatusEffect> statusEffects;
  StringSet effectEmitters;
  Maybe<String> emote;
  Maybe<String> dance;
  Maybe<Directives> directives;
  JsonObject armorCosmeticOverrides;
  Maybe<String> cursorOverride;
  Maybe<bool> suppressTools;
  bool cameraFocus;
};

// Extends an AnchorableEntity to have more specific effects when anchoring,
// such as status effects and lounge controls.  All LoungeableEntity methods
// may be called on both the master and slave.
class LoungeableEntity : public AnchorableEntity {
public:
  virtual size_t anchorCount() const override = 0;
  EntityAnchorConstPtr anchor(size_t anchorPositionIndex) const override;
  virtual LoungeAnchorConstPtr loungeAnchor(size_t anchorPositionIndex) const = 0;

  // Default does nothing.
  virtual void loungeControl(size_t anchorPositionIndex, LoungeControl loungeControl);
  virtual void loungeAim(size_t anchorPositionIndex, Vec2F const& aimPosition);

  // Queries around this entity's metaBoundBox for any LoungingEntities
  // reporting that they are lounging in this entity, and returns ones that are
  // lounging in the given position.
  Set<EntityId> entitiesLoungingIn(size_t anchorPositionIndex) const;
  // Returns pairs of entity ids, and the position they are lounging in.
  Set<pair<EntityId, size_t>> entitiesLounging() const;
};

// Any lounging entity should report the entity it is lounging in on both
// master and slave, so that lounging entities can cooperate and avoid lounging
// in the same spot.
class LoungingEntity : public virtual Entity {
public:
  virtual Maybe<EntityAnchorState> loungingIn() const = 0;
  // Returns true if the entity is in a lounge achor, but other entities are
  // also reporting being in that lounge anchor.
  bool inConflictingLoungeAnchor() const;
};

}

export module star.lounging_entities;

export namespace Star {
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::LoungeAnchor;
  using ::Star::LoungeAnchorPtr;
  using ::Star::LoungeAnchorConstPtr;
  using ::Star::LoungeAnchorWeakPtr;
  using ::Star::LoungeAnchorConstWeakPtr;
  using ::Star::LoungeAnchorUPtr;
  using ::Star::LoungeAnchorConstUPtr;
  using ::Star::LoungeableEntity;
  using ::Star::LoungeableEntityPtr;
  using ::Star::LoungeableEntityConstPtr;
  using ::Star::LoungeableEntityWeakPtr;
  using ::Star::LoungeableEntityConstWeakPtr;
  using ::Star::LoungeableEntityUPtr;
  using ::Star::LoungeableEntityConstUPtr;
  using ::Star::LoungingEntity;
  using ::Star::LoungingEntityPtr;
  using ::Star::LoungingEntityConstPtr;
  using ::Star::LoungingEntityWeakPtr;
  using ::Star::LoungingEntityConstWeakPtr;
  using ::Star::LoungingEntityUPtr;
  using ::Star::LoungingEntityConstUPtr;
  using ::Star::LoungeOrientation;
  using ::Star::LoungeOrientationNames;
  using ::Star::LoungeControl;
  using ::Star::LoungeControlNames;
}
