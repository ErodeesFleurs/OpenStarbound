#pragma once

#include "StarPlatformServices_pc.hpp"

namespace Star {

class SteamUserGeneratedContentService;
using SteamUserGeneratedContentServicePtr = shared_ptr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstPtr = shared_ptr<SteamUserGeneratedContentService const>;
using SteamUserGeneratedContentServiceWeakPtr = weak_ptr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstWeakPtr = weak_ptr<SteamUserGeneratedContentService const>;
using SteamUserGeneratedContentServiceUPtr = unique_ptr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstUPtr = unique_ptr<SteamUserGeneratedContentService const>;

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
