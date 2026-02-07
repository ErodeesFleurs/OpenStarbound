#pragma once

#include "StarAssetSource.hpp"
#include "StarConfig.hpp"
#include "StarIODevice.hpp"
#include "StarImage.hpp"

namespace Star {

class MemoryAssetSource : public AssetSource {
public:
  MemoryAssetSource(String const& name, JsonObject metadata = JsonObject());

  [[nodiscard]] auto name() const -> String;
  [[nodiscard]] auto metadata() const -> JsonObject override;
  [[nodiscard]] auto assetPaths() const -> StringList override;

  // do not use the returned IODevice after the file is gone or bad things will happen
  auto open(String const& path) -> Ptr<IODevice> override;

  [[nodiscard]] auto empty() const -> bool;
  [[nodiscard]] auto contains(String const& path) const -> bool;
  auto erase(String const& path) -> bool;
  void set(String const& path, ByteArray data);
  void set(String const& path, Image const& image);
  void set(String const& path, Image&& image);
  auto read(String const& path) -> ByteArray override;
  auto image(String const& path) -> ConstPtr<Image>;

private:
  using FileEntry = Variant<ByteArray, Ptr<Image>>;

  String m_name;
  JsonObject m_metadata;
  StringMap<FileEntry> m_files;
};

}// namespace Star
