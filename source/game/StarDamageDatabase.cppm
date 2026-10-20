module;

import std;
import star.string;
import star.map;
import star.json;

namespace Star {

class Assets;

enum class HitType;

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
  explicit DamageDatabase(std::shared_ptr<Assets const> assets);

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
  using ::Star::ElementalType;
  using ::Star::DamageDatabase;
}
