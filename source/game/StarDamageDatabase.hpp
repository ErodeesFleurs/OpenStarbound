#pragma once

#include "StarDamageTypes.hpp"
#include "StarJson.hpp"

namespace Star {

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

  [[nodiscard]] auto damageKind(String name) const -> DamageKind const&;
  [[nodiscard]] auto elementalType(String const& name) const -> ElementalType const&;

private:
  StringMap<DamageKind> m_damageKinds;
  StringMap<ElementalType> m_elementalTypes;
};

}// namespace Star
