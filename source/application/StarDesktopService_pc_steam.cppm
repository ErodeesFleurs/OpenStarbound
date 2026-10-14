module;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#endif
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import std;
import star.thread;
import star.input_event;
import star.application;
import star.string;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;


import star.platform_services_pc;

namespace Star {

class SteamDesktopService;
using SteamDesktopServicePtr = std::shared_ptr<SteamDesktopService>;
using SteamDesktopServiceConstPtr = std::shared_ptr<SteamDesktopService const>;
using SteamDesktopServiceWeakPtr = std::weak_ptr<SteamDesktopService>;
using SteamDesktopServiceConstWeakPtr = std::weak_ptr<SteamDesktopService const>;
using SteamDesktopServiceUPtr = std::unique_ptr<SteamDesktopService>;
using SteamDesktopServiceConstUPtr = std::unique_ptr<SteamDesktopService const>;

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
