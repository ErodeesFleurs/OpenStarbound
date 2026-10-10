module;

#include "StarThread.hpp"
#include "StarInputEvent.hpp"
#include "StarJson.hpp"
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "StarSteamDispatch.hpp"
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import star.platform_services_pc;

namespace Star {

STAR_CLASS(SteamStatisticsService);

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
  void onUserStatsReceived(UserStatsReceived_t* callback);
  void onUserStatsStored(UserStatsStored_t* callback);
  void onAchievementStored(UserAchievementStored_t* callback);

  PcPlatformServicesStatePtr m_state;
  SteamCallback<SteamStatisticsService, UserStatsReceived_t> m_callbackUserStatsReceived;
  SteamCallback<SteamStatisticsService, UserStatsStored_t> m_callbackUserStatsStored;
  SteamCallback<SteamStatisticsService, UserAchievementStored_t> m_callbackAchievementStored;

  uint64_t m_appId;
  bool m_initialized = false;
  Maybe<String> m_error;
};

}

export module star.statistics_service_pc_steam;

export namespace Star {
  using ::Star::SteamStatisticsService;
  using ::Star::SteamStatisticsServicePtr;
  using ::Star::SteamStatisticsServiceConstPtr;
  using ::Star::SteamStatisticsServiceUPtr;
  using ::Star::SteamStatisticsServiceConstUPtr;
  using ::Star::SteamStatisticsServiceWeakPtr;
  using ::Star::SteamStatisticsServiceConstWeakPtr;
}
