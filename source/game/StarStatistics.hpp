#pragma once

#include "StarConfig.hpp"
#include "StarLua.hpp"
#include "StarLuaRoot.hpp"
#include "StarStatisticsService.hpp"

import std;

namespace Star {

class Statistics {
public:
  Statistics(String const& storageDirectory, Ptr<StatisticsService> service = {});

  void writeStatistics();

  [[nodiscard]] auto stat(String const& name, Json def = {}) const -> Json;
  [[nodiscard]] auto statType(String const& name) const -> std::optional<String>;
  [[nodiscard]] auto achievementUnlocked(String const& name) const -> bool;

  void recordEvent(String const& name, Json const& fields);
  auto reset() -> bool;

  void update();

private:
  struct Stat {
    static auto fromJson(Json const& json) -> Stat;
    [[nodiscard]] auto toJson() const -> Json;

    String type;
    Json value;
  };

  void processEvent(String const& name, Json const& fields);

  // setStat and unlockAchievement must be kept private as some platforms'
  // services don't implement the API calls these correspond to.
  void setStat(String const& name, String const& type, Json const& value);
  void unlockAchievement(String const& name);
  auto checkAchievement(String const& achievementName) -> bool;

  void readStatistics();
  void mergeServiceStatistics();

  auto makeStatisticsCallbacks() -> LuaCallbacks;

  template <typename Result = LuaValue, typename... V>
  auto runStatScript(StringList const& scripts, Json const& config, String const& functionName, V&&... args) -> std::optional<Result>;

  Ptr<StatisticsService> m_service;
  String m_storageDirectory;
  bool m_initialized;

  List<std::pair<String, Json>> m_pendingEvents;
  StringSet m_pendingAchievementChecks;

  StringMap<Stat> m_stats;
  StringSet m_achievements;

  Ptr<LuaRoot> m_luaRoot;
};

}// namespace Star
