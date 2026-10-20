module;
import std;
import star.string;
import star.variant;
import star.format;
import star.json;
import star.byte_array;


import star.io_device;
import star.asset_source;
import star.data_stream_devices;
import star.data_stream_extra;
import star.image;
import star.math_common;
import star.type_identity;

export module star.memory_asset_source;

export namespace Star {

class MemoryAssetSource : public AssetSource {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<MemoryAssetSource, AssetSource>(this, key);
  }

  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<MemoryAssetSource>();
  }

  MemoryAssetSource(String const& name, JsonObject metadata = JsonObject());

  String name() const;
  JsonObject metadata() const override;
  StringList assetPaths() const override;

  // do not use the returned IODevice after the file is gone or bad things will happen
  std::shared_ptr<IODevice> open(String const& path) override;

  bool empty() const;
  bool contains(String const& path) const;
  bool erase(String const& path);
  void set(String const& path, ByteArray data);
  void set(String const& path, Image const& image);
  void set(String const& path, Image&& image);
  ByteArray read(String const& path) override;
  std::shared_ptr<Image const> image(String const& path);
private:
  typedef Variant<ByteArray, std::shared_ptr<Image>> FileEntry;

  String m_name;
  JsonObject m_metadata;
  StringMap<FileEntry> m_files;
};

}

namespace Star {

MemoryAssetSource::MemoryAssetSource(String const& name, JsonObject metadata) : m_name(name), m_metadata(metadata) {}

String MemoryAssetSource::name() const {
  return m_name;
}

JsonObject MemoryAssetSource::metadata() const {
  return m_metadata;
}

StringList MemoryAssetSource::assetPaths() const {
  return m_files.keys();
}

std::shared_ptr<IODevice> MemoryAssetSource::open(String const& path) {
  struct AssetReader : public IODevice {
    AssetReader(char* assetData, std::size_t assetSize, String name) {
      this->assetData = assetData;
      this->assetSize = assetSize;
      this->name = std::move(name);
      setMode(IOMode::Read);
    }

    std::size_t read(char* data, std::size_t len) override {
      len = std::min<StreamOffset>(len, StreamOffset(assetSize) - assetPos);
      std::memcpy(data, assetData + assetPos, len);
      assetPos += len;
      return len;
    }

    std::size_t write(char const*, std::size_t) override {
      throw IOException("Assets IODevices are read-only");
    }

    StreamOffset size() override { return assetSize; }
    StreamOffset pos() override { return assetPos; }

    String deviceName() const override { return name; }

    bool atEnd() override {
      return assetPos >= (StreamOffset)assetSize;
    }

    void seek(StreamOffset p, IOSeek mode) override {
      if (mode == IOSeek::Absolute)
        assetPos = p;
      else if (mode == IOSeek::Relative)
        assetPos = clamp<StreamOffset>(assetPos + p, 0, assetSize);
      else
        assetPos = clamp<StreamOffset>(assetPos - p, 0, assetSize);
    }

    std::shared_ptr<IODevice> clone() override {
      auto cloned = std::make_shared<AssetReader>(assetData, assetSize, name);
      cloned->assetPos = assetPos;
      return cloned;
    }

    char* assetData;
    std::size_t assetSize;
    StreamOffset assetPos = 0;
    String name;
  };

  auto p = m_files.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in memory", path);
  else if (auto byteArray = p->ptr<ByteArray>())
    return std::make_shared<AssetReader>(byteArray->ptr(), byteArray->size(), path);
  else {
    auto image = p->get<std::shared_ptr<Image>>().get();
    return std::make_shared<AssetReader>((char*)image->data(), image->width() * image->height() * image->bytesPerPixel(), path);
  }
}

bool MemoryAssetSource::empty() const {
  return m_files.empty();
}

bool MemoryAssetSource::contains(String const& path) const {
  return m_files.contains(path);
}

bool MemoryAssetSource::erase(String const& path) {
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

ByteArray MemoryAssetSource::read(String const& path) {
  auto p = m_files.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in memory", path);
  else if (auto bytes = p->ptr<ByteArray>())
    return *bytes;
  else {
    Image const* image = p->get<std::shared_ptr<Image>>().get();
    return ByteArray((char const*)image->data(), image->width() * image->height() * image->bytesPerPixel());
  }
}

std::shared_ptr<Image const> MemoryAssetSource::image(String const& path) {
  auto p = m_files.ptr(path);
  if (!p)
    throw AssetSourceException::format("Requested file '{}' does not exist in memory", path);
  else if (auto imagePtr = p->ptr<std::shared_ptr<Image>>())
    return *imagePtr;
  else
    return nullptr;
}

}
