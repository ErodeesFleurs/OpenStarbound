import star.json;
#include "StarJsonExtra.hpp"
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.format;
import star.version;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.statistics_database;

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

std::shared_ptr<StatEvent> StatisticsDatabase::event(String const& name) const {
  MutexLocker locker(m_cacheMutex);
  return m_eventCache.get(name, [this](String const& name) -> std::shared_ptr<StatEvent> {
      if (auto path = m_eventPaths.maybe(name))
        return readEvent(*path);
      return {};
    });
}

std::shared_ptr<Achievement> StatisticsDatabase::achievement(String const& name) const {
  MutexLocker locker(m_cacheMutex);
  return m_achievementCache.get(name, [this](String const& name) -> std::shared_ptr<Achievement> {
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

std::shared_ptr<StatEvent> StatisticsDatabase::readEvent(String const& path) {
  auto assets = Root::singleton().assets();
  Json config = assets->json(path);

  return std::make_shared<StatEvent>(StatEvent {
      config.getString("eventName"),
      jsonToStringList(config.get("scripts")),
      config
    });
}

std::shared_ptr<Achievement> StatisticsDatabase::readAchievement(String const& path) {
  auto assets = Root::singleton().assets();
  Json config = assets->json(path);

  return std::make_shared<Achievement>(Achievement {
      config.getString("name"),
      jsonToStringList(config.get("triggers")),
      jsonToStringList(config.get("scripts")),
      config
    });
}

}
