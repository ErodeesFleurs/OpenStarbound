#pragma once

#include "StarVector.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"

namespace Star {
class DataStream;

enum DamageType : uint8_t { NoDamage, Damage, IgnoresDef, Knockback, Environment, Status };
extern EnumMap<DamageType> const DamageTypeNames;

enum class HitType { Hit, StrongHit, WeakHit, ShieldHit, Kill };
extern EnumMap<HitType> const HitTypeNames;

enum class TeamType : uint8_t {
  Null,
  // non-PvP-enabled players and player allied NPCs
  Friendly,
  // hostile and neutral NPCs and monsters
  Enemy,
  // PvP-enabled players
  PVP,
  // cannot damage anything, can be damaged by Friendly/PVP/Assistant
  Passive,
  // cannot damage or be damaged
  Ghostly,
  // cannot damage enemies, can be damaged by anything except enemy
  Environment,
  // damages anything except ghostly, damaged by anything except ghostly/passive
  // used for self damage
  Indiscriminate,
  // cannot damage friendlies and cannot be damaged by anything
  Assistant
};
extern EnumMap<TeamType> const TeamTypeNames;

using TeamNumber = uint16_t;

struct EntityDamageTeam {
  EntityDamageTeam() = default;
  explicit EntityDamageTeam(TeamType type, TeamNumber team = 0);
  explicit EntityDamageTeam(Json const& json);

  [[nodiscard]] Json toJson() const;

  [[nodiscard]] bool canDamage(EntityDamageTeam victim, bool victimIsSelf) const;

  [[nodiscard]] bool operator==(EntityDamageTeam const& rhs) const;

  TeamType type = TeamType::Null;
  TeamNumber team = 0;
};
DataStream& operator<<(DataStream& ds, EntityDamageTeam const& team);
DataStream& operator>>(DataStream& ds, EntityDamageTeam& team);

[[nodiscard]] TeamNumber soloPvpTeam(ConnectionId clientId);
}
