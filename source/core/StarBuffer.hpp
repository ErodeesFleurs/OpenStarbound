module;

#include "StarIODevice.hpp"

import std;

namespace Star {

// Wraps a ByteArray to an IODevice
class Buffer : public IODevice {
public:
  // Constructs buffer open ReadWrite
  Buffer();
  Buffer(std::size_t initialSize);
  Buffer(ByteArray b);
  Buffer(Buffer const& buffer);
  Buffer(Buffer&& buffer);

  auto pos() -> std::int64_t override;
  void seek(std::int64_t pos, IOSeek mode = IOSeek::Absolute) override;
  void resize(std::int64_t size) override;
  auto atEnd() -> bool override;

  auto read(char* data, std::size_t len) -> std::size_t override;
  auto write(char const* data, std::size_t len) -> std::size_t override;

  auto readAbsolute(std::int64_t readPosition, char* data, std::size_t len) -> std::size_t override;
  auto writeAbsolute(std::int64_t writePosition, char const* data, std::size_t len) -> std::size_t override;

  void open(IOMode mode) override;

  auto deviceName() const -> String override;

  auto size() -> std::int64_t override;

  auto clone() -> Ptr<IODevice> override;

  auto data() -> ByteArray&;
  auto data() const -> ByteArray const&;

  // If this class holds the underlying data, then this method is cheap, and
  // will move the data out of this class into the returned array, otherwise,
  // this will incur a copy.  Afterwards, this Buffer will be left empty.
  auto takeData() -> ByteArray;

  // Returns a pointer to the beginning of the Buffer.
  auto ptr() -> char*;
  auto ptr() const -> char const*;

  // Same thing as size(), just size_t type (since this is in-memory)
  auto dataSize() const -> std::size_t;
  void reserve(std::size_t size);

  // Clears buffer, moves position to 0.
  void clear();
  auto empty() const -> bool;

  // Reset buffer with new contents, moves position to 0.
  void reset(std::size_t newSize);
  void reset(ByteArray b);

  auto operator=(Buffer const& buffer) -> Buffer&;
  auto operator=(Buffer&& buffer) -> Buffer&;

private:
  auto doRead(std::size_t pos, char* data, std::size_t len) -> std::size_t;
  auto doWrite(std::size_t pos, char const* data, std::size_t len) -> std::size_t;

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
  auto operator=(ExternalBuffer const& buffer) -> ExternalBuffer& = default;

  auto pos() -> std::int64_t override;
  void seek(std::int64_t pos, IOSeek mode = IOSeek::Absolute) override;
  auto atEnd() -> bool override;

  auto read(char* data, std::size_t len) -> std::size_t override;
  auto write(char const* data, std::size_t len) -> std::size_t override;

  auto readAbsolute(std::int64_t readPosition, char* data, std::size_t len) -> std::size_t override;
  auto writeAbsolute(std::int64_t writePosition, char const* data, std::size_t len) -> std::size_t override;

  auto deviceName() const -> String override;

  auto size() -> std::int64_t override;

  auto clone() -> Ptr<IODevice> override;

  // Returns a pointer to the beginning of the Buffer.
  auto ptr() const -> char const*;

  // Same thing as size(), just std::size_t type (since this is in-memory)
  auto dataSize() const -> std::size_t;

  // Clears buffer, moves position to 0.
  auto empty() const -> bool;

  operator bool() const;

  // Reset buffer with new contents, moves position to 0.
  void reset(char const* externalData, std::size_t len);

private:
  auto doRead(std::size_t pos, char* data, std::size_t len) -> std::size_t;

  std::size_t m_pos;
  char const* m_bytes;
  std::size_t m_size;
};

}
