#pragma once

#include "StarJson.hpp"

import std;

namespace Star {

class StatisticsService {
public:
  virtual ~StatisticsService() = default;

  [[nodiscard]] virtual auto initialized() const -> bool = 0;
  [[nodiscard]] virtual auto error() const -> std::optional<String> = 0;

  // The functions below aren't valid unless initialized() returns true and
  // error() is empty.

  // setStat should return false for stats or types that aren't known by the
  // service, without reporting an error.
  // By sending all stats to the StatisticsService, we can configure collection
  // of new stats entirely on the service, without any modifications to the game.
  virtual auto setStat(String const& name, String const& type, Json const& value) -> bool = 0;
  [[nodiscard]] virtual auto getStat(String const& name, String const& type, Json def = {}) const -> Json = 0;

  // reportEvent should return false if the service doesn't handle this event.
  virtual auto reportEvent(String const& name, Json const& fields) -> bool = 0;

  virtual auto unlockAchievement(String const& name) -> bool = 0;
  [[nodiscard]] virtual auto achievementsUnlocked() const -> StringSet = 0;

  virtual void refresh() = 0;
  virtual void flush() = 0;
  virtual auto reset() -> bool = 0;
};

}// namespace Star
