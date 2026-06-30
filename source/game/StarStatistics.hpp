#pragma once

#include "StarVersioningDatabase.hpp"
#include "StarStatisticsDatabase.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaRoot.hpp"
#include "StarStatisticsService.hpp"

namespace Star {

class Statistics;
using StatisticsPtr = SharedPtr<Statistics>;

class Statistics {
public:
  Statistics(String const& storageDirectory, VersioningDatabaseConstPtr versioningDatabase, StatisticsDatabaseConstPtr statisticsDatabase, LuaRootServices luaRootServices, StatisticsServicePtr service = {});

  void writeStatistics();

  [[nodiscard]] Json stat(String const& name, Json def = {}) const;
  [[nodiscard]] Maybe<String> statType(String const& name) const;
  [[nodiscard]] bool achievementUnlocked(String const& name) const;

  void recordEvent(String const& name, Json const& fields);
  [[nodiscard]] bool reset();

  void update();

private:
  struct Stat {
    [[nodiscard]] static Stat fromJson(Json const& json);
    [[nodiscard]] Json toJson() const;

    String type;
    Json value;
  };

  void processEvent(String const& name, Json const& fields);

  // setStat and unlockAchievement must be kept private as some platforms'
  // services don't implement the API calls these correspond to.
  void setStat(String const& name, String const& type, Json const& value);
  void unlockAchievement(String const& name);
  [[nodiscard]] bool checkAchievement(String const& achievementName);

  void readStatistics();
  void mergeServiceStatistics();

  [[nodiscard]] LuaCallbacks makeStatisticsCallbacks();

  template <typename Result = LuaValue, typename... V>
  [[nodiscard]] Maybe<Result> runStatScript(StringList const& scripts, Json const& config, String const& functionName, V&&... args);

  StatisticsServicePtr m_service;
  VersioningDatabaseConstPtr m_versioningDatabase;
  StatisticsDatabaseConstPtr m_statisticsDatabase;
  String m_storageDirectory;
  bool m_initialized;

  struct PendingEvent {
    String name;
    Json fields;
  };
  List<PendingEvent> m_pendingEvents;
  StringSet m_pendingAchievementChecks;

  StringMap<Stat> m_stats;
  StringSet m_achievements;

  LuaRootPtr m_luaRoot;
};

}
