#include "StarStatisticsDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"

namespace Star {

StatisticsDatabase::StatisticsDatabase(AssetsConstPtr assets) : m_assets(std::move(assets)), m_cacheMutex(), m_eventCache() {
  if (!m_assets)
    throw StarException("StatisticsDatabase requires assets service");

  auto& eventFiles = m_assets->scanExtension("event");
  m_assets->queueJsons(eventFiles);
  auto& achievementFiles = m_assets->scanExtension("achievement");
  m_assets->queueJsons(achievementFiles);

  for (auto& file : eventFiles) {
    try {
      String name = m_assets->json(file).getString("eventName");
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
      Json achievement = m_assets->json(file);
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

StatEventPtr StatisticsDatabase::event(String const& name) const {
  MutexLocker locker(m_cacheMutex);
  return m_eventCache.get(name, [this](String const& name) -> StatEventPtr {
      if (auto path = m_eventPaths.maybe(name))
        return readEvent(*path);
      return {};
    });
}

AchievementPtr StatisticsDatabase::achievement(String const& name) const {
  MutexLocker locker(m_cacheMutex);
  return m_achievementCache.get(name, [this](String const& name) -> AchievementPtr {
      if (auto path = m_achievementPaths.maybe(name))
        return readAchievement(*path);
      return {};
    });
}

StringList StatisticsDatabase::allAchievements() const {
  return m_achievementPaths.keys();
}

StringList StatisticsDatabase::achievementsForStat(String const& statName) const {
  return m_statAchievements.value(statName);
}

StatEventPtr StatisticsDatabase::readEvent(String const& path) const {
  Json config = m_assets->json(path);

  return make_shared<StatEvent>(StatEvent {
      config.getString("eventName"),
      jsonToStringList(config.get("scripts")),
      config
    });
}

AchievementPtr StatisticsDatabase::readAchievement(String const& path) const {
  Json config = m_assets->json(path);

  return make_shared<Achievement>(Achievement {
      config.getString("name"),
      jsonToStringList(config.get("triggers")),
      jsonToStringList(config.get("scripts")),
      config
    });
}

}
