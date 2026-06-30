#pragma once

#include "StarWorldTiles.hpp"
#include "StarTileModification.hpp"

namespace Star {

class WorldClient;

class StarWorldClientTilePrediction {
public:
  friend class WorldClient;

  explicit StarWorldClientTilePrediction(WorldClient& worldClient);

  void informTilePrediction(Vec2I const& pos, TileModification const& modification);
  [[nodiscard]] bool readNetTile(Vec2I const& pos, NetTile const& netTile, bool updateCollision = true);
  void expirePredictedTiles();

private:
  WorldClient& m_worldClient;

  HashMap<Vec2I, PredictedTile> m_predictedTiles;
  int m_modifiedTilePredictionTimeout = 0;
};

}
