module;

#include "StarThread.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarString.hpp"
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;

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

private:
  PcPlatformServicesStatePtr m_state;
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
