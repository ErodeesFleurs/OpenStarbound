#pragma once

#include "StarSet.hpp"
#include "StarMap.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class WorldServer;

class WorldServerDungeonProtection {
public:
  friend class WorldServer;

  explicit WorldServerDungeonProtection(WorldServer& worldServer);

  [[nodiscard]] bool isTileProtected(Vec2I const& pos) const;
  [[nodiscard]] bool getTileProtection(DungeonId dungeonId) const;
  void setTileProtection(DungeonId dungeonId, bool isProtected);
  [[nodiscard]] size_t setTileProtection(List<DungeonId> const& dungeonIds, bool isProtected);
  void setTileProtectionEnabled(bool enabled);
  void setDungeonId(RectI const& tileRegion, DungeonId dungeonId);
  [[nodiscard]] DungeonId dungeonId(Vec2I const& pos) const;
  void setDungeonGravity(DungeonId dungeonId, Maybe<float> gravity);
  void setDungeonBreathable(DungeonId dungeonId, Maybe<bool> breathable);
  [[nodiscard]] bool isPlayerModified(RectI const& region) const;

private:
  WorldServer& m_worldServer;

  StableHashSet<DungeonId> m_protectedDungeonIds;
  bool m_tileProtectionEnabled = true;
  HashMap<DungeonId, float> m_dungeonIdGravity;
  HashMap<DungeonId, bool> m_dungeonIdBreathable;
  bool m_generatingDungeon = false;
};

}
