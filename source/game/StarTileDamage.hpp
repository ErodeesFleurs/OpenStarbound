#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementGroup.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

using TileDamageException = ExceptionDerived<"TileDamageException">;

auto tileAreaBrush(float range, Vec2F const& centerOffset, bool diameterMode) -> List<Vec2I>;

enum class TileDamageType : std::uint8_t {
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

auto tileDamageIsPenetrating(TileDamageType damageType) -> bool;

struct TileDamage {
  TileDamage();
  TileDamage(TileDamageType type, float amount, unsigned harvestLevel = 1);

  TileDamageType type;
  float amount;
  unsigned harvestLevel;
};

auto operator>>(DataStream& ds, TileDamage& tileDamage) -> DataStream&;
auto operator<<(DataStream& ds, TileDamage const& tileDamage) -> DataStream&;

class TileDamageParameters {
public:
  TileDamageParameters();

  // If 'config' is a string type, it is assumed to be a descriptor file,
  // otherwise it should contain map configuration data.
  explicit TileDamageParameters(Json config, std::optional<float> healthOverride = {}, std::optional<unsigned> requiredHarvestLevelOverride = {});

  [[nodiscard]] auto damageDone(TileDamage const& damage) const -> float;
  [[nodiscard]] auto recoveryPerSecond() const -> float;
  [[nodiscard]] auto requiredHarvestLevel() const -> unsigned;
  [[nodiscard]] auto maximumEffectTime() const -> float;
  [[nodiscard]] auto totalHealth() const -> float;

  [[nodiscard]] auto sum(TileDamageParameters const& other) const -> TileDamageParameters;

  [[nodiscard]] auto toJson() const -> Json;

  friend auto operator>>(DataStream& ds, TileDamageParameters& tileDamage) -> DataStream&;
  friend auto operator<<(DataStream& ds, TileDamageParameters const& tileDamage) -> DataStream&;

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

  [[nodiscard]] auto damagePercentage() const -> float;
  [[nodiscard]] auto damageEffectPercentage() const -> float;
  [[nodiscard]] auto sourcePosition() const -> Vec2F;
  [[nodiscard]] auto damageType() const -> TileDamageType;

  void reset();
  void damage(TileDamageParameters const& damageParameters, Vec2F const& sourcePosition, TileDamage const& damage);
  void recover(TileDamageParameters const& damageParameters, float dt);

  [[nodiscard]] auto healthy() const -> bool;
  [[nodiscard]] auto damaged() const -> bool;
  [[nodiscard]] auto damageProtected() const -> bool;
  [[nodiscard]] auto dead() const -> bool;
  [[nodiscard]] auto harvested() const -> bool;

  friend auto operator>>(DataStream& ds, TileDamageStatus& tileDamageStatus) -> DataStream&;
  friend auto operator<<(DataStream& ds, TileDamageStatus const& tileDamageStatus) -> DataStream&;

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

  auto damagePercentage() const -> float;
  auto damageEffectPercentage() const -> float;
  auto damageType() const -> TileDamageType;

  void reset();
  void damage(TileDamageParameters const& damageParameters, TileDamage const& damage);
  void recover(TileDamageParameters const& damageParameters, float dt);

  auto healthy() const -> bool;
  auto damaged() const -> bool;
  auto damageProtected() const -> bool;
  auto dead() const -> bool;
  auto harvested() const -> bool;

private:
  NetElementFloat m_damagePercentage;
  NetElementFloat m_damageEffectTimeFactor;
  NetElementBool m_damageHarvested;
  NetElementEnum<TileDamageType> m_damageType;
};

inline auto TileDamageStatus::damagePercentage() const -> float {
  return m_damagePercentage;
}

inline auto TileDamageStatus::damageEffectPercentage() const -> float {
  return m_damageEffectPercentage;
}

inline auto TileDamageStatus::sourcePosition() const -> Vec2F {
  return m_damageSourcePosition;
}

inline auto TileDamageStatus::damageType() const -> TileDamageType {
  return m_damageType;
}

}// namespace Star
