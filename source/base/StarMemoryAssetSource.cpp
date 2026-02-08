#include "StarMemoryAssetSource.hpp"
#include "StarConfig.hpp"
#include "StarImage.hpp"

import std;

namespace Star {

MemoryAssetSource::MemoryAssetSource(String const& name, JsonObject metadata) : m_name(std::move(name)), m_metadata(std::move(metadata)) {}

auto MemoryAssetSource::name() const -> String {
  return m_name;
}

auto MemoryAssetSource::metadata() const -> JsonObject {
  return m_metadata;
}

auto MemoryAssetSource::assetPaths() const -> StringList {
  return m_files.keys();
}

auto MemoryAssetSource::open(String const& path) -> Ptr<IODevice> {
  struct AssetReader : public IODevice {
    AssetReader(char* assetData, std::size_t assetSize, String name) {
      this->assetData = assetData;
      this->assetSize = assetSize;
      this->name = std::move(name);
      setMode(IOMode::Read);
    }

    auto read(char* data, std::size_t len) -> std::size_t override {
      len = std::min<std::int64_t>(len, std::int64_t(assetSize) - assetPos);
      std::memcpy(data, assetData + assetPos, len);
      assetPos += len;
      return len;
    }

    auto write(char const*, std::size_t) -> std::size_t override {
      throw IOException("Assets IODevices are read-only");
    }

    auto size() -> std::int64_t override { return assetSize; }
    auto pos() -> std::int64_t override { return assetPos; }

    auto deviceName() const -> String override { return name; }

    auto atEnd() -> bool override {
      return assetPos >= (std::int64_t)assetSize;
    }

    void seek(std::int64_t p, IOSeek mode) override {
      if (mode == IOSeek::Absolute)
        assetPos = p;
      else if (mode == IOSeek::Relative)
        assetPos = clamp<std::int64_t>(assetPos + p, 0, assetSize);
      else
        assetPos = clamp<std::int64_t>(assetPos - p, 0, assetSize);
    }

    auto clone() -> Ptr<IODevice> override {
      auto cloned = std::make_shared<AssetReader>(assetData, assetSize, name);
      cloned->assetPos = assetPos;
      return cloned;
    }

    char* assetData;
    std::size_t assetSize;
    std::int64_t assetPos = 0;
    String name;
  };

  auto p = m_files.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in memory", path);
  else if (auto byteArray = p->ptr<ByteArray>())
    return std::make_shared<AssetReader>(byteArray->ptr(), byteArray->size(), path);
  else {
    auto image = p->get<Ptr<Image>>().get();
    return std::make_shared<AssetReader>((char*)image->data(), image->width() * image->height() * image->bytesPerPixel(), path);
  }
}

auto MemoryAssetSource::empty() const -> bool {
  return m_files.empty();
}

auto MemoryAssetSource::contains(String const& path) const -> bool {
  return m_files.contains(path);
}

auto MemoryAssetSource::erase(String const& path) -> bool {
  return m_files.erase(path) != 0;
}

void MemoryAssetSource::set(String const& path, ByteArray data) {
  m_files[path] = std::move(data);
}

void MemoryAssetSource::set(String const& path, Image const& image) {
  m_files[path] = std::make_shared<Image>(image);
}

void MemoryAssetSource::set(String const& path, Image&& image) {
  m_files[path] = std::make_shared<Image>(std::move(image));
}

auto MemoryAssetSource::read(String const& path) -> ByteArray {
  auto p = m_files.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in memory", path);
  else if (auto bytes = p->ptr<ByteArray>())
    return *bytes;
  else {
    Image const* image = p->get<Ptr<Image>>().get();
    return {(char const*)image->data(), image->width() * image->height() * image->bytesPerPixel()};
  }
}

auto MemoryAssetSource::image(String const& path) -> ConstPtr<Image> {
  auto p = m_files.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in memory", path);
  else if (auto imagePtr = p->ptr<Ptr<Image>>())
    return *imagePtr;
  else
    return nullptr;
}

}// namespace Star
