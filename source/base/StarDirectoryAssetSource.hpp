#pragma once

#include "StarAssetSource.hpp"
#include "StarConfig.hpp"
#include "StarString.hpp"

import std;

namespace Star {

class DirectoryAssetSource : public AssetSource {
public:
  // Any file that forms an asset path that matches any of the patterns in
  // 'ignorePatterns' is ignored.
  DirectoryAssetSource(String const& baseDirectory, StringList const& ignorePatterns = {});

  [[nodiscard]] auto metadata() const -> JsonObject override;
  [[nodiscard]] auto assetPaths() const -> StringList override;

  auto open(String const& path) -> Ptr<IODevice> override;
  auto read(String const& path) -> ByteArray override;

  // Converts an asset path to the path on the filesystem
  [[nodiscard]] auto toFilesystem(String const& path) const -> String;

  // Update metadata file or add a new one.
  void setMetadata(JsonObject metadata);

private:
  void scanAll(String const& assetDirectory, StringList& output) const;

  String m_baseDirectory;
  List<String> m_ignorePatterns;
  std::optional<String> m_metadataFile;
  JsonObject m_metadata;
  StringList m_assetPaths;
};

}
