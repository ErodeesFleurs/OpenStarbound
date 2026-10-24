module;
import std;
import star.format;
import star.string;
import star.json;

import star.thread;
import star.ordered_map;
import star.lru_cache;

namespace Star {

class Assets;
class Root;

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
  [[nodiscard]] static std::expected<std::shared_ptr<StatisticsDatabase>, StarException> create(Root& root);

  [[nodiscard]] std::expected<std::shared_ptr<StatEvent>, StarException> event(String const& eventName) const;

  [[nodiscard]] std::expected<std::shared_ptr<Achievement>, StarException> achievement(String const& name) const;
  StringList allAchievements() const;
  StringList achievementsForStat(String const& statName) const;

private:
  explicit StatisticsDatabase(Root& root);
  [[nodiscard]] std::expected<void, StarException> load();
  [[nodiscard]] static std::expected<std::shared_ptr<StatEvent>, StarException> readEvent(Assets const& assets, String const& path);
  [[nodiscard]] static std::expected<std::shared_ptr<Achievement>, StarException> readAchievement(Assets const& assets, String const& path);

  Root& m_root;
  StringMap<String> m_eventPaths;
  StringMap<String> m_achievementPaths;
  StringMap<StringList> m_statAchievements;
  mutable std::mutex m_cacheMutex;
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
