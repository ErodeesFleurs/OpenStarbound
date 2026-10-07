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
import star.user_generated_content_service_pc_steam;
#include "StarLogging.hpp"
#include "StarLexicalCast.hpp"

namespace Star {

SteamUserGeneratedContentService::SteamUserGeneratedContentService(PcPlatformServicesStatePtr)
  : m_callbackDownloadResult(this, &SteamUserGeneratedContentService::onDownloadResult),
    m_checkedUGC(false) {};

StringList SteamUserGeneratedContentService::subscribedContentIds() const {
  List<PublishedFileId_t> contentIds(SteamUGC()->GetNumSubscribedItems(), {});
  SteamUGC()->GetSubscribedItems(contentIds.ptr(), contentIds.size());
  return contentIds.transformed([](PublishedFileId_t id) {
      return String(toString(id));
    });
}

Maybe<String> SteamUserGeneratedContentService::contentDownloadDirectory(String const& contentId) const {
  PublishedFileId_t id = lexicalCast<PublishedFileId_t>(contentId);
  uint32 itemState = SteamUGC()->GetItemState(id);
  if (itemState & k_EItemStateInstalled) {
    char path[4096];
    if (SteamUGC()->GetItemInstallInfo(id, nullptr, path, sizeof(path), nullptr))
      return String(path);
  }
  return {};
}

UserGeneratedContentService::UGCState SteamUserGeneratedContentService::triggerContentDownload() {
  List<PublishedFileId_t> contentIds(SteamUGC()->GetNumSubscribedItems(), {});
  SteamUGC()->GetSubscribedItems(contentIds.ptr(), contentIds.size());

  if (!m_checkedUGC) {
    bool contentNeedsUpdate = false;
    for (uint64 contentId : contentIds) {
      uint32 itemState = SteamUGC()->GetItemState(contentId);
      if (!(itemState & k_EItemStateInstalled) || itemState & k_EItemStateNeedsUpdate) {
        // Download is needed
        contentNeedsUpdate = true;
      }
    }
    m_checkedUGC = true;
    if (!contentNeedsUpdate) {
      // No download was needed
      return UserGeneratedContentService::UGCState::NoDownload;
    }
  }

  for (uint64 contentId : contentIds) {
    if (!m_currentDownloadState.contains(contentId)) {
      uint32 itemState = SteamUGC()->GetItemState(contentId);
      if (!(itemState & k_EItemStateInstalled) || itemState & k_EItemStateNeedsUpdate) {
        // DownloadItem has a return bool if it succeeds in attempting to download.
        if (SteamUGC()->DownloadItem(contentId, true)) {
          itemState = SteamUGC()->GetItemState(contentId);
          m_currentDownloadState[contentId] = !(itemState & k_EItemStateDownloading);
        } else {
          // DownloadItem failed for some reason. Just set this to true to skip this item and prevent an infinite loop.
          m_currentDownloadState[contentId] = true;
        }
      } else {
        m_currentDownloadState[contentId] = true;
      }
    }
  }

  bool allDownloaded = true;
  for (auto const& p : m_currentDownloadState) {
    if (!p.second)
      allDownloaded = false;
  }

  if (allDownloaded) {
    return UserGeneratedContentService::UGCState::Finished;
  } else {
    return UserGeneratedContentService::UGCState::InProgress;
  }
}

void SteamUserGeneratedContentService::onDownloadResult(DownloadItemResult_t* result) {
  m_currentDownloadState[result->m_nPublishedFileId] = true;
}

}
