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
import star.user_generated_content_service_pc_steam;
#include "StarLogging.hpp"
import star.lexical_cast;

namespace Star {

SteamUserGeneratedContentService::SteamUserGeneratedContentService(PcPlatformServicesStatePtr state)
  : m_state(std::move(state)),
    m_callbackDownloadResult(m_state->steamDispatch, this, &SteamUserGeneratedContentService::onDownloadResult),
    m_checkedUGC(false) {};

StringList SteamUserGeneratedContentService::subscribedContentIds() const {
  List<PublishedFileId_t> contentIds(SteamAPI_ISteamUGC_GetNumSubscribedItems(SteamAPI_SteamUGC()), {});
  SteamAPI_ISteamUGC_GetSubscribedItems(SteamAPI_SteamUGC(), contentIds.ptr(), contentIds.size());
  return contentIds.transformed([](PublishedFileId_t id) {
      return String(toString(id));
    });
}

Maybe<String> SteamUserGeneratedContentService::contentDownloadDirectory(String const& contentId) const {
  PublishedFileId_t id = lexicalCast<PublishedFileId_t>(contentId);
  uint32 itemState = SteamAPI_ISteamUGC_GetItemState(SteamAPI_SteamUGC(), id);
  if (itemState & k_EItemStateInstalled) {
    char path[4096];
    if (SteamAPI_ISteamUGC_GetItemInstallInfo(SteamAPI_SteamUGC(), id, nullptr, path, sizeof(path), nullptr))
      return String(path);
  }
  return {};
}

UserGeneratedContentService::UGCState SteamUserGeneratedContentService::triggerContentDownload() {
  List<PublishedFileId_t> contentIds(SteamAPI_ISteamUGC_GetNumSubscribedItems(SteamAPI_SteamUGC()), {});
  SteamAPI_ISteamUGC_GetSubscribedItems(SteamAPI_SteamUGC(), contentIds.ptr(), contentIds.size());

  if (!m_checkedUGC) {
    bool contentNeedsUpdate = false;
    for (uint64 contentId : contentIds) {
      uint32 itemState = SteamAPI_ISteamUGC_GetItemState(SteamAPI_SteamUGC(), contentId);
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
      uint32 itemState = SteamAPI_ISteamUGC_GetItemState(SteamAPI_SteamUGC(), contentId);
      if (!(itemState & k_EItemStateInstalled) || itemState & k_EItemStateNeedsUpdate) {
        // DownloadItem has a return bool if it succeeds in attempting to download.
        if (SteamAPI_ISteamUGC_DownloadItem(SteamAPI_SteamUGC(), contentId, true)) {
          itemState = SteamAPI_ISteamUGC_GetItemState(SteamAPI_SteamUGC(), contentId);
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
  PublishedFileId_t file = result->m_nPublishedFileId;
  m_currentDownloadState[file] = true;
}

}
