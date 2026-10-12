module;

#include "StarThread.hpp"
import star.input_event;
import star.application;
#include "StarString.hpp"
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

STAR_CLASS(SteamUserGeneratedContentService);

class SteamUserGeneratedContentService final : public UserGeneratedContentService {
public:
  SteamUserGeneratedContentService(PcPlatformServicesStatePtr state);

  StringList subscribedContentIds() const override;
  Maybe<String> contentDownloadDirectory(String const& contentId) const override;
  UserGeneratedContentService::UGCState triggerContentDownload() override;

private:
  void onDownloadResult(DownloadItemResult_t* result);

  PcPlatformServicesStatePtr m_state;
  SteamCallback<SteamUserGeneratedContentService, DownloadItemResult_t> m_callbackDownloadResult;

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
