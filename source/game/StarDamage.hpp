#pragma once

#include "StarDamageTypes.hpp"
#include "StarStatusTypes.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

struct DamageSource {
  using DamageArea = MVariant<PolyF, Line2F>;
  using Knockback = MVariant<float, Vec2F>;

  DamageSource();
  DamageSource(Json const& v);
  DamageSource(DamageType damageType,
               DamageArea damageArea,
               float damage,
               bool trackSourceEntity,
               EntityId sourceEntityId,
               EntityDamageTeam team,
               std::optional<String> damageRepeatGroup,
               std::optional<float> damageRepeatTimeout,
               String damageSourceKind,
               List<EphemeralStatusEffect> statusEffects,
               Knockback knockback,
               bool rayCheck);

  [[nodiscard]] auto toJson() const -> Json;

  auto translate(Vec2F const& position) -> DamageSource&;

  [[nodiscard]] auto intersectsWithPoly(WorldGeometry const& worldGeometry, PolyF const& poly) const -> bool;
  [[nodiscard]] auto knockbackMomentum(WorldGeometry const& worldGeometry, Vec2F const& targetCenter) const -> Vec2F;

  auto operator==(DamageSource const& rhs) const -> bool;

  DamageType damageType;
  DamageArea damageArea;
  float damage;

  bool trackSourceEntity;
  // The originating entity for the damage, which can be different than the
  // actual causing entity.  Optional, defaults to NullEntityId.
  EntityId sourceEntityId;
  EntityDamageTeam team;

  // Applying damage will block other DamageSources with the same
  // damageRepeatGroup from applying damage until the timeout expires, to
  // prevent damages being applied every tick.  This is optional, and if it is
  // omitted then the repeat group will instead be based on the causing entity
  // id.
  std::optional<String> damageRepeatGroup;
  // Can override the default repeat damage timeout with a custom timeout.
  std::optional<float> damageRepeatTimeout;

  String damageSourceKind;
  List<EphemeralStatusEffect> statusEffects;
  // Either directionless knockback momentum or directional knockback momentum
  Knockback knockback;
  // Should a collision check from the source entity to the impact center be
  // peformed before applying the damage?
  bool rayCheck;
};

auto operator<<(DataStream& ds, DamageSource const& damageSource) -> DataStream&;
auto operator>>(DataStream& ds, DamageSource& damageSource) -> DataStream&;

struct DamageRequest {
  DamageRequest();
  DamageRequest(Json const& v);
  DamageRequest(HitType hitType,
                DamageType type,
                float damage,
                Vec2F const& knockbackMomentum,
                EntityId sourceEntityId,
                String const& damageSourceKind,
                List<EphemeralStatusEffect> const& statusEffects);

  [[nodiscard]] auto toJson() const -> Json;

  HitType hitType;
  DamageType damageType;
  float damage;
  Vec2F knockbackMomentum;
  // May be different than the entity that actually caused damage, for example,
  // a player firing a projectile.
  EntityId sourceEntityId;
  String damageSourceKind;
  List<EphemeralStatusEffect> statusEffects;
};

auto operator<<(DataStream& ds, DamageRequest const& damageRequest) -> DataStream&;
auto operator>>(DataStream& ds, DamageRequest& damageRequest) -> DataStream&;

struct DamageNotification {
  DamageNotification();
  DamageNotification(Json const& v);
  DamageNotification(EntityId sourceEntityId,
                     EntityId targetEntityId,
                     Vec2F position,
                     float damageDealt,
                     float healthLost,
                     HitType hitType,
                     String damageSourceKind,
                     String targetMaterialKind);

  [[nodiscard]] auto toJson() const -> Json;

  EntityId sourceEntityId;
  EntityId targetEntityId;
  Vec2F position;
  float damageDealt;
  float healthLost;
  HitType hitType;
  String damageSourceKind;
  String targetMaterialKind;
};

auto operator<<(DataStream& ds, DamageNotification const& damageNotification) -> DataStream&;
auto operator>>(DataStream& ds, DamageNotification& damageNotification) -> DataStream&;
}// namespace Star
