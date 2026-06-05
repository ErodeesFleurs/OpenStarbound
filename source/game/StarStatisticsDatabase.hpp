#pragma once

#include "StarRoot.hpp"
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
