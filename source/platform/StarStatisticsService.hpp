#pragma once

#include "StarJson.hpp"

namespace Star {

class StatisticsService;
using StatisticsServicePtr = SharedPtr<StatisticsService>;

class StatisticsService {
public:
  virtual ~StatisticsService() = default;

  [[nodiscard]] virtual bool initialized() const = 0;
  [[nodiscard]] virtual Maybe<String> error() const = 0;

  // The functions below aren't valid unless initialized() returns true and
  // error() is empty.

  // setStat should return false for stats or types that aren't known by the
  // service, without reporting an error.
  // By sending all stats to the StatisticsService, we can configure collection
  // of new stats entirely on the service, without any modifications to the game.
  [[nodiscard]] virtual bool setStat(String const& name, String const& type, Json const& value) = 0;
  [[nodiscard]] virtual Json getStat(String const& name, String const& type, Json def = {}) const = 0;

  // reportEvent should return false if the service doesn't handle this event.
  [[nodiscard]] virtual bool reportEvent(String const& name, Json const& fields) = 0;

  [[nodiscard]] virtual bool unlockAchievement(String const& name) = 0;
  [[nodiscard]] virtual StringSet achievementsUnlocked() const = 0;

  virtual void refresh() = 0;
  virtual void flush() = 0;
  [[nodiscard]] virtual bool reset() = 0;
};

}
