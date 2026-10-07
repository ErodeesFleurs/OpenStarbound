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

STAR_CLASS(SteamDesktopService);

class SteamDesktopService final : public DesktopService {
public:
  SteamDesktopService(PcPlatformServicesStatePtr state);

  void openUrl(String const& url) override;
};

}

export module star.desktop_service_pc_steam;

export namespace Star {
  using ::Star::SteamDesktopService;
  using ::Star::SteamDesktopServicePtr;
  using ::Star::SteamDesktopServiceConstPtr;
  using ::Star::SteamDesktopServiceUPtr;
  using ::Star::SteamDesktopServiceConstUPtr;
  using ::Star::SteamDesktopServiceWeakPtr;
  using ::Star::SteamDesktopServiceConstWeakPtr;
}
