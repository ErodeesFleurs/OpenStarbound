#pragma once

#include "StarCellularLiquid.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarLiquidTypes.hpp"
#include "StarList.hpp"
#include "StarItemDescriptor.hpp"

namespace Star {

class WorldServer;

class WorldServerLiquid {
public:
  friend class WorldServer;

  WorldServerLiquid() = default;
  explicit WorldServerLiquid(WorldServer* worldServer);

  LiquidLevel liquidLevel(Vec2I const& pos) const;
  LiquidLevel liquidLevel(RectF const& region) const;
  void modifyLiquid(Vec2I const& pos, LiquidId liquid, float quantity, bool additive = false);
  void setLiquid(Vec2I const& pos, LiquidId liquid, float level, float pressure);
  void activateLiquidRegion(RectI const& region);
  ItemDescriptor collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId);
  shared_ptr<LiquidCellEngine<LiquidId>> liquidEngine() const;

private:
  WorldServer* m_worldServer = nullptr;

  shared_ptr<LiquidCellEngine<LiquidId>> m_liquidEngine;
};

}
