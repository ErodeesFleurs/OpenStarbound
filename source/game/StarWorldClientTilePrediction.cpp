#include "StarWorldClientTilePrediction.hpp"
#include "StarWorldClient.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarLogging.hpp"

namespace Star {

StarWorldClientTilePrediction::StarWorldClientTilePrediction(WorldClient& worldClient)
  : m_worldClient(worldClient) {}

void StarWorldClientTilePrediction::informTilePrediction(Vec2I const& pos, TileModification const& modification) {
  auto now = Time::monotonicMilliseconds();
  auto& p = m_predictedTiles[pos];
  p.time = now;
  if (auto placeMaterial = modification.ptr<PlaceMaterial>()) {
    if (placeMaterial->layer == TileLayer::Foreground) {
      auto materialDatabase = m_worldClient.m_materialDatabase;
      if (!materialDatabase->isCascadingFallingMaterial(placeMaterial->material)
          && !materialDatabase->isFallingMaterial(placeMaterial->material)) {
        p.foreground = placeMaterial->material;
        p.foregroundHueShift = placeMaterial->materialHueShift;
      } else {
        p.foreground = StructureMaterialId;
      }
      if (placeMaterial->collisionOverride != TileCollisionOverride::None)
        p.collision = collisionKindFromOverride(placeMaterial->collisionOverride);
      else
        p.collision = materialDatabase->materialCollisionKind(placeMaterial->material);
      m_worldClient.dirtyCollision(RectI::withSize(pos, {1, 1}));
    } else {
      p.background = placeMaterial->material;
      p.backgroundHueShift = placeMaterial->materialHueShift;
    }
  }
  else if (auto placeMod = modification.ptr<PlaceMod>()) {
    if (placeMod->layer == TileLayer::Foreground)
      p.foregroundMod = placeMod->mod;
    else
      p.backgroundMod = placeMod->mod;
  }
  else if (auto placeColor = modification.ptr<PlaceMaterialColor>()) {
    if (placeColor->layer == TileLayer::Foreground)
      p.foregroundColorVariant = placeColor->color;
    else
      p.backgroundColorVariant = placeColor->color;
  }
  else if (auto placeLiquid = modification.ptr<PlaceLiquid>()) {
    if (!p.liquid || p.liquid->liquid != placeLiquid->liquid)
      p.liquid = LiquidLevel(placeLiquid->liquid, placeLiquid->liquidLevel);
    else
      p.liquid->level += placeLiquid->liquidLevel;
  }
}

bool StarWorldClientTilePrediction::readNetTile(Vec2I const& pos, NetTile const& netTile, bool updateCollision) {
  ClientTile* tile = m_worldClient.m_tileArray->modifyTile(pos);
  if (!tile)
    return false;

  if (!m_predictedTiles.empty()) {
    auto findPrediction = m_predictedTiles.find(pos);
    if (findPrediction != m_predictedTiles.end()) {
      auto& p = findPrediction->second;

      if (p.collision && *p.collision == netTile.collision)
        p.collision.reset();
      if (p.foreground && (*p.foreground == StructureMaterialId || *p.foreground == netTile.foreground))
        p.foreground.reset();
      if (p.foregroundMod && *p.foregroundMod == netTile.foregroundMod)
        p.foregroundMod.reset();
      if (p.foregroundHueShift && *p.foregroundHueShift == netTile.foregroundHueShift)
        p.foregroundHueShift.reset();
      if (p.foregroundModHueShift && *p.foregroundModHueShift == netTile.foregroundModHueShift)
        p.foregroundModHueShift.reset();

      if (p.background && *p.background == netTile.background)
        p.background.reset();
      if (p.backgroundMod && *p.backgroundMod == netTile.backgroundMod)
        p.backgroundMod.reset();
      if (p.backgroundHueShift && *p.backgroundHueShift == netTile.backgroundHueShift)
        p.backgroundHueShift.reset();
      if (p.backgroundModHueShift && *p.backgroundModHueShift == netTile.backgroundModHueShift)
        p.backgroundModHueShift.reset();

      if (!p)
        m_predictedTiles.erase(findPrediction);
    }
  }

  tile->background = netTile.background;
  tile->backgroundHueShift = netTile.backgroundHueShift;
  tile->backgroundColorVariant = netTile.backgroundColorVariant;
  tile->backgroundMod = netTile.backgroundMod;
  tile->backgroundModHueShift = netTile.backgroundModHueShift;
  tile->foreground = netTile.foreground;
  tile->foregroundHueShift = netTile.foregroundHueShift;
  tile->foregroundColorVariant = netTile.foregroundColorVariant;
  tile->foregroundMod = netTile.foregroundMod;
  tile->foregroundModHueShift = netTile.foregroundModHueShift;
  tile->collision = netTile.collision;
  tile->blockBiomeIndex = netTile.blockBiomeIndex;
  tile->environmentBiomeIndex = netTile.environmentBiomeIndex;
  tile->liquid = netTile.liquid.liquidLevel();
  tile->dungeonId = netTile.dungeonId;

  auto materialDatabase = m_worldClient.m_materialDatabase;
  tile->backgroundLightTransparent = materialDatabase->backgroundLightTransparent(tile->background);
  tile->foregroundLightTransparent =
      materialDatabase->foregroundLightTransparent(tile->foreground) && tile->collision != CollisionKind::Dynamic;

  if (updateCollision)
    m_worldClient.dirtyCollision(RectI::withSize(pos, {1, 1}));

  return true;
}

void StarWorldClientTilePrediction::expirePredictedTiles() {
  float expireTime = min(float(m_worldClient.m_latency + 800), 2000.f);
  auto now = Time::monotonicMilliseconds();
  eraseWhere(m_predictedTiles, [&](auto& predictedTile) {
    auto& [position, prediction] = predictedTile;
    float expiry = static_cast<float>(now - prediction.time) / expireTime;
    auto center = Vec2F(position) + Vec2F::filled(0.5f);
    auto size = Vec2F::filled(0.875f - expiry * 0.875f);
    auto poly = PolyF(RectF::withCenter(center, size));
    SpatialLogger::logPoly("world", poly, Color::Cyan.mix(Color::Red, expiry).toRgba());
    if (expiry >= 1.0f) {
      m_worldClient.dirtyCollision(RectI::withSize(position, { 1, 1 }));
      return true;
    } else {
      return false;
    }
  });
}

}
