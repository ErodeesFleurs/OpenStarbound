#pragma once

#include "StarAssets.hpp"
#include "StarJson.hpp"
#include "StarEither.hpp"
#include "StarGameTypes.hpp"
#include "StarList.hpp"
#include "StarCellularLiquid.hpp"
#include "StarItemDescriptor.hpp"

namespace Star {

struct LiquidSettings;
using LiquidSettingsConstPtr = SharedPtr<LiquidSettings const>;
class LiquidsDatabase;
using LiquidsDatabasePtr = SharedPtr<LiquidsDatabase>;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;
class MaterialDatabase;
using MaterialDatabaseConstPtr = SharedPtr<MaterialDatabase const>;

struct LiquidExceptionTag { static constexpr char const* typeName = "LiquidException"; };
using LiquidException = TypedException<StarException, LiquidExceptionTag>;

using LiquidInteractionResult = Either<MaterialId, LiquidId>;

struct LiquidSettings {
  LiquidSettings() = default;

  String name;
  LiquidId id = EmptyLiquidId;
  String path;
  Json config;
  Json descriptions;

  unsigned tickDelta = 0;
  Vec4B liquidColor;
  Vec3F radiantLightLevel;
  ItemDescriptor itemDrop;
  JsonArray statusEffects;

  HashMap<LiquidId, Maybe<LiquidInteractionResult>> interactions;
};

class LiquidsDatabase {
public:
  LiquidsDatabase(AssetsConstPtr assets, MaterialDatabaseConstPtr materialDatabase);

  [[nodiscard]] LiquidCellEngineParameters liquidEngineParameters() const;
  [[nodiscard]] float backgroundDrain() const;

  [[nodiscard]] StringList liquidNames() const;

  // Returns settings object for all liquids except "empty"
  [[nodiscard]] List<LiquidSettingsConstPtr> allLiquidSettings() const;

  [[nodiscard]] bool isLiquidName(String const& name) const;
  [[nodiscard]] bool isValidLiquidId(LiquidId liquidId) const;

  [[nodiscard]] LiquidId liquidId(String const& str) const;
  [[nodiscard]] String liquidName(LiquidId liquidId) const;
  [[nodiscard]] String liquidDescription(LiquidId liquidId, String const& species) const;
  [[nodiscard]] String liquidDescription(LiquidId liquidId) const;
  [[nodiscard]] Maybe<String> liquidPath(LiquidId liquidId) const;
  [[nodiscard]] Maybe<Json> liquidConfig(LiquidId liquidId) const;

  // Returns null on EmptyLiquidId or invalid liquid id
  [[nodiscard]] LiquidSettingsConstPtr liquidSettings(LiquidId liquidId) const;

  [[nodiscard]] Vec3F radiantLight(LiquidLevel level) const;

  [[nodiscard]] Maybe<LiquidInteractionResult> interact(LiquidId target, LiquidId other) const;

private:
  LiquidCellEngineParameters m_liquidEngineParameters;
  float m_backgroundDrain;
  List<LiquidSettingsConstPtr> m_settings;
  StringMap<LiquidId> m_liquidNames;
};

[[nodiscard]] inline float LiquidsDatabase::backgroundDrain() const {
  return m_backgroundDrain;
}

[[nodiscard]] inline bool LiquidsDatabase::isLiquidName(String const& name) const {
  return m_liquidNames.contains(name);
}

[[nodiscard]] inline bool LiquidsDatabase::isValidLiquidId(LiquidId liquidId) const {
  return liquidId == EmptyLiquidId || (liquidId < m_settings.size() && m_settings[liquidId]);
}

[[nodiscard]] inline LiquidSettingsConstPtr LiquidsDatabase::liquidSettings(LiquidId liquidId) const {
  if (liquidId >= m_settings.size())
    return {};
  return m_settings[liquidId];
}

[[nodiscard]] inline Vec3F LiquidsDatabase::radiantLight(LiquidLevel level) const {
  if (level.liquid < m_settings.size()) {
    if (auto const& settings = m_settings[level.liquid])
      return settings->radiantLightLevel * level.level;
  }

  return Vec3F();
}

}
