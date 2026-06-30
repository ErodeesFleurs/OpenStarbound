#pragma once

#include "StarAssetSource.hpp"
#include "StarIODevice.hpp"

namespace Star {

class MemoryAssetSource;
using MemoryAssetSourcePtr = SharedPtr<MemoryAssetSource>;
class Image;
using ImagePtr = SharedPtr<Image>;
using ImageConstPtr = SharedPtr<Image const>;

class MemoryAssetSource : public AssetSource {
public:
  MemoryAssetSource(String const& name, JsonObject metadata = JsonObject());

  [[nodiscard]] String name() const;
  [[nodiscard]] JsonObject metadata() const override;
  [[nodiscard]] StringList assetPaths() const override;

  // do not use the returned IODevice after the file is gone or bad things will happen
  [[nodiscard]] IODevicePtr open(String const& path) override;

  [[nodiscard]] bool empty() const;
  [[nodiscard]] bool contains(String const& path) const;
  [[nodiscard]] bool erase(String const& path);
  void set(String const& path, ByteArray data);
  void set(String const& path, Image const& image);
  void set(String const& path, Image&& image);
  [[nodiscard]] ByteArray read(String const& path) override;
  [[nodiscard]] ImageConstPtr image(String const& path);

private:
  using FileEntry = Variant<ByteArray, ImagePtr>;

  String m_name;
  JsonObject m_metadata;
  StringMap<FileEntry> m_files;
};

}// namespace Star
