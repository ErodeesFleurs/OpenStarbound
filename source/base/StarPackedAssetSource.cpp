#include "StarPackedAssetSource.hpp"
#include "StarConfig.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarDirectoryAssetSource.hpp"
#include "StarFile.hpp"
#include "StarOrderedSet.hpp"

import std;

namespace Star {

void PackedAssetSource::build(DirectoryAssetSource& directorySource, String const& targetPackedFile,
                              StringList const& extensionSorting, BuildProgressCallback progressCallback) {
  Ptr<File> file = File::open(targetPackedFile, IOMode::ReadWrite | IOMode::Truncate);

  DataStreamIODevice ds(file);

  ds.writeData("SBAsset6", 8);
  // Skip 8 bytes, this will be a pointer to the index once we are done.
  ds.seek(8, IOSeek::Relative);

  // Insert every found entry into the packed file, and also simultaneously
  // compute the full index.
  StringMap<std::pair<std::uint64_t, std::uint64_t>> index;

  OrderedHashSet<String> extensionOrdering;
  for (auto const& str : extensionSorting)
    extensionOrdering.add(str.toLower());

  StringList assetPaths = directorySource.assetPaths();

  // Returns a value for the asset that can be used to predictably sort assets
  // by name and then by extension, where every extension listed in
  // "extensionSorting" will come first, and then any extension not listed will
  // come after.
  auto getOrderingValue = [&extensionOrdering](String const& asset) -> std::pair<std::size_t, String> {
    String extension;
    auto lastDot = asset.findLast(".");
    if (lastDot != std::numeric_limits<std::size_t>::max())
      extension = asset.substr(lastDot + 1);

    if (auto i = extensionOrdering.indexOf(extension.toLower())) {
      return {*i, asset.toLower()};
    } else {
      return {extensionOrdering.size(), asset.toLower()};
    }
  };

  assetPaths.sort([&getOrderingValue](String const& a, String const& b) -> bool {
    return getOrderingValue(a) < getOrderingValue(b);
  });

  for (std::size_t i = 0; i < assetPaths.size(); ++i) {
    String const& assetPath = assetPaths[i];
    ByteArray contents = directorySource.read(assetPath);

    if (progressCallback)
      progressCallback(i, assetPaths.size(), directorySource.toFilesystem(assetPath), assetPath);
    index.add(assetPath, {ds.pos(), contents.size()});
    ds.writeBytes(contents);
  }

  std::uint64_t indexStart = ds.pos();
  ds.writeData("INDEX", 5);
  ds.write(directorySource.metadata());
  ds.write(index);

  ds.seek(8);
  ds.write(indexStart);
}

PackedAssetSource::PackedAssetSource(String const& filename) {
  m_packedFile = File::open(filename, IOMode::Read);

  DataStreamIODevice ds(m_packedFile);
  if (ds.readBytes(8) != ByteArray("SBAsset6", 8))
    throw AssetSourceException("Packed assets file format unrecognized!");

  auto indexStart = ds.read<std::uint64_t>();

  ds.seek(indexStart);
  ByteArray header = ds.readBytes(5);
  if (header != ByteArray("INDEX", 5))
    throw AssetSourceException("No index header found!");
  ds.read(m_metadata);
  ds.read(m_index);
}

auto PackedAssetSource::metadata() const -> JsonObject {
  return m_metadata;
}

auto PackedAssetSource::assetPaths() const -> StringList {
  return m_index.keys();
}

auto PackedAssetSource::open(String const& path) -> Ptr<IODevice> {
  struct AssetReader : public IODevice {
    AssetReader(Ptr<File> file, String path, std::int64_t offset, std::int64_t size)
        : file(std::move(file)), path(std::move(path)), fileOffset(offset), assetSize(size) {
      setMode(IOMode::Read);
    }

    auto read(char* data, std::size_t len) -> std::size_t override {
      len = std::min<std::int64_t>(len, assetSize - assetPos);
      file->readFullAbsolute(fileOffset + assetPos, data, len);
      assetPos += len;
      return len;
    }

    auto write(char const*, std::size_t) -> std::size_t override {
      throw IOException("Assets IODevices are read-only");
    }

    auto size() -> std::int64_t override {
      return assetSize;
    }

    auto pos() -> std::int64_t override {
      return assetPos;
    }

    auto deviceName() const -> String override {
      return strf("{}:{}", file->deviceName(), path);
    }

    auto atEnd() -> bool override {
      return assetPos >= assetSize;
    }

    void seek(std::int64_t p, IOSeek mode) override {
      if (mode == IOSeek::Absolute)
        assetPos = p;
      else if (mode == IOSeek::Relative)
        assetPos = std::clamp<std::int64_t>(assetPos + p, 0, assetSize);
      else
        assetPos = std::clamp<std::int64_t>(assetSize - p, 0, assetSize);
    }

    auto clone() -> Ptr<IODevice> override {
      auto cloned = std::make_shared<AssetReader>(file, path, fileOffset, assetSize);
      cloned->assetPos = assetPos;
      return cloned;
    }

    Ptr<File> file;
    String path;
    std::int64_t fileOffset;
    std::int64_t assetSize;
    std::int64_t assetPos{};
  };

  auto p = m_index.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in the packed assets file", path);

  return make_shared<AssetReader>(m_packedFile, path, p->first, p->second);
}

auto PackedAssetSource::read(String const& path) -> ByteArray {
  auto p = m_index.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in the packed assets file", path);

  ByteArray data(p->second, 0);
  m_packedFile->readFullAbsolute(p->first, data.ptr(), p->second);
  return data;
}

}// namespace Star
