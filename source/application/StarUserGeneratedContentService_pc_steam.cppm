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

STAR_CLASS(SteamUserGeneratedContentService);

class SteamUserGeneratedContentService final : public UserGeneratedContentService {
public:
  SteamUserGeneratedContentService(PcPlatformServicesStatePtr state);

  StringList subscribedContentIds() const override;
  Maybe<String> contentDownloadDirectory(String const& contentId) const override;
  UserGeneratedContentService::UGCState triggerContentDownload() override;

private:
  STEAM_CALLBACK(SteamUserGeneratedContentService, onDownloadResult, DownloadItemResult_t, m_callbackDownloadResult);

  HashMap<PublishedFileId_t, bool> m_currentDownloadState;

  bool m_checkedUGC;
};

}

export module star.user_generated_content_service_pc_steam;

export namespace Star {
  using ::Star::SteamUserGeneratedContentService;
  using ::Star::SteamUserGeneratedContentServicePtr;
  using ::Star::SteamUserGeneratedContentServiceConstPtr;
  using ::Star::SteamUserGeneratedContentServiceUPtr;
  using ::Star::SteamUserGeneratedContentServiceConstUPtr;
  using ::Star::SteamUserGeneratedContentServiceWeakPtr;
  using ::Star::SteamUserGeneratedContentServiceConstWeakPtr;
}
