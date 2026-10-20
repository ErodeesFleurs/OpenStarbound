module;

import std;
import star.string;
import star.list;
import star.json;
import star.lua;

namespace Star {

class LuaCallbacks;
class LuaRoot;
class StatisticsService;
class Root;

class Statistics {
public:
  Statistics(Root& root, String const& storageDirectory, std::shared_ptr<StatisticsService> service = {});

  void writeStatistics();

  Json stat(String const& name, Json def = {}) const;
  std::optional<String> statType(String const& name) const;
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
  std::optional<Result> runStatScript(StringList const& scripts, Json const& config, String const& functionName, V&&... args);

  Root& m_root;
  std::shared_ptr<StatisticsService> m_service;
  String m_storageDirectory;
  bool m_initialized;

  List<std::pair<String, Json>> m_pendingEvents;
  StringSet m_pendingAchievementChecks;

  StringMap<Stat> m_stats;
  StringSet m_achievements;

  std::shared_ptr<LuaRoot> m_luaRoot;
};

}

export module star.statistics;

export namespace Star {
using ::Star::Statistics;
}
