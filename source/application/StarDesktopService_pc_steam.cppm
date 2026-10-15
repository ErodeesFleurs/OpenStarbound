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

class SteamDesktopService final : public DesktopService {
public:
  SteamDesktopService(std::shared_ptr<PcPlatformServicesState> state);

  void openUrl(String const& url) override;

private:
  std::shared_ptr<PcPlatformServicesState> m_state;
};

}

export module star.desktop_service_pc_steam;

export namespace Star {
  using ::Star::SteamDesktopService;
}
