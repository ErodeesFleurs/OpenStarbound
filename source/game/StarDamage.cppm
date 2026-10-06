module;

#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
import star.damage_types;
#include "StarPoly.hpp"
import star.world_geometry;
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"
#include "StarIdMap.hpp"
import star.status_types;

namespace Star {

struct DamageSource {
  friend DataStream& operator<<(DataStream& ds, DamageSource const& damageSource);
  friend DataStream& operator>>(DataStream& ds, DamageSource& damageSource);

  typedef MVariant<PolyF, Line2F> DamageArea;
  typedef MVariant<float, Vec2F> Knockback;

  DamageSource();
  DamageSource(Json const& v);
  DamageSource(DamageType damageType,
      DamageArea damageArea,
      float damage,
      bool trackSourceEntity,
      EntityId sourceEntityId,
      EntityDamageTeam team,
      Maybe<String> damageRepeatGroup,
      Maybe<float> damageRepeatTimeout,
      String damageSourceKind,
      List<EphemeralStatusEffect> statusEffects,
      Knockback knockback,
      bool rayCheck);

  Json toJson() const;

  DamageSource& translate(Vec2F const& position);

  bool intersectsWithPoly(WorldGeometry const& worldGeometry, PolyF const& poly) const;
  Vec2F knockbackMomentum(WorldGeometry const& worldGeometry, Vec2F const& targetCenter) const;

  bool operator==(DamageSource const& rhs) const;

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
  Maybe<String> damageRepeatGroup;
  // Can override the default repeat damage timeout with a custom timeout.
  Maybe<float> damageRepeatTimeout;

  String damageSourceKind;
  List<EphemeralStatusEffect> statusEffects;
  // Either directionless knockback momentum or directional knockback momentum
  Knockback knockback;
  // Should a collision check from the source entity to the impact center be
  // peformed before applying the damage?
  bool rayCheck;
};

DataStream& operator<<(DataStream& ds, DamageSource const& damageSource);
DataStream& operator>>(DataStream& ds, DamageSource& damageSource);

struct DamageRequest {
  friend DataStream& operator<<(DataStream& ds, DamageRequest const& damageRequest);
  friend DataStream& operator>>(DataStream& ds, DamageRequest& damageRequest);

  DamageRequest();
  DamageRequest(Json const& v);
  DamageRequest(HitType hitType,
      DamageType type,
      float damage,
      Vec2F const& knockbackMomentum,
      EntityId sourceEntityId,
      String const& damageSourceKind,
      List<EphemeralStatusEffect> const& statusEffects);

  Json toJson() const;

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

DataStream& operator<<(DataStream& ds, DamageRequest const& damageRequest);
DataStream& operator>>(DataStream& ds, DamageRequest& damageRequest);

struct DamageNotification {
  friend DataStream& operator<<(DataStream& ds, DamageNotification const& damageNotification);
  friend DataStream& operator>>(DataStream& ds, DamageNotification& damageNotification);

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

  Json toJson() const;

  EntityId sourceEntityId;
  EntityId targetEntityId;
  Vec2F position;
  float damageDealt;
  float healthLost;
  HitType hitType;
  String damageSourceKind;
  String targetMaterialKind;
};

DataStream& operator<<(DataStream& ds, DamageNotification const& damageNotification);
DataStream& operator>>(DataStream& ds, DamageNotification& damageNotification);
}

export module star.damage;

export namespace Star {
  using ::Star::DamageSource;
  using ::Star::DamageRequest;
  using ::Star::DamageNotification;
}
