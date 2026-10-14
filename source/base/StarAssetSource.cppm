module;
import std;
import star.byte_array;

import star.string;
import star.format;
import star.json;
import star.io_device;

namespace Star {

class AssetSource;
using AssetSourcePtr = std::shared_ptr<AssetSource>;
using AssetSourceConstPtr = std::shared_ptr<AssetSource const>;
using AssetSourceWeakPtr = std::weak_ptr<AssetSource>;
using AssetSourceConstWeakPtr = std::weak_ptr<AssetSource const>;
using AssetSourceUPtr = std::unique_ptr<AssetSource>;
using AssetSourceConstUPtr = std::unique_ptr<AssetSource const>;

struct AssetSourceExceptionTag {
  static constexpr char const* name() { return "AssetSourceException"; }
};
using AssetSourceException = StarError<AssetSourceExceptionTag, StarException>;

// An asset source could be a directory on a filesystem, where assets are
// pulled directly from files, or a single pak-like file containing all assets,
// where assets are pulled from the correct region of the pak-like file.
class AssetSource {
public:
  virtual ~AssetSource() = default;

  // An asset source can have arbitrary metadata attached.
  virtual JsonObject metadata() const = 0;

  // Should return all the available assets in this source
  virtual StringList assetPaths() const = 0;

  // Open the given path in this source and return an IODevicePtr to it.
  virtual IODevicePtr open(String const& path) = 0;

  // Read the entirety of the given path into a buffer.
  virtual ByteArray read(String const& path) = 0;
};

}

export module star.asset_source;

export namespace Star {
  using ::Star::AssetSourceExceptionTag;
  using ::Star::AssetSourceException;
  using ::Star::AssetSource;
  using ::Star::AssetSourcePtr;
  using ::Star::AssetSourceConstPtr;
  using ::Star::AssetSourceWeakPtr;
  using ::Star::AssetSourceConstWeakPtr;
  using ::Star::AssetSourceUPtr;
  using ::Star::AssetSourceConstUPtr;
}
