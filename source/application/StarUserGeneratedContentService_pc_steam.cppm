module;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#endif
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import std;
import star.format;
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
import star.steam_dispatch;
#endif

import star.platform_services_pc;

namespace Star {

class SteamUserGeneratedContentService final : public UserGeneratedContentService {
public:
  SteamUserGeneratedContentService(std::shared_ptr<PcPlatformServicesState> state);

  StringList subscribedContentIds() const override;
  [[nodiscard]] std::expected<std::optional<String>, StarException> contentDownloadDirectory(String const& contentId) const override;
  [[nodiscard]] std::expected<UserGeneratedContentService::UGCState, StarException> triggerContentDownload() override;

private:
  std::expected<void, StarException> onDownloadResult(DownloadItemResult_t& result);

  std::shared_ptr<PcPlatformServicesState> m_state;
  SteamCallback<SteamUserGeneratedContentService, DownloadItemResult_t> m_callbackDownloadResult;

  HashMap<PublishedFileId_t, bool> m_currentDownloadState;
  std::optional<StarException> m_downloadError;

  bool m_checkedUGC;
};

}

export module star.user_generated_content_service_pc_steam;

export namespace Star {
  using ::Star::SteamUserGeneratedContentService;
}
