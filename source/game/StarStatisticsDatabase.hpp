#pragma once

#include "StarConfig.hpp"
#include "StarJson.hpp"
#include "StarLruCache.hpp"
#include "StarString.hpp"
#include "StarThread.hpp"

import std;

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

  auto event(String const& eventName) const -> Ptr<StatEvent>;

  auto achievement(String const& name) const -> Ptr<Achievement>;
  auto allAchievements() const -> StringList;
  auto achievementsForStat(String const& statName) const -> StringList;

private:
  static auto readEvent(String const& path) -> Ptr<StatEvent>;
  static auto readAchievement(String const& path) -> Ptr<Achievement>;

  StringMap<String> m_eventPaths;
  StringMap<String> m_achievementPaths;
  StringMap<StringList> m_statAchievements;
  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, Ptr<StatEvent>> m_eventCache;
  mutable HashLruCache<String, Ptr<Achievement>> m_achievementCache;
};

}// namespace Star
