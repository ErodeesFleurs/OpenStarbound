#include "StarUniverseSettings.hpp"
#include "StarAlgorithm.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"

namespace Star {

UniverseFlagAction parseUniverseFlagAction(Json const& json) {
  String actionType = json.getString("type");

  if (actionType == "placeDungeon") {
    PlaceDungeonFlagAction res;
    res.dungeonId = json.getString("dungeonId");
    res.targetInstance = json.getString("targetInstance");
    res.targetPosition = jsonToVec2I(json.get("targetPosition"));
    return res;
  } else {
    throw StarException(strf("Unsupported universe flag action type {}", actionType));
  }
}

UniverseSettings::UniverseSettings(AssetsConstPtr assets) {
  loadFlagActions(requireServiceValueAs<StarException>(std::move(assets), "UniverseSettings", "assets"));
}

UniverseSettings::UniverseSettings(AssetsConstPtr assets, Json const& json) {
  m_uuid = Uuid(json.getString("uuid"));
  m_flags = jsonToStringSet(json.get("flags"));

  loadFlagActions(requireServiceValueAs<StarException>(std::move(assets), "UniverseSettings", "assets"));
}

Json UniverseSettings::toJson() const {
  MutexLocker locker(m_lock);

  return JsonObject{
      {"uuid", m_uuid.hex()},
      {"flags", jsonFromStringSet(m_flags)}
    };
}

Uuid UniverseSettings::uuid() const {
  return m_uuid;
}

StringSet UniverseSettings::flags() const {
  MutexLocker locker(m_lock);

  return m_flags;
}

void UniverseSettings::setFlag(String const& flag) {
  MutexLocker locker(m_lock);

  if (m_flags.add(flag)) {
    Logger::info("Universe flags set to {}", m_flags);
    if (auto flagActions = m_flagActions.maybe(flag))
      m_pendingFlagActions.appendAll(*flagActions);
    else
      Logger::info("No actions configured for universe flag {}", flag);
  }
}

Maybe<List<UniverseFlagAction>> UniverseSettings::pullPendingFlagActions() {
  if (!m_pendingFlagActions.empty())
    return take(m_pendingFlagActions);
  return {};
}

List<UniverseFlagAction> UniverseSettings::currentFlagActions() const {
  List<UniverseFlagAction> res;
  for (String flag : m_flags) {
    if (auto flagActions = m_flagActions.maybe(flag))
      res.appendAll(*flagActions);
  }
  return res;
}

List<UniverseFlagAction> UniverseSettings::currentFlagActionsForInstanceWorld(String const& instanceName) const {
  List<UniverseFlagAction> res;
  for (String flag : m_flags) {
    for (auto flagAction : m_flagActions.maybe(flag).value()) {
      if (flagAction.is<PlaceDungeonFlagAction>() && flagAction.get<PlaceDungeonFlagAction>().targetInstance == instanceName)
        res.append(flagAction);
    }
  }
  return res;
}

void UniverseSettings::resetFlags() {
  m_flags.clear();
}

void UniverseSettings::loadFlagActions(AssetsConstPtr assets) {
  assets = requireServiceValueAs<StarException>(std::move(assets), "UniverseSettings", "assets");

  m_flagActions.clear();

  Json flagsConfig = assets->json("/universeflags.config");
  for (auto const& [flagName, flagConfig] : flagsConfig.iterateObject()) {
    List<UniverseFlagAction> actions;
    for (auto actionConfig : flagConfig.get("actions").iterateArray())
      actions.append(parseUniverseFlagAction(actionConfig));
    m_flagActions[flagName] = actions;
  }
}

}
