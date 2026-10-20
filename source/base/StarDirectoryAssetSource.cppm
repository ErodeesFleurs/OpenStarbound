module;
import std;
import star.byte_array;

import star.list;
import star.maybe;
import star.json;
import star.io_device;
import star.asset_source;
import star.string;

namespace Star {

class Logger;

class DirectoryAssetSource : public AssetSource {
public:
  // Any file that forms an asset path that matches any of the patterns in
  // 'ignorePatterns' is ignored.
  DirectoryAssetSource(Logger& logger, String const& baseDirectory, StringList const& ignorePatterns = {});

  JsonObject metadata() const override;
  StringList assetPaths() const override;

  std::shared_ptr<IODevice> open(String const& path) override;
  ByteArray read(String const& path) override;

  // Converts an asset path to the path on the filesystem
  String toFilesystem(String const& path) const;

  // Update metadata file or add a new one.
  void setMetadata(JsonObject metadata);

private:
  // 'ancestorDirectories' holds the filesystem path of every directory on the
  // current recursion chain, so that a symlink leading back into one of them is
  // skipped instead of being followed until the operating system errors out.
  void scanAll(String const& assetDirectory, StringList& output, List<String> const& ancestorDirectories) const;

  Logger& m_logger;
  String m_baseDirectory;
  List<String> m_ignorePatterns;
  Maybe<String> m_metadataFile;
  JsonObject m_metadata;
  StringList m_assetPaths;
};

}

export module star.directory_asset_source;

export namespace Star {
  using ::Star::DirectoryAssetSource;
}
