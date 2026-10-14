module;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#endif
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import std;
import star.maybe;
import star.thread;
import star.input_event;
import star.application;
import star.string;
import star.map;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "StarSteamDispatch.hpp"
#endif

import star.platform_services_pc;

namespace Star {

class SteamUserGeneratedContentService;
using SteamUserGeneratedContentServicePtr = std::shared_ptr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstPtr = std::shared_ptr<SteamUserGeneratedContentService const>;
using SteamUserGeneratedContentServiceWeakPtr = std::weak_ptr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstWeakPtr = std::weak_ptr<SteamUserGeneratedContentService const>;
using SteamUserGeneratedContentServiceUPtr = std::unique_ptr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstUPtr = std::unique_ptr<SteamUserGeneratedContentService const>;

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
