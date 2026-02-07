#pragma once

#include "StarConfig.hpp"
#include "StarIODevice.hpp"
#include "StarJson.hpp"
#include "StarException.hpp"

namespace Star {

using AssetSourceException = ExceptionDerived<"AssetSourceException">;

// An asset source could be a directory on a filesystem, where assets are
// pulled directly from files, or a single pak-like file containing all assets,
// where assets are pulled from the correct region of the pak-like file.
class AssetSource {
public:
  virtual ~AssetSource() = default;

  // An asset source can have arbitrary metadata attached.
  [[nodiscard]] virtual auto metadata() const -> JsonObject = 0;

  // Should return all the available assets in this source
  [[nodiscard]] virtual auto assetPaths() const -> StringList = 0;

  // Open the given path in this source and return an IODevicePtr to it.
  virtual auto open(String const& path) -> Ptr<IODevice> = 0;

  // Read the entirety of the given path into a buffer.
  virtual auto read(String const& path) -> ByteArray = 0;
};

}
