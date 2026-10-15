module;
import std;
import star.string;
import star.json;

import star.thread;
import star.ordered_map;
import star.lru_cache;

namespace Star {

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

  std::shared_ptr<StatEvent> event(String const& eventName) const;

  std::shared_ptr<Achievement> achievement(String const& name) const;
  StringList allAchievements() const;
  StringList achievementsForStat(String const& statName) const;

private:
  static std::shared_ptr<StatEvent> readEvent(String const& path);
  static std::shared_ptr<Achievement> readAchievement(String const& path);

  StringMap<String> m_eventPaths;
  StringMap<String> m_achievementPaths;
  StringMap<StringList> m_statAchievements;
  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, std::shared_ptr<StatEvent>> m_eventCache;
  mutable HashLruCache<String, std::shared_ptr<Achievement>> m_achievementCache;
};

}

export module star.statistics_database;

export namespace Star {
  using ::Star::StatEvent;
  using ::Star::Achievement;
  using ::Star::StatisticsDatabase;
}
