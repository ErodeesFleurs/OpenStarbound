#include "StarWorldClientTilePrediction.hpp"
#include "StarWorldClient.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarLogging.hpp"

namespace Star {

StarWorldClientTilePrediction::StarWorldClientTilePrediction(WorldClient& worldClient)
  : m_worldClient(worldClient) {}

void StarWorldClientTilePrediction::informTilePrediction(Vec2I const& pos, TileModification const& modification) {
  auto now = Time::monotonicMilliseconds();
  auto& predictedTile = m_predictedTiles[pos];
  predictedTile.time = now;
  if (auto placeMaterial = modification.ptr<PlaceMaterial>()) {
    if (placeMaterial->layer == TileLayer::Foreground) {
      auto materialDatabase = m_worldClient.m_materialDatabase;
      if (!materialDatabase->isCascadingFallingMaterial(placeMaterial->material)
          && !materialDatabase->isFallingMaterial(placeMaterial->material)) {
        predictedTile.foreground = placeMaterial->material;
        predictedTile.foregroundHueShift = placeMaterial->materialHueShift;
      } else {
        predictedTile.foreground = StructureMaterialId;
      }
      if (placeMaterial->collisionOverride != TileCollisionOverride::None)
        predictedTile.collision = collisionKindFromOverride(placeMaterial->collisionOverride);
      else
        predictedTile.collision = materialDatabase->materialCollisionKind(placeMaterial->material);
      m_worldClient.dirtyCollision(RectI::withSize(pos, {1, 1}));
    } else {
      predictedTile.background = placeMaterial->material;
      predictedTile.backgroundHueShift = placeMaterial->materialHueShift;
    }
  }
  else if (auto placeMod = modification.ptr<PlaceMod>()) {
    if (placeMod->layer == TileLayer::Foreground)
      predictedTile.foregroundMod = placeMod->mod;
    else
      predictedTile.backgroundMod = placeMod->mod;
  }
  else if (auto placeColor = modification.ptr<PlaceMaterialColor>()) {
    if (placeColor->layer == TileLayer::Foreground)
      predictedTile.foregroundColorVariant = placeColor->color;
    else
      predictedTile.backgroundColorVariant = placeColor->color;
  }
  else if (auto placeLiquid = modification.ptr<PlaceLiquid>()) {
    if (!predictedTile.liquid || predictedTile.liquid->liquid != placeLiquid->liquid)
      predictedTile.liquid = LiquidLevel(placeLiquid->liquid, placeLiquid->liquidLevel);
    else
      predictedTile.liquid->level += placeLiquid->liquidLevel;
  }
}

bool StarWorldClientTilePrediction::readNetTile(Vec2I const& pos, NetTile const& netTile, bool updateCollision) {
  ClientTile* tile = m_worldClient.m_tileArray->modifyTile(pos);
  if (!tile)
    return false;

  if (!m_predictedTiles.empty()) {
    auto findPrediction = m_predictedTiles.find(pos);
    if (findPrediction != m_predictedTiles.end()) {
      auto& predictedTile = findPrediction->second;

      if (predictedTile.collision && *predictedTile.collision == netTile.collision)
        predictedTile.collision.reset();
      if (predictedTile.foreground && (*predictedTile.foreground == StructureMaterialId || *predictedTile.foreground == netTile.foreground))
        predictedTile.foreground.reset();
      if (predictedTile.foregroundMod && *predictedTile.foregroundMod == netTile.foregroundMod)
        predictedTile.foregroundMod.reset();
      if (predictedTile.foregroundHueShift && *predictedTile.foregroundHueShift == netTile.foregroundHueShift)
        predictedTile.foregroundHueShift.reset();
      if (predictedTile.foregroundModHueShift && *predictedTile.foregroundModHueShift == netTile.foregroundModHueShift)
        predictedTile.foregroundModHueShift.reset();

      if (predictedTile.background && *predictedTile.background == netTile.background)
        predictedTile.background.reset();
      if (predictedTile.backgroundMod && *predictedTile.backgroundMod == netTile.backgroundMod)
        predictedTile.backgroundMod.reset();
      if (predictedTile.backgroundHueShift && *predictedTile.backgroundHueShift == netTile.backgroundHueShift)
        predictedTile.backgroundHueShift.reset();
      if (predictedTile.backgroundModHueShift && *predictedTile.backgroundModHueShift == netTile.backgroundModHueShift)
        predictedTile.backgroundModHueShift.reset();

      if (!predictedTile)
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
