module;

#include "StarJson.hpp"

namespace Star {

enum class HitType;

STAR_STRUCT(DamageKind);
STAR_CLASS(DamageDatabase);
STAR_STRUCT(ElementalType);

typedef String TargetMaterial;

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

export module star.damage_database;

export namespace Star {
  using ::Star::HitType;
  using ::Star::TargetMaterial;
  using ::Star::DamageEffect;
  using ::Star::DamageKind;
  using ::Star::DamageKindPtr;
  using ::Star::DamageKindConstPtr;
  using ::Star::ElementalType;
  using ::Star::ElementalTypePtr;
  using ::Star::ElementalTypeConstPtr;
  using ::Star::DamageDatabase;
  using ::Star::DamageDatabasePtr;
  using ::Star::DamageDatabaseConstPtr;
}
