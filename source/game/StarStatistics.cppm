module;

import std;
import star.string;
import star.list;
import star.maybe;
import star.json;
import star.lua;

namespace Star {

class LuaCallbacks;
class LuaRoot;
using LuaRootPtr = std::shared_ptr<LuaRoot>;
using LuaRootConstPtr = std::shared_ptr<const LuaRoot>;
using LuaRootWeakPtr = std::weak_ptr<LuaRoot>;
using LuaRootConstWeakPtr = std::weak_ptr<const LuaRoot>;
using LuaRootUPtr = std::unique_ptr<LuaRoot>;
using LuaRootConstUPtr = std::unique_ptr<const LuaRoot>;
class StatisticsService;
using StatisticsServicePtr = std::shared_ptr<StatisticsService>;
using StatisticsServiceConstPtr = std::shared_ptr<const StatisticsService>;
using StatisticsServiceWeakPtr = std::weak_ptr<StatisticsService>;
using StatisticsServiceConstWeakPtr = std::weak_ptr<const StatisticsService>;
using StatisticsServiceUPtr = std::unique_ptr<StatisticsService>;
using StatisticsServiceConstUPtr = std::unique_ptr<const StatisticsService>;

class Statistics;
using StatisticsPtr = std::shared_ptr<Statistics>;
using StatisticsConstPtr = std::shared_ptr<const Statistics>;
using StatisticsWeakPtr = std::weak_ptr<Statistics>;
using StatisticsConstWeakPtr = std::weak_ptr<const Statistics>;
using StatisticsUPtr = std::unique_ptr<Statistics>;
using StatisticsConstUPtr = std::unique_ptr<const Statistics>;

class Statistics {
public:
  Statistics(String const& storageDirectory, StatisticsServicePtr service = {});

  void writeStatistics();

  Json stat(String const& name, Json def = {}) const;
  Maybe<String> statType(String const& name) const;
  bool achievementUnlocked(String const& name) const;

  void recordEvent(String const& name, Json const& fields);
  bool reset();

  void update();

private:
  struct Stat {
    static Stat fromJson(Json const& json);
    Json toJson() const;

    String type;
    Json value;
  };

  void processEvent(String const& name, Json const& fields);

  // setStat and unlockAchievement must be kept private as some platforms'
  // services don't implement the API calls these correspond to.
  void setStat(String const& name, String const& type, Json const& value);
  void unlockAchievement(String const& name);
  bool checkAchievement(String const& achievementName);

  void readStatistics();
  void mergeServiceStatistics();

  LuaCallbacks makeStatisticsCallbacks();

  template <typename Result = LuaValue, typename... V>
  Maybe<Result> runStatScript(StringList const& scripts, Json const& config, String const& functionName, V&&... args);

  StatisticsServicePtr m_service;
  String m_storageDirectory;
  bool m_initialized;

  List<std::pair<String, Json>> m_pendingEvents;
  StringSet m_pendingAchievementChecks;

  StringMap<Stat> m_stats;
  StringSet m_achievements;

  LuaRootPtr m_luaRoot;
};

}

export module star.statistics;

export namespace Star {
using ::Star::Statistics;
using ::Star::StatisticsPtr;
using ::Star::StatisticsConstPtr;
using ::Star::StatisticsWeakPtr;
using ::Star::StatisticsConstWeakPtr;
using ::Star::StatisticsUPtr;
using ::Star::StatisticsConstUPtr;
}
