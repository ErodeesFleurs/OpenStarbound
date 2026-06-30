#pragma once

#include "StarVector.hpp"
#include "StarMaybe.hpp"

namespace Star {

class WorldServer;

class WorldServerSpawnFinder {
public:
  friend class WorldServer;

  explicit WorldServerSpawnFinder(WorldServer& worldServer);

  [[nodiscard]] Vec2F findPlayerStart(Maybe<Vec2F> firstTry = {});
  [[nodiscard]] Vec2F findPlayerSpaceStart(float targetX);
  void setPlayerStart(Vec2F const& startPosition, bool respawnInWorld = false);
  [[nodiscard]] Vec2F playerStart() const;
  [[nodiscard]] bool adjustPlayerStart() const;
  [[nodiscard]] bool respawnInWorld() const;

private:
  WorldServer& m_worldServer;

  Vec2F m_playerStart;
  bool m_adjustPlayerStart = true;
  bool m_respawnInWorld = false;
};

}
