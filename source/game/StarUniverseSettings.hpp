#pragma once

#include "StarJson.hpp"
#include "StarThread.hpp"
#include "StarUuid.hpp"

import std;

namespace Star {

struct PlaceDungeonFlagAction {
  String dungeonId;
  String targetInstance;
  Vec2I targetPosition;
};

using UniverseFlagAction = MVariant<PlaceDungeonFlagAction>;

auto parseUniverseFlagAction(Json const& json) -> UniverseFlagAction;

class UniverseSettings {
public:
  UniverseSettings();
  UniverseSettings(Json const& json);

  auto toJson() const -> Json;

  auto uuid() const -> Uuid;
  auto flags() const -> StringSet;
  void setFlag(String const& flag);
  auto pullPendingFlagActions() -> std::optional<List<UniverseFlagAction>>;
  auto currentFlagActions() const -> List<UniverseFlagAction>;
  auto currentFlagActionsForInstanceWorld(String const& instanceName) const -> List<UniverseFlagAction>;
  void resetFlags();

private:
  void loadFlagActions();

  mutable Mutex m_lock;

  Uuid m_uuid;
  StringSet m_flags;

  StringMap<List<UniverseFlagAction>> m_flagActions;
  List<UniverseFlagAction> m_pendingFlagActions;
};

}// namespace Star
