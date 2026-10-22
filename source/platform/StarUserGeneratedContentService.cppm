module;

import std;
import star.format;
import star.string;

namespace Star {

class UserGeneratedContentService {
public:
  enum class UGCState {
    NoDownload = 0,
    InProgress = 1,
    Finished = 2
  };
	
  virtual ~UserGeneratedContentService() = default;

  // Returns a list of the content the user is currently subscribed to.
  virtual StringList subscribedContentIds() const = 0;

  // If the content has been downloaded successfully, returns the path to the
  // downloaded content directory on the filesystem, otherwise nothing.
  [[nodiscard]] virtual std::expected<std::optional<String>, StarException> contentDownloadDirectory(String const& contentId) const = 0;

  // Starts or polls background downloads; failures retain their real diagnostic.
  [[nodiscard]] virtual std::expected<UserGeneratedContentService::UGCState, StarException> triggerContentDownload() = 0;
};

}

export module star.user_generated_content_service;

export namespace Star {
  using ::Star::UserGeneratedContentService;
}
