#pragma once

#include "StarAnchorableEntity.hpp"
#include "StarConfig.hpp"
#include "StarDirectives.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarStatusTypes.hpp"

import std;

namespace Star {

enum class LoungeOrientation { None,
                               Sit,
                               Lay,
                               Stand };
extern EnumMap<LoungeOrientation> const LoungeOrientationNames;

enum class LoungeControl { Left,
                           Right,
                           Down,
                           Up,
                           Jump,
                           PrimaryFire,
                           AltFire,
                           Special1,
                           Special2,
                           Special3,
                           Walk };
extern EnumMap<LoungeControl> const LoungeControlNames;

struct LoungeAnchor : EntityAnchor {
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
  [[nodiscard]] auto anchorCount() const -> std::size_t override = 0;
  [[nodiscard]] auto anchor(std::size_t anchorPositionIndex) const -> ConstPtr<EntityAnchor> override;
  [[nodiscard]] virtual auto loungeAnchor(std::size_t anchorPositionIndex) const -> ConstPtr<LoungeAnchor> = 0;

  // Default does nothing.
  virtual void loungeControl(std::size_t anchorPositionIndex, LoungeControl loungeControl);
  virtual void loungeAim(std::size_t anchorPositionIndex, Vec2F const& aimPosition);

  // Queries around this entity's metaBoundBox for any LoungingEntities
  // reporting that they are lounging in this entity, and returns ones that are
  // lounging in the given position.
  [[nodiscard]] auto entitiesLoungingIn(std::size_t anchorPositionIndex) const -> Set<EntityId>;
  // Returns pairs of entity ids, and the position they are lounging in.
  [[nodiscard]] auto entitiesLounging() const -> Set<std::pair<EntityId, std::size_t>>;
};

// Any lounging entity should report the entity it is lounging in on both
// master and slave, so that lounging entities can cooperate and avoid lounging
// in the same spot.
class LoungingEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto loungingIn() const -> std::optional<EntityAnchorState> = 0;
  // Returns true if the entity is in a lounge achor, but other entities are
  // also reporting being in that lounge anchor.
  [[nodiscard]] auto inConflictingLoungeAnchor() const -> bool;
};

}// namespace Star
