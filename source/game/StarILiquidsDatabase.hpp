#pragma once

#include "StarGameTypes.hpp"
#include "StarLiquidTypes.hpp"
#include "StarMaterialTypes.hpp"
#include "StarMaybe.hpp"
#include "StarVector.hpp"

namespace Star {

struct LiquidSettings;
using LiquidSettingsConstPtr = SharedPtr<LiquidSettings const>;

class ILiquidsDatabase {
public:
  virtual ~ILiquidsDatabase() = default;

  virtual bool isLiquidName(String const& name) const = 0;
  virtual LiquidId liquidId(String const& name) const = 0;
  virtual String liquidName(LiquidId id) const = 0;
  virtual LiquidSettingsConstPtr liquidSettings(LiquidId liquidId) const = 0;
  virtual Vec3F radiantLight(LiquidLevel level) const = 0;
};

using ILiquidsDatabasePtr = SharedPtr<ILiquidsDatabase>;
using ILiquidsDatabaseConstPtr = SharedPtr<ILiquidsDatabase const>;

}
