module;
import std;
import star.string;
import star.list;
import star.variant;
import star.json;

import star.thread;
import star.data_stream;
import star.uuid;
import star.vector;
import star.game_types;

namespace Star {

class Root;

struct PlaceDungeonFlagAction {
  String dungeonId;
  String targetInstance;
  Vec2I targetPosition;
};

typedef MVariant<PlaceDungeonFlagAction> UniverseFlagAction;

UniverseFlagAction parseUniverseFlagAction(Json const& json);

class UniverseSettings {
public:
  explicit UniverseSettings(Root& root);
  UniverseSettings(Root& root, Json const& json);

  Json toJson() const;

  Uuid uuid() const;
  StringSet flags() const;
  void setFlag(String const& flag);
  std::optional<List<UniverseFlagAction>> pullPendingFlagActions();
  List<UniverseFlagAction> currentFlagActions() const;
  List<UniverseFlagAction> currentFlagActionsForInstanceWorld(String const& instanceName) const;
  void resetFlags();

private:
  void loadFlagActions();

  Root& m_root;

  mutable Mutex m_lock;

  Uuid m_uuid;
  StringSet m_flags;

  StringMap<List<UniverseFlagAction>> m_flagActions;
  List<UniverseFlagAction> m_pendingFlagActions;
};

}

export module star.universe_settings;

export namespace Star {
  using ::Star::PlaceDungeonFlagAction;
  using ::Star::UniverseFlagAction;
  using ::Star::parseUniverseFlagAction;
  using ::Star::UniverseSettings;
}
