#pragma once

#include "StarAssets.hpp"
#include "StarLruCache.hpp"

namespace Star {

struct StatEvent;
using StatEventPtr = SharedPtr<StatEvent>;
struct Achievement;
using AchievementPtr = SharedPtr<Achievement>;
class StatisticsDatabase;
using StatisticsDatabasePtr = SharedPtr<StatisticsDatabase>;
using StatisticsDatabaseConstPtr = SharedPtr<StatisticsDatabase const>;

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
  StatisticsDatabase(AssetsConstPtr assets);

  [[nodiscard]] StatEventPtr event(String const& eventName) const;

  [[nodiscard]] AchievementPtr achievement(String const& name) const;
  [[nodiscard]] StringList allAchievements() const;
  [[nodiscard]] StringList achievementsForStat(String const& statName) const;

private:
  [[nodiscard]] StatEventPtr readEvent(String const& path) const;
  [[nodiscard]] AchievementPtr readAchievement(String const& path) const;

  AssetsConstPtr m_assets;
  StringMap<String> m_eventPaths;
  StringMap<String> m_achievementPaths;
  StringMap<StringList> m_statAchievements;
  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, StatEventPtr> m_eventCache;
  mutable HashLruCache<String, AchievementPtr> m_achievementCache;
};

}
