module;
import std;
import star.set;
import star.maybe;
import star.json;
import star.id_map;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
import star.vector;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;

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

class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;

struct LoungeAnchor;
using LoungeAnchorPtr = std::shared_ptr<LoungeAnchor>;
using LoungeAnchorConstPtr = std::shared_ptr<const LoungeAnchor>;
using LoungeAnchorWeakPtr = std::weak_ptr<LoungeAnchor>;
using LoungeAnchorConstWeakPtr = std::weak_ptr<const LoungeAnchor>;
using LoungeAnchorUPtr = std::unique_ptr<LoungeAnchor>;
using LoungeAnchorConstUPtr = std::unique_ptr<const LoungeAnchor>;
class LoungeableEntity;
using LoungeableEntityPtr = std::shared_ptr<LoungeableEntity>;
using LoungeableEntityConstPtr = std::shared_ptr<const LoungeableEntity>;
using LoungeableEntityWeakPtr = std::weak_ptr<LoungeableEntity>;
using LoungeableEntityConstWeakPtr = std::weak_ptr<const LoungeableEntity>;
using LoungeableEntityUPtr = std::unique_ptr<LoungeableEntity>;
using LoungeableEntityConstUPtr = std::unique_ptr<const LoungeableEntity>;
class LoungingEntity;
using LoungingEntityPtr = std::shared_ptr<LoungingEntity>;
using LoungingEntityConstPtr = std::shared_ptr<const LoungingEntity>;
using LoungingEntityWeakPtr = std::weak_ptr<LoungingEntity>;
using LoungingEntityConstWeakPtr = std::weak_ptr<const LoungingEntity>;
using LoungingEntityUPtr = std::unique_ptr<LoungingEntity>;
using LoungingEntityConstUPtr = std::unique_ptr<const LoungingEntity>;

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
  virtual std::size_t anchorCount() const override = 0;
  EntityAnchorConstPtr anchor(std::size_t anchorPositionIndex) const override;
  virtual LoungeAnchorConstPtr loungeAnchor(std::size_t anchorPositionIndex) const = 0;

  // Default does nothing.
  virtual void loungeControl(std::size_t anchorPositionIndex, LoungeControl loungeControl);
  virtual void loungeAim(std::size_t anchorPositionIndex, Vec2F const& aimPosition);

  // Queries around this entity's metaBoundBox for any LoungingEntities
  // reporting that they are lounging in this entity, and returns ones that are
  // lounging in the given position.
  Set<EntityId> entitiesLoungingIn(std::size_t anchorPositionIndex) const;
  // Returns pairs of entity ids, and the position they are lounging in.
  Set<std::pair<EntityId, std::size_t>> entitiesLounging() const;
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
