#pragma once

#include "StarPlatformServices_pc.hpp"

namespace Star {

class SteamStatisticsService : public StatisticsService {
public:
  SteamStatisticsService(PcPlatformServicesStatePtr state);

  [[nodiscard]] bool initialized() const override;
  [[nodiscard]] Maybe<String> error() const override;

  [[nodiscard]] bool setStat(String const& name, String const& type, Json const& value) override;
  [[nodiscard]] Json getStat(String const& name, String const& type, Json def = {}) const override;

  [[nodiscard]] bool reportEvent(String const& name, Json const& fields) override;

  [[nodiscard]] bool unlockAchievement(String const& name) override;
  [[nodiscard]] StringSet achievementsUnlocked() const override;

  void refresh() override;
  void flush() override;
  [[nodiscard]] bool reset() override;

private:
  STEAM_CALLBACK(SteamStatisticsService, onUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
  STEAM_CALLBACK(SteamStatisticsService, onUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
  STEAM_CALLBACK(SteamStatisticsService, onAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);

  uint64_t m_appId;
  bool m_initialized = false;
  Maybe<String> m_error;
};

}
