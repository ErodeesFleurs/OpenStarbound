#pragma once

#include "StarIODevice.hpp"
#include "StarString.hpp"

namespace Star {

class Buffer;
using BufferPtr = SharedPtr<Buffer>;
class ExternalBuffer;

// Wraps a ByteArray to an IODevice
class Buffer : public IODevice {
public:
  // Constructs buffer open ReadWrite
  Buffer();
  Buffer(size_t initialSize);
  Buffer(ByteArray b);
  Buffer(Buffer const& buffer);
  Buffer(Buffer&& buffer);

  [[nodiscard]] StreamOffset pos() override;
  void seek(StreamOffset pos, IOSeek mode = IOSeek::Absolute) override;
  void resize(StreamOffset size) override;
  [[nodiscard]] bool atEnd() override;

  [[nodiscard]] size_t read(char* data, size_t len) override;
  [[nodiscard]] size_t write(char const* data, size_t len) override;

  [[nodiscard]] size_t readAbsolute(StreamOffset readPosition, char* data, size_t len) override;
  [[nodiscard]] size_t writeAbsolute(StreamOffset writePosition, char const* data, size_t len) override;

  void open(IOMode mode) override;

  [[nodiscard]] String deviceName() const override;

  [[nodiscard]] StreamOffset size() override;
  
  [[nodiscard]] IODevicePtr clone() override;

  [[nodiscard]] ByteArray& data();
  [[nodiscard]] ByteArray const& data() const;

  // If this class holds the underlying data, then this method is cheap, and
  // will move the data out of this class into the returned array, otherwise,
  // this will incur a copy.  Afterwards, this Buffer will be left empty.
  [[nodiscard]] ByteArray takeData();

  // Returns a pointer to the beginning of the Buffer.
  [[nodiscard]] char* ptr();
  [[nodiscard]] char const* ptr() const;

  // Same thing as size(), just size_t type (since this is in-memory)
  [[nodiscard]] size_t dataSize() const;
  void reserve(size_t size);

  // Clears buffer, moves position to 0.
  void clear();
  [[nodiscard]] bool empty() const;

  // Reset buffer with new contents, moves position to 0.
  void reset(size_t newSize);
  void reset(ByteArray b);

  Buffer& operator=(Buffer const& buffer);
  Buffer& operator=(Buffer&& buffer);

private:
  [[nodiscard]] size_t doRead(size_t pos, char* data, size_t len);
  [[nodiscard]] size_t doWrite(size_t pos, char const* data, size_t len);

  size_t m_pos = 0;
  ByteArray m_bytes;
};

// Wraps an externally held sequence of bytes to a read-only IODevice
class ExternalBuffer : public IODevice {
public:
  // Constructs an empty ReadOnly ExternalBuffer.
  ExternalBuffer();
  // Constructs a ReadOnly ExternalBuffer pointing to the given external data, which
  // must be valid for the lifetime of the ExternalBuffer.
  ExternalBuffer(char const* externalData, size_t len);

  ExternalBuffer(ExternalBuffer const& buffer) noexcept = default;
  ExternalBuffer& operator=(ExternalBuffer const& buffer) noexcept = default;

  [[nodiscard]] StreamOffset pos() override;
  void seek(StreamOffset pos, IOSeek mode = IOSeek::Absolute) override;
  [[nodiscard]] bool atEnd() override;

  [[nodiscard]] size_t read(char* data, size_t len) override;
  [[nodiscard]] size_t write(char const* data, size_t len) override;

  [[nodiscard]] size_t readAbsolute(StreamOffset readPosition, char* data, size_t len) override;
  [[nodiscard]] size_t writeAbsolute(StreamOffset writePosition, char const* data, size_t len) override;

  [[nodiscard]] String deviceName() const override;

  [[nodiscard]] StreamOffset size() override;
  
  [[nodiscard]] IODevicePtr clone() override;

  // Returns a pointer to the beginning of the Buffer.
  [[nodiscard]] char const* ptr() const;

  // Same thing as size(), just size_t type (since this is in-memory)
  [[nodiscard]] size_t dataSize() const;

  // Clears buffer, moves position to 0.
  [[nodiscard]] bool empty() const;

  [[nodiscard]] explicit operator bool() const;

  // Reset buffer with new contents, moves position to 0.
  void reset(char const* externalData, size_t len);

private:
  [[nodiscard]] size_t doRead(size_t pos, char* data, size_t len);

  size_t m_pos = 0;
  char const* m_bytes = nullptr;
  size_t m_size = 0;
};

}
