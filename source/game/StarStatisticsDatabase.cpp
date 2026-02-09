#include "StarStatisticsDatabase.hpp"

#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

StatisticsDatabase::StatisticsDatabase() : m_cacheMutex(), m_eventCache() {
  auto assets = Root::singleton().assets();

  auto& eventFiles = assets->scanExtension("event");
  assets->queueJsons(eventFiles);
  auto& achievementFiles = assets->scanExtension("achievement");
  assets->queueJsons(achievementFiles);

  for (auto& file : eventFiles) {
    try {
      String name = assets->json(file).getString("eventName");
      if (m_eventPaths.contains(name))
        Logger::error("Event {} defined twice, second time from {}", name, file);
      else
        m_eventPaths[name] = file;
    } catch (std::exception const& e) {
      Logger::error("Error loading event file {}: {}", file, outputException(e, true));
    }
  }

  for (auto& file : achievementFiles) {
    try {
      Json achievement = assets->json(file);
      String name = achievement.getString("name");
      if (m_achievementPaths.contains(name))
        Logger::error("Achievement {} defined twice, second time from {}", name, file);
      else
        m_achievementPaths[name] = file;

      for (Json const& stat : achievement.getArray("triggers", {})) {
        m_statAchievements[stat.toString()].append(name);
      }
    } catch (std::exception const& e) {
      Logger::error("Error loading achievement file {}: {}", file, outputException(e, true));
    }
  }
}

auto StatisticsDatabase::event(String const& name) const -> Ptr<StatEvent> {
  MutexLocker locker(m_cacheMutex);
  return m_eventCache.get(name, [this](String const& name) -> Ptr<StatEvent> {
    if (auto path = m_eventPaths.maybe(name))
      return readEvent(*path);
    return {};
  });
}

auto StatisticsDatabase::achievement(String const& name) const -> Ptr<Achievement> {
  MutexLocker locker(m_cacheMutex);
  return m_achievementCache.get(name, [this](String const& name) -> Ptr<Achievement> {
    if (auto path = m_achievementPaths.maybe(name))
      return readAchievement(*path);
    return {};
  });
}

auto StatisticsDatabase::allAchievements() const -> StringList {
  return m_achievementPaths.keys();
}

auto StatisticsDatabase::achievementsForStat(String const& statName) const -> StringList {
  return m_statAchievements.value(statName);
}

auto StatisticsDatabase::readEvent(String const& path) -> Ptr<StatEvent> {
  auto assets = Root::singleton().assets();
  Json config = assets->json(path);

  return std::make_shared<StatEvent>(StatEvent{
    .eventName = config.getString("eventName"),
    .scripts = jsonToStringList(config.get("scripts")),
    .config = config});
}

auto StatisticsDatabase::readAchievement(String const& path) -> Ptr<Achievement> {
  auto assets = Root::singleton().assets();
  Json config = assets->json(path);

  return std::make_shared<Achievement>(Achievement{
    .name = config.getString("name"),
    .triggers = jsonToStringList(config.get("triggers")),
    .scripts = jsonToStringList(config.get("scripts")),
    .config = config});
}

}// namespace Star
