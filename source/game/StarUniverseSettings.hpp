#pragma once

#include "StarThread.hpp"
#include "StarAssets.hpp"
#include "StarJson.hpp"
#include "StarUuid.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class UniverseSettings;
using UniverseSettingsPtr = SharedPtr<UniverseSettings>;

struct PlaceDungeonFlagAction {
  String dungeonId;
  String targetInstance;
  Vec2I targetPosition;
};

using UniverseFlagAction = MVariant<PlaceDungeonFlagAction>;

[[nodiscard]] UniverseFlagAction parseUniverseFlagAction(Json const& json);

class UniverseSettings {
public:
  UniverseSettings(AssetsConstPtr assets);
  UniverseSettings(AssetsConstPtr assets, Json const& json);

  [[nodiscard]] Json toJson() const;

  [[nodiscard]] Uuid uuid() const;
  [[nodiscard]] StringSet flags() const;
  void setFlag(String const& flag);
  [[nodiscard]] Maybe<List<UniverseFlagAction>> pullPendingFlagActions();
  [[nodiscard]] List<UniverseFlagAction> currentFlagActions() const;
  [[nodiscard]] List<UniverseFlagAction> currentFlagActionsForInstanceWorld(String const& instanceName) const;
  void resetFlags();

private:
  void loadFlagActions(AssetsConstPtr assets);

  mutable Mutex m_lock;

  Uuid m_uuid;
  StringSet m_flags;

  StringMap<List<UniverseFlagAction>> m_flagActions;
  List<UniverseFlagAction> m_pendingFlagActions;
};

}
