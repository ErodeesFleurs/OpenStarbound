module;

#include "StarThread.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
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
  STEAM_CALLBACK(SteamStatisticsService, onUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
  STEAM_CALLBACK(SteamStatisticsService, onUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
  STEAM_CALLBACK(SteamStatisticsService, onAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);

  uint64_t m_appId;
  bool m_initialized;
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
