#pragma once

#include "StarJson.hpp"
#include "StarThread.hpp"
#include "StarDamageTypes.hpp"

namespace Star {

struct DamageKind;
class DamageDatabase;
using DamageDatabasePtr = SharedPtr<DamageDatabase>;
using DamageDatabaseConstPtr = SharedPtr<DamageDatabase const>;
struct ElementalType;

using TargetMaterial = String;

struct ElementalType {
  String resistanceStat;
  HashMap<HitType, String> damageNumberParticles;
};

struct DamageEffect {
  Json sounds;
  Json particles;
};

struct DamageKind {
  String name;
  HashMap<TargetMaterial, HashMap<HitType, DamageEffect>> effects;
  String elementalType;
};

class DamageDatabase {
public:
  DamageDatabase();

  DamageKind const& damageKind(String name) const;
  ElementalType const& elementalType(String const& name) const;

private:
  StringMap<DamageKind> m_damageKinds;
  StringMap<ElementalType> m_elementalTypes;
};

}
