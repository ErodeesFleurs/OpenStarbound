module;

import std;
import star.format;
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
  [[nodiscard]] static std::expected<std::shared_ptr<Statistics>, StarException> create(Root& root, String const& storageDirectory, std::shared_ptr<StatisticsService> service = {});

  [[nodiscard]] std::expected<void, StarException> writeStatistics();

  Json stat(String const& name, Json def = {}) const;
  std::optional<String> statType(String const& name) const;
  bool achievementUnlocked(String const& name) const;

  void recordEvent(String const& name, Json const& fields);
  bool reset();

  [[nodiscard]] std::expected<void, StarException> update();

private:
  struct Stat {
    [[nodiscard]] static std::expected<Stat, StarException> fromJson(Json const& json);
    Json toJson() const;

    String type;
    Json value;
  };

  Statistics(Root& root, String const& storageDirectory, std::shared_ptr<StatisticsService> service);
  [[nodiscard]] std::expected<void, StarException> processEvent(String const& name, Json const& fields);

  // setStat and unlockAchievement must be kept private as some platforms'
  // services don't implement the API calls these correspond to.
  [[nodiscard]] std::expected<void, StarException> setStat(String const& name, String const& type, Json const& value);
  [[nodiscard]] std::expected<void, StarException> unlockAchievement(String const& name);
  [[nodiscard]] std::expected<bool, StarException> checkAchievement(String const& achievementName);

  [[nodiscard]] std::expected<void, StarException> readStatistics();
  [[nodiscard]] std::expected<void, StarException> mergeServiceStatistics();

  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeStatisticsCallbacks();

  template <typename Result = LuaValue, typename... V>
  [[nodiscard]] std::expected<std::optional<Result>, StarException> runStatScript(StringList const& scripts, Json const& config, String const& functionName, V&&... args);

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
