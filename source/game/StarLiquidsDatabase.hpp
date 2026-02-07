#pragma once

#include "StarCellularLiquid.hpp"
#include "StarConfig.hpp"
#include "StarEither.hpp"
#include "StarException.hpp"
#include "StarItemDescriptor.hpp"
#include "StarJson.hpp"
#include "StarLiquidTypes.hpp"
#include "StarMaterialTypes.hpp"

import std;

namespace Star {

using LiquidException = ExceptionDerived<"LiquidException">;

using LiquidInteractionResult = Either<MaterialId, LiquidId>;

struct LiquidSettings {
  LiquidSettings();

  String name;
  LiquidId id;
  String path;
  Json config;
  Json descriptions;

  unsigned tickDelta;
  Vec4B liquidColor;
  Vec3F radiantLightLevel;
  ItemDescriptor itemDrop;
  JsonArray statusEffects;

  HashMap<LiquidId, std::optional<LiquidInteractionResult>> interactions;
};

class LiquidsDatabase {
public:
  LiquidsDatabase();

  [[nodiscard]] auto liquidEngineParameters() const -> LiquidCellEngineParameters;
  [[nodiscard]] auto backgroundDrain() const -> float;

  [[nodiscard]] auto liquidNames() const -> StringList;

  // Returns settings object for all liquids except "empty"
  [[nodiscard]] auto allLiquidSettings() const -> List<ConstPtr<LiquidSettings>>;

  [[nodiscard]] auto isLiquidName(String const& name) const -> bool;
  [[nodiscard]] auto isValidLiquidId(LiquidId liquidId) const -> bool;

  [[nodiscard]] auto liquidId(String const& str) const -> LiquidId;
  [[nodiscard]] auto liquidName(LiquidId liquidId) const -> String;
  [[nodiscard]] auto liquidDescription(LiquidId liquidId, String const& species) const -> String;
  [[nodiscard]] auto liquidDescription(LiquidId liquidId) const -> String;
  [[nodiscard]] auto liquidPath(LiquidId liquidId) const -> std::optional<String>;
  [[nodiscard]] auto liquidConfig(LiquidId liquidId) const -> std::optional<Json>;

  // Returns null on EmptyLiquidId or invalid liquid id
  [[nodiscard]] auto liquidSettings(LiquidId liquidId) const -> ConstPtr<LiquidSettings>;

  [[nodiscard]] auto radiantLight(LiquidLevel level) const -> Vec3F;

  [[nodiscard]] auto interact(LiquidId target, LiquidId other) const -> std::optional<LiquidInteractionResult>;

private:
  LiquidCellEngineParameters m_liquidEngineParameters;
  float m_backgroundDrain;
  List<ConstPtr<LiquidSettings>> m_settings;
  StringMap<LiquidId> m_liquidNames;
};

inline auto LiquidsDatabase::backgroundDrain() const -> float {
  return m_backgroundDrain;
}

inline auto LiquidsDatabase::isLiquidName(String const& name) const -> bool {
  return m_liquidNames.contains(name);
}

inline auto LiquidsDatabase::isValidLiquidId(LiquidId liquidId) const -> bool {
  return liquidId == EmptyLiquidId || (liquidId < m_settings.size() && m_settings[liquidId]);
}

inline auto LiquidsDatabase::liquidSettings(LiquidId liquidId) const -> ConstPtr<LiquidSettings> {
  if (liquidId >= m_settings.size())
    return {};
  return m_settings[liquidId];
}

inline auto LiquidsDatabase::radiantLight(LiquidLevel level) const -> Vec3F {
  if (level.liquid < m_settings.size()) {
    if (auto const& settings = m_settings[level.liquid])
      return settings->radiantLightLevel * level.level;
  }

  return {};
}

}// namespace Star
