module;

#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"

namespace Star {

STAR_CLASS(TileDamageParameters);
STAR_CLASS(TileDamageStatus);
STAR_CLASS(EntityTileDamageStatus);

struct TileDamageExceptionTag {
  static constexpr char const* name() { return "TileDamageException"; }
};
using TileDamageException = StarError<TileDamageExceptionTag, StarException>;

List<Vec2I> tileAreaBrush(float range, Vec2F const& centerOffset, bool diameterMode);

enum class TileDamageType : uint8_t {
  // Damage done that will not actually kill the target
  Protected,
  // Best at chopping down trees, things made of wood, etc.
  Plantish,
  // For digging / drilling through materials
  Blockish,
  // Gravity gun etc
  Beamish,
  // Penetrating damage done passivly by explosions.
  Explosive,
  // Can melt certain block types
  Fire,
  // Can "till" certain materials into others
  Tilling
};
extern EnumMap<TileDamageType> const TileDamageTypeNames;

bool tileDamageIsPenetrating(TileDamageType damageType);

struct TileDamage {
  friend DataStream& operator>>(DataStream& ds, TileDamage& tileDamage);
  friend DataStream& operator<<(DataStream& ds, TileDamage const& tileDamage);
  TileDamage();
  TileDamage(TileDamageType type, float amount, unsigned harvestLevel = 1);

  TileDamageType type;
  float amount;
  unsigned harvestLevel;
};

DataStream& operator>>(DataStream& ds, TileDamage& tileDamage);
DataStream& operator<<(DataStream& ds, TileDamage const& tileDamage);

class TileDamageParameters {
public:
  TileDamageParameters();

  // If 'config' is a string type, it is assumed to be a descriptor file,
  // otherwise it should contain map configuration data.
  explicit TileDamageParameters(Json config, Maybe<float> healthOverride = {}, Maybe<unsigned> requiredHarvestLevelOverride = {});

  float damageDone(TileDamage const& damage) const;
  float recoveryPerSecond() const;
  unsigned requiredHarvestLevel() const;
  float maximumEffectTime() const;
  float totalHealth() const;

  TileDamageParameters sum(TileDamageParameters const& other) const;

  Json toJson() const;

  friend DataStream& operator>>(DataStream& ds, TileDamageParameters& tileDamage);
  friend DataStream& operator<<(DataStream& ds, TileDamageParameters const& tileDamage);

private:
  Map<TileDamageType, float> m_damages;
  float m_damageRecoveryPerSecond;
  float m_maximumEffectTime;
  float m_totalHealth;
  unsigned m_requiredHarvestLevel;
};

class TileDamageStatus {
public:
  TileDamageStatus();

  float damagePercentage() const;
  float damageEffectPercentage() const;
  Vec2F sourcePosition() const;
  TileDamageType damageType() const;

  void reset();
  void damage(TileDamageParameters const& damageParameters, Vec2F const& sourcePosition, TileDamage const& damage);
  void recover(TileDamageParameters const& damageParameters, float dt);

  bool healthy() const;
  bool damaged() const;
  bool damageProtected() const;
  bool dead() const;
  bool harvested() const;

  friend DataStream& operator>>(DataStream& ds, TileDamageStatus& tileDamageStatus);
  friend DataStream& operator<<(DataStream& ds, TileDamageStatus const& tileDamageStatus);

private:
  void updateDamageEffectPercentage();

  float m_damagePercentage;
  float m_damageEffectTimeFactor;
  bool m_harvested;
  Vec2F m_damageSourcePosition;
  TileDamageType m_damageType;
  float m_damageEffectPercentage;
};

class EntityTileDamageStatus : public NetElementGroup {
public:
  EntityTileDamageStatus();

  float damagePercentage() const;
  float damageEffectPercentage() const;
  TileDamageType damageType() const;

  void reset();
  void damage(TileDamageParameters const& damageParameters, TileDamage const& damage);
  void recover(TileDamageParameters const& damageParameters, float dt);

  bool healthy() const;
  bool damaged() const;
  bool damageProtected() const;
  bool dead() const;
  bool harvested() const;

private:
  NetElementFloat m_damagePercentage;
  NetElementFloat m_damageEffectTimeFactor;
  NetElementBool m_damageHarvested;
  NetElementEnum<TileDamageType> m_damageType;
};

inline float TileDamageStatus::damagePercentage() const {
  return m_damagePercentage;
}

inline float TileDamageStatus::damageEffectPercentage() const {
  return m_damageEffectPercentage;
}

inline Vec2F TileDamageStatus::sourcePosition() const {
  return m_damageSourcePosition;
}

inline TileDamageType TileDamageStatus::damageType() const {
  return m_damageType;
}

}

export module star.tile_damage;

export namespace Star {
  using ::Star::TileDamageParameters;
  using ::Star::TileDamageStatus;
  using ::Star::EntityTileDamageStatus;
  using ::Star::TileDamageExceptionTag;
  using ::Star::TileDamageException;
  using ::Star::tileAreaBrush;
  using ::Star::TileDamageType;
  using ::Star::TileDamageTypeNames;
  using ::Star::tileDamageIsPenetrating;
  using ::Star::TileDamage;
  using ::Star::TileDamageParametersPtr;
  using ::Star::TileDamageParametersConstPtr;
  using ::Star::TileDamageParametersWeakPtr;
  using ::Star::TileDamageParametersConstWeakPtr;
  using ::Star::TileDamageParametersUPtr;
  using ::Star::TileDamageParametersConstUPtr;
  using ::Star::TileDamageStatusPtr;
  using ::Star::TileDamageStatusConstPtr;
  using ::Star::TileDamageStatusWeakPtr;
  using ::Star::TileDamageStatusConstWeakPtr;
  using ::Star::TileDamageStatusUPtr;
  using ::Star::TileDamageStatusConstUPtr;
  using ::Star::EntityTileDamageStatusPtr;
  using ::Star::EntityTileDamageStatusConstPtr;
  using ::Star::EntityTileDamageStatusWeakPtr;
  using ::Star::EntityTileDamageStatusConstWeakPtr;
  using ::Star::EntityTileDamageStatusUPtr;
  using ::Star::EntityTileDamageStatusConstUPtr;
}
