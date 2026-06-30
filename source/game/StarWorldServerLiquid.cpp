#include "StarWorldServerLiquid.hpp"
#include "StarWorldServer.hpp"
#include "StarWorldImpl.hpp"
#include "StarRoot.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarAssets.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLiquidTypes.hpp"

namespace Star {

WorldServerLiquid::WorldServerLiquid(WorldServer* worldServer)
  : m_worldServer(worldServer) {}

LiquidLevel WorldServerLiquid::liquidLevel(Vec2I const& pos) const {
  return m_worldServer->m_tileArray->tile(pos).liquid;
}

LiquidLevel WorldServerLiquid::liquidLevel(RectF const& region) const {
  return WorldImpl::liquidLevel(m_worldServer->m_tileArray, region);
}

void WorldServerLiquid::modifyLiquid(Vec2I const& pos, LiquidId liquid, float quantity, bool additive) {
  if (liquid == EmptyLiquidId)
    quantity = 0;

  if (ServerTile* tile = m_worldServer->m_tileArray->modifyTile(pos)) {
    auto materialDatabase = Root::singleton().materialDatabase();
    if (tile->foreground == EmptyMaterialId || !isSolidColliding(materialDatabase->materialCollisionKind(tile->foreground))) {
      if (additive && liquid == tile->liquid.liquid)
        quantity += tile->liquid.level;

      setLiquid(pos, liquid, quantity, tile->liquid.pressure);
      m_liquidEngine->visitLocation(pos);
    }
  }
}

void WorldServerLiquid::setLiquid(Vec2I const& pos, LiquidId liquid, float level, float pressure) {
  if (ServerTile* tile = m_worldServer->m_tileArray->modifyTile(pos)) {
    if (liquid == EmptyLiquidId)
      level = 0;

    if (auto netUpdate = tile->liquid.update(liquid, level, pressure)) {
      for (auto const& pair : m_worldServer->m_clientInfo) {
        if (pair.second->activeSectors.contains(m_worldServer->m_tileArray->sectorFor(pos)))
          pair.second->pendingLiquidUpdates.add(pos);
      }
    }
  }
}

void WorldServerLiquid::activateLiquidRegion(RectI const& region) {
  m_liquidEngine->visitRegion(region);
}

ItemDescriptor WorldServerLiquid::collectLiquid(List<Vec2I> const& tilePositions, LiquidId liquidId) {
  float bucketSize = Root::singleton().assets()->json("/items/defaultParameters.config:liquidItems.bucketSize").toFloat();
  unsigned drainedUnits = 0;
  float nextUnit = bucketSize;
  List<ServerTile*> maybeDrainTiles;

  for (auto const& pos : tilePositions) {
    ServerTile* tile = m_worldServer->m_tileArray->modifyTile(pos);
    if (tile->liquid.liquid == liquidId && !m_worldServer->isTileProtected(pos)) {
      if (tile->liquid.level >= nextUnit) {
        tile->liquid.take(nextUnit);
        nextUnit = bucketSize;
        drainedUnits++;

        for (auto previousTile : maybeDrainTiles)
          previousTile->liquid.take(previousTile->liquid.level);

        maybeDrainTiles.clear();
      }

      if (tile->liquid.level > 0) {
        nextUnit -= tile->liquid.level;
        maybeDrainTiles.append(tile);
      }

      for (auto const& pair : m_worldServer->m_clientInfo) {
        if (pair.second->activeSectors.contains(m_worldServer->m_tileArray->sectorFor(pos)))
          pair.second->pendingLiquidUpdates.add(pos);
      }
      m_liquidEngine->visitLocation(pos);
    }
  }

  if (drainedUnits > 0) {
    auto liquidConfig = Root::singleton().liquidsDatabase()->liquidSettings(liquidId);
    if (liquidConfig && liquidConfig->itemDrop)
      return liquidConfig->itemDrop.multiply(drainedUnits);
  }

  return ItemDescriptor();
}

shared_ptr<LiquidCellEngine<LiquidId>> WorldServerLiquid::liquidEngine() const {
  return m_liquidEngine;
}

}
