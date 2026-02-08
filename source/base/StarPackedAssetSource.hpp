#pragma once

#include "StarConfig.hpp"
#include "StarDirectoryAssetSource.hpp"
#include "StarFile.hpp"
#include "StarOrderedMap.hpp"

import std;

namespace Star {

class PackedAssetSource : public AssetSource {
public:
  using BuildProgressCallback = std::function<void(std::size_t, std::size_t, String, String)>;

  // Build a packed asset file from the given DirectoryAssetSource.
  //
  // 'extensionSorting' sorts the packed file with file extensions that case
  // insensitive match the given extensions in the order they are given.  If a
  // file has an extension that doesn't match any in this list, it goes after
  // all other files.  All files are sorted secondarily by case insensitive
  // alphabetical order.
  //
  // If given, 'progressCallback' will be called with the total number of
  // files, the current file number, the file name, and the asset path.
  static void build(DirectoryAssetSource& directorySource, String const& targetPackedFile,
                    StringList const& extensionSorting = {}, BuildProgressCallback progressCallback = {});

  PackedAssetSource(String const& packedFileName);

  [[nodiscard]] auto metadata() const -> JsonObject override;
  [[nodiscard]] auto assetPaths() const -> StringList override;

  auto open(String const& path) -> Ptr<IODevice> override;
  auto read(String const& path) -> ByteArray override;

private:
  Ptr<File> m_packedFile;
  JsonObject m_metadata;
  OrderedHashMap<String, std::pair<std::uint64_t, std::uint64_t>> m_index;
};

}// namespace Star
