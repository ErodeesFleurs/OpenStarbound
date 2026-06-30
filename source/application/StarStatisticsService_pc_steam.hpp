#pragma once

#include "StarPlatformServices_pc.hpp"

namespace Star {

class SteamStatisticsService;
using SteamStatisticsServicePtr = shared_ptr<SteamStatisticsService>;
using SteamStatisticsServiceConstPtr = shared_ptr<SteamStatisticsService const>;
using SteamStatisticsServiceWeakPtr = weak_ptr<SteamStatisticsService>;
using SteamStatisticsServiceConstWeakPtr = weak_ptr<SteamStatisticsService const>;
using SteamStatisticsServiceUPtr = unique_ptr<SteamStatisticsService>;
using SteamStatisticsServiceConstUPtr = unique_ptr<SteamStatisticsService const>;

class SteamStatisticsService : public StatisticsService {
public:
  SteamStatisticsService(PcPlatformServicesStatePtr state);

  bool initialized() const override;
  Maybe<String> error() const override;

  bool setStat(String const& name, String const& type, Json const& value) override;
  Json getStat(String const& name, String const& type, Json def = {}) const override;

  bool reportEvent(String const& name, Json const& fields) override;

  bool unlockAchievement(String const& name) override;
  StringSet achievementsUnlocked() const override;

  void refresh() override;
  void flush() override;
  bool reset() override;

private:
  STEAM_CALLBACK(SteamStatisticsService, onUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
  STEAM_CALLBACK(SteamStatisticsService, onUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
  STEAM_CALLBACK(SteamStatisticsService, onAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);

  uint64_t m_appId;
  bool m_initialized;
  Maybe<String> m_error;
};

}
