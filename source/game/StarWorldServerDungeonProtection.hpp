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

  WorldServerDungeonProtection() = default;
  explicit WorldServerDungeonProtection(WorldServer* worldServer);

  bool isTileProtected(Vec2I const& pos) const;
  bool getTileProtection(DungeonId dungeonId) const;
  void setTileProtection(DungeonId dungeonId, bool isProtected);
  size_t setTileProtection(List<DungeonId> const& dungeonIds, bool isProtected);
  void setTileProtectionEnabled(bool enabled);
  void setDungeonId(RectI const& tileRegion, DungeonId dungeonId);
  DungeonId dungeonId(Vec2I const& pos) const;
  void setDungeonGravity(DungeonId dungeonId, Maybe<float> gravity);
  void setDungeonBreathable(DungeonId dungeonId, Maybe<bool> breathable);
  bool isPlayerModified(RectI const& region) const;

private:
  WorldServer* m_worldServer = nullptr;

  StableHashSet<DungeonId> m_protectedDungeonIds;
  bool m_tileProtectionEnabled = true;
  HashMap<DungeonId, float> m_dungeonIdGravity;
  HashMap<DungeonId, bool> m_dungeonIdBreathable;
  bool m_generatingDungeon = false;
};

}
