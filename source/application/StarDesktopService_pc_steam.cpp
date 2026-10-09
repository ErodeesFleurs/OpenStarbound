#include "StarThread.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import star.platform_services_pc;
import star.desktop_service_pc_steam;

namespace Star {

SteamDesktopService::SteamDesktopService(PcPlatformServicesStatePtr state)
  : m_state(std::move(state)) {}

void SteamDesktopService::openUrl(String const& url) {
  SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(SteamAPI_SteamFriends(), url.utf8Ptr(),
      k_EActivateGameOverlayToWebPageMode_Default);
}

}
