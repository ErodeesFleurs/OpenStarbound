#pragma once

#include "StarPlatformServices_pc.hpp"

namespace Star {

class SteamUserGeneratedContentService;
using SteamUserGeneratedContentServicePtr = SharedPtr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstPtr = SharedPtr<SteamUserGeneratedContentService const>;
using SteamUserGeneratedContentServiceWeakPtr = WeakPtr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstWeakPtr = WeakPtr<SteamUserGeneratedContentService const>;
using SteamUserGeneratedContentServiceUPtr = UniquePtr<SteamUserGeneratedContentService>;
using SteamUserGeneratedContentServiceConstUPtr = UniquePtr<SteamUserGeneratedContentService const>;

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
