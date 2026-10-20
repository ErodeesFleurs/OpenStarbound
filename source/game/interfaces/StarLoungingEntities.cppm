module;
import std;
import star.type_identity;
import star.set;
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
import star.casting;
import star.vector;
import star.game_types;
import star.strong_typedef;
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

enum class LoungeOrientation { None, Sit, Lay, Stand };
extern EnumMap<LoungeOrientation> const LoungeOrientationNames;

enum class LoungeControl { Left, Right, Down, Up, Jump, PrimaryFire, AltFire, Special1, Special2, Special3, Walk };
extern EnumMap<LoungeControl> const LoungeControlNames;

struct LoungeAnchor : EntityAnchor {
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<LoungeAnchor, EntityAnchor>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<LoungeAnchor>();
  }

  LoungeOrientation orientation;
  EntityRenderLayer loungeRenderLayer;
  bool controllable;
  List<PersistentStatusEffect> statusEffects;
  StringSet effectEmitters;
  std::optional<String> emote;
  std::optional<String> dance;
  std::optional<Directives> directives;
  JsonObject armorCosmeticOverrides;
  std::optional<String> cursorOverride;
  std::optional<bool> suppressTools;
  bool cameraFocus;
};

// Extends an AnchorableEntity to have more specific effects when anchoring,
// such as status effects and lounge controls.  All LoungeableEntity methods
// may be called on both the master and slave.
class LoungeableEntity : public AnchorableEntity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<LoungeableEntity, AnchorableEntity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<LoungeableEntity>();
  }

  virtual std::size_t anchorCount() const override = 0;
  std::shared_ptr<EntityAnchor const> anchor(std::size_t anchorPositionIndex) const override;
  virtual std::shared_ptr<LoungeAnchor const> loungeAnchor(std::size_t anchorPositionIndex) const = 0;

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
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<LoungingEntity, Entity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<LoungingEntity>();
  }

  virtual std::optional<EntityAnchorState> loungingIn() const = 0;
  // Returns true if the entity is in a lounge achor, but other entities are
  // also reporting being in that lounge anchor.
  bool inConflictingLoungeAnchor() const;
};

}

export module star.lounging_entities;

export namespace Star {
  using ::Star::World;
  using ::Star::LoungeAnchor;
  using ::Star::LoungeableEntity;
  using ::Star::LoungingEntity;
  using ::Star::LoungeOrientation;
  using ::Star::LoungeOrientationNames;
  using ::Star::LoungeControl;
  using ::Star::LoungeControlNames;
}
