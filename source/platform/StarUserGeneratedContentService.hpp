#pragma once

#include "StarString.hpp"

import std;

namespace Star {

class UserGeneratedContentService {
public:
  enum class UGCState {
    NoDownload = 0,
    InProgress = 1,
    Finished = 2
  };

  ~UserGeneratedContentService() = default;

  // Returns a list of the content the user is currently subscribed to.
  [[nodiscard]] virtual auto subscribedContentIds() const -> StringList = 0;

  // If the content has been downloaded successfully, returns the path to the
  // downloaded content directory on the filesystem, otherwise nothing.
  [[nodiscard]] virtual auto contentDownloadDirectory(String const& contentId) const -> std::optional<String> = 0;

  // Start downloading subscribed content in the background, returns true when
  // all content is synchronized.
  virtual auto triggerContentDownload() -> UserGeneratedContentService::UGCState = 0;
};

}// namespace Star
