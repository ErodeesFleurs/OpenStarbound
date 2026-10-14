module;
import std;
import star.string;
import star.json;

import star.thread;
import star.ordered_map;
import star.lru_cache;

namespace Star {

struct StatEvent;
using StatEventPtr = std::shared_ptr<StatEvent>;
using StatEventConstPtr = std::shared_ptr<const StatEvent>;
using StatEventWeakPtr = std::weak_ptr<StatEvent>;
using StatEventConstWeakPtr = std::weak_ptr<const StatEvent>;
using StatEventUPtr = std::unique_ptr<StatEvent>;
using StatEventConstUPtr = std::unique_ptr<const StatEvent>;
struct Achievement;
using AchievementPtr = std::shared_ptr<Achievement>;
using AchievementConstPtr = std::shared_ptr<const Achievement>;
using AchievementWeakPtr = std::weak_ptr<Achievement>;
using AchievementConstWeakPtr = std::weak_ptr<const Achievement>;
using AchievementUPtr = std::unique_ptr<Achievement>;
using AchievementConstUPtr = std::unique_ptr<const Achievement>;
class StatisticsDatabase;
using StatisticsDatabasePtr = std::shared_ptr<StatisticsDatabase>;
using StatisticsDatabaseConstPtr = std::shared_ptr<const StatisticsDatabase>;
using StatisticsDatabaseWeakPtr = std::weak_ptr<StatisticsDatabase>;
using StatisticsDatabaseConstWeakPtr = std::weak_ptr<const StatisticsDatabase>;
using StatisticsDatabaseUPtr = std::unique_ptr<StatisticsDatabase>;
using StatisticsDatabaseConstUPtr = std::unique_ptr<const StatisticsDatabase>;

struct StatEvent {
  String eventName;
  StringList scripts;
  Json config;
};

struct Achievement {
  String name;
  StringList triggers;
  StringList scripts;
  Json config;
};

class StatisticsDatabase {
public:
  StatisticsDatabase();

  StatEventPtr event(String const& eventName) const;

  AchievementPtr achievement(String const& name) const;
  StringList allAchievements() const;
  StringList achievementsForStat(String const& statName) const;

private:
  static StatEventPtr readEvent(String const& path);
  static AchievementPtr readAchievement(String const& path);

  StringMap<String> m_eventPaths;
  StringMap<String> m_achievementPaths;
  StringMap<StringList> m_statAchievements;
  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, StatEventPtr> m_eventCache;
  mutable HashLruCache<String, AchievementPtr> m_achievementCache;
};

}

export module star.statistics_database;

export namespace Star {
  using ::Star::StatEvent;
  using ::Star::StatEventPtr;
  using ::Star::StatEventConstPtr;
  using ::Star::Achievement;
  using ::Star::AchievementPtr;
  using ::Star::AchievementConstPtr;
  using ::Star::StatisticsDatabase;
  using ::Star::StatisticsDatabasePtr;
  using ::Star::StatisticsDatabaseConstPtr;
}
