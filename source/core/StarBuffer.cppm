module;
import std;
import star.byte_array;
import star.io_device;
import star.string;

namespace Star {

class Buffer;
using BufferPtr = std::shared_ptr<Buffer>;
using BufferConstPtr = std::shared_ptr<const Buffer>;
using BufferWeakPtr = std::weak_ptr<Buffer>;
using BufferConstWeakPtr = std::weak_ptr<const Buffer>;
using BufferUPtr = std::unique_ptr<Buffer>;
using BufferConstUPtr = std::unique_ptr<const Buffer>;
class ExternalBuffer;
using ExternalBufferPtr = std::shared_ptr<ExternalBuffer>;
using ExternalBufferConstPtr = std::shared_ptr<const ExternalBuffer>;
using ExternalBufferWeakPtr = std::weak_ptr<ExternalBuffer>;
using ExternalBufferConstWeakPtr = std::weak_ptr<const ExternalBuffer>;
using ExternalBufferUPtr = std::unique_ptr<ExternalBuffer>;
using ExternalBufferConstUPtr = std::unique_ptr<const ExternalBuffer>;

// Wraps a ByteArray to an IODevice
class Buffer : public IODevice {
public:
  // Constructs buffer open ReadWrite
  Buffer();
  Buffer(std::size_t initialSize);
  Buffer(ByteArray b);
  Buffer(Buffer const& buffer);
  Buffer(Buffer&& buffer);

  StreamOffset pos() override;
  void seek(StreamOffset pos, IOSeek mode = IOSeek::Absolute) override;
  void resize(StreamOffset size) override;
  bool atEnd() override;

  std::size_t read(char* data, std::size_t len) override;
  std::size_t write(char const* data, std::size_t len) override;

  std::size_t readAbsolute(StreamOffset readPosition, char* data, std::size_t len) override;
  std::size_t writeAbsolute(StreamOffset writePosition, char const* data, std::size_t len) override;

  void open(IOMode mode) override;

  String deviceName() const override;

  StreamOffset size() override;
  
  IODevicePtr clone() override;

  ByteArray& data();
  ByteArray const& data() const;

  // If this class holds the underlying data, then this method is cheap, and
  // will move the data out of this class into the returned array, otherwise,
  // this will incur a copy.  Afterwards, this Buffer will be left empty.
  ByteArray takeData();

  // Returns a pointer to the beginning of the Buffer.
  char* ptr();
  char const* ptr() const;

  // Same thing as size(), just size_t type (since this is in-memory)
  std::size_t dataSize() const;
  void reserve(std::size_t size);

  // Clears buffer, moves position to 0.
  void clear();
  bool empty() const;

  // Reset buffer with new contents, moves position to 0.
  void reset(std::size_t newSize);
  void reset(ByteArray b);

  Buffer& operator=(Buffer const& buffer);
  Buffer& operator=(Buffer&& buffer);

private:
  std::size_t doRead(std::size_t pos, char* data, std::size_t len);
  std::size_t doWrite(std::size_t pos, char const* data, std::size_t len);

  std::size_t m_pos;
  ByteArray m_bytes;
};

// Wraps an externally held sequence of bytes to a read-only IODevice
class ExternalBuffer : public IODevice {
public:
  // Constructs an empty ReadOnly ExternalBuffer.
  ExternalBuffer();
  // Constructs a ReadOnly ExternalBuffer pointing to the given external data, which
  // must be valid for the lifetime of the ExternalBuffer.
  ExternalBuffer(char const* externalData, std::size_t len);

  ExternalBuffer(ExternalBuffer const& buffer) = default;
  ExternalBuffer& operator=(ExternalBuffer const& buffer) = default;

  StreamOffset pos() override;
  void seek(StreamOffset pos, IOSeek mode = IOSeek::Absolute) override;
  bool atEnd() override;

  std::size_t read(char* data, std::size_t len) override;
  std::size_t write(char const* data, std::size_t len) override;

  std::size_t readAbsolute(StreamOffset readPosition, char* data, std::size_t len) override;
  std::size_t writeAbsolute(StreamOffset writePosition, char const* data, std::size_t len) override;

  String deviceName() const override;

  StreamOffset size() override;
  
  IODevicePtr clone() override;

  // Returns a pointer to the beginning of the Buffer.
  char const* ptr() const;

  // Same thing as size(), just size_t type (since this is in-memory)
  std::size_t dataSize() const;

  // Clears buffer, moves position to 0.
  bool empty() const;

  operator bool() const;

  // Reset buffer with new contents, moves position to 0.
  void reset(char const* externalData, std::size_t len);

private:
  std::size_t doRead(std::size_t pos, char* data, std::size_t len);

  std::size_t m_pos;
  char const* m_bytes;
  std::size_t m_size;
};

}

export module star.buffer;

export namespace Star {
  using ::Star::Buffer;
  using ::Star::BufferPtr;
  using ::Star::BufferConstPtr;
  using ::Star::BufferUPtr;
  using ::Star::BufferConstUPtr;
  using ::Star::BufferWeakPtr;
  using ::Star::BufferConstWeakPtr;
  using ::Star::ExternalBuffer;
  using ::Star::ExternalBufferPtr;
  using ::Star::ExternalBufferConstPtr;
  using ::Star::ExternalBufferUPtr;
  using ::Star::ExternalBufferConstUPtr;
  using ::Star::ExternalBufferWeakPtr;
  using ::Star::ExternalBufferConstWeakPtr;
}
