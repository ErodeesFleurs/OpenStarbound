module;

import std;
import star.string;
import star.map;
import star.json;

namespace Star {

enum class HitType;

struct DamageKind;
using DamageKindPtr = std::shared_ptr<DamageKind>;
using DamageKindConstPtr = std::shared_ptr<const DamageKind>;
using DamageKindWeakPtr = std::weak_ptr<DamageKind>;
using DamageKindConstWeakPtr = std::weak_ptr<const DamageKind>;
using DamageKindUPtr = std::unique_ptr<DamageKind>;
using DamageKindConstUPtr = std::unique_ptr<const DamageKind>;
class DamageDatabase;
using DamageDatabasePtr = std::shared_ptr<DamageDatabase>;
using DamageDatabaseConstPtr = std::shared_ptr<const DamageDatabase>;
using DamageDatabaseWeakPtr = std::weak_ptr<DamageDatabase>;
using DamageDatabaseConstWeakPtr = std::weak_ptr<const DamageDatabase>;
using DamageDatabaseUPtr = std::unique_ptr<DamageDatabase>;
using DamageDatabaseConstUPtr = std::unique_ptr<const DamageDatabase>;
struct ElementalType;
using ElementalTypePtr = std::shared_ptr<ElementalType>;
using ElementalTypeConstPtr = std::shared_ptr<const ElementalType>;
using ElementalTypeWeakPtr = std::weak_ptr<ElementalType>;
using ElementalTypeConstWeakPtr = std::weak_ptr<const ElementalType>;
using ElementalTypeUPtr = std::unique_ptr<ElementalType>;
using ElementalTypeConstUPtr = std::unique_ptr<const ElementalType>;

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
