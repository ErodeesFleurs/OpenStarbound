#pragma once

#include "StarConfig.hpp"
#include "StarPlatformServices_pc.hpp"

import std;


namespace Star {

class SteamStatisticsSeSTEAM_CALLBACKrvice : public StatisticsService {
public:
  SteamStatisticsService(Ptr<PcPlatformServicesState> state);

  [[nodiscard]] auto initialized() const -> bool override;
  [[nodiscard]] auto error() const -> std::optional<String> override;

  auto setStat(String const& name, String const& type, Json const& value) -> bool override;
  [[nodiscard]] auto getStat(String const& name, String const& type, Json def = {}) const -> Json override;

  auto reportEvent(String const& name, Json const& fields) -> bool override;

  auto unlockAchievement(String const& name) -> bool override;
  [[nodiscard]] auto achievementsUnlocked() const -> StringSet override;

  void refresh() override;
  void flush() override;
  bool reset() override;

private:
  // STEAM_CALLBACK(SteamStatisticsService, onUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
  // STEAM_CALLBACK(SteamStatisticsService, onUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
  // STEAM_CALLBACK(SteamStatisticsService, onAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);

  std::uint64_t m_appId;
  bool m_initialized;
  std::optional<String> m_error;
};

}
