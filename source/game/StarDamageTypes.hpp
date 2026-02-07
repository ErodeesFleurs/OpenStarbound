#pragma once

#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

enum DamageType : std::uint8_t { NoDamage,
                                 Damage,
                                 IgnoresDef,
                                 Knockback,
                                 Environment,
                                 Status };
extern EnumMap<DamageType> const DamageTypeNames;

enum class HitType { Hit,
                     StrongHit,
                     WeakHit,
                     ShieldHit,
                     Kill };
extern EnumMap<HitType> const HitTypeNames;

enum class TeamType : std::uint8_t {
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

using TeamNumber = std::uint16_t;

struct EntityDamageTeam {
  EntityDamageTeam();
  explicit EntityDamageTeam(TeamType type, TeamNumber team = 0);
  explicit EntityDamageTeam(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto canDamage(EntityDamageTeam victim, bool victimIsSelf) const -> bool;

  auto operator==(EntityDamageTeam const& rhs) const -> bool;

  TeamType type;
  TeamNumber team;
};
auto operator<<(DataStream& ds, EntityDamageTeam const& team) -> DataStream&;
auto operator>>(DataStream& ds, EntityDamageTeam& team) -> DataStream&;

auto soloPvpTeam(ConnectionId clientId) -> TeamNumber;
}// namespace Star
