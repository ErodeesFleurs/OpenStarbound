#pragma once

#include "StarByteArray.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarString.hpp"

import std;

namespace Star {

using EofException = ExceptionDerived<"EofException">;

enum class IOMode : std::uint8_t {
  Closed = 0x0,
  Read = 0x1,
  Write = 0x2,
  ReadWrite = 0x3,
  Append = 0x4,
  Truncate = 0x8,
};

auto operator|(IOMode a, IOMode b) -> IOMode;
auto operator&(IOMode a, IOMode b) -> bool;

// Should match SEEK_SET, SEEK_CUR, AND SEEK_END
enum IOSeek : std::uint8_t {
  Absolute = 0,
  Relative = 1,
  End = 2
};

// Abstract Interface to a random access I/O device.
class IODevice {
public:
  IODevice(IOMode mode = IOMode::Closed);
  virtual ~IODevice();

  // Do a read or write that may result in less data read or written than
  // requested.
  virtual auto read(char* data, std::size_t len) -> std::size_t = 0;
  virtual auto write(char const* data, std::size_t len) -> std::size_t = 0;

  virtual auto pos() -> std::int64_t = 0;
  virtual void seek(std::int64_t pos, IOSeek mode = IOSeek::Absolute) = 0;

  // Default implementation throws unsupported exception.
  virtual void resize(std::int64_t size);

  // Read / write from an absolute offset in the file without modifying the
  // current file position.  Default implementation stores the file position,
  // then seeks and calls read/write partial, then restores the file position,
  // and is not thread safe.
  virtual auto readAbsolute(std::int64_t readPosition, char* data, std::size_t len) -> std::size_t;
  virtual auto writeAbsolute(std::int64_t writePosition, char const* data, std::size_t len) -> std::size_t;

  // Read and write fully, and throw an exception in every other case.  The
  // default implementations here will call the normal read or write, and if
  // the full amount is not read will throw an exception.
  virtual void readFull(char* data, std::size_t len);
  virtual void writeFull(char const* data, std::size_t len);
  virtual void readFullAbsolute(std::int64_t readPosition, char* data, std::size_t len);
  virtual void writeFullAbsolute(std::int64_t writePosition, char const* data, std::size_t len);

  // Default implementation throws exception if opening in a different mode
  // than the current mode.
  virtual void open(IOMode mode);

  // Default implementation sets mode equal to Closed
  virtual void close();

  // Default implementation is a no-op
  virtual void sync();

  // Returns a clone of this device with the same mode
  virtual auto clone() -> Ptr<IODevice> = 0;

  // Default implementation just prints address of generic IODevice
  virtual auto deviceName() const -> String;

  // Is the file position at the end of the file and there is no more to read?
  // This is not the same as feof, which returns true after an unsuccesful read
  // past the end, it should return true after succesfully reading the final
  // byte.  Default implementation returns pos() >= size();
  virtual auto atEnd() -> bool;

  // Default is to store position, seek end, then restore position.
  virtual auto size() -> std::int64_t;

  auto mode() const -> IOMode;
  auto isOpen() const -> bool;
  auto isReadable() const -> bool;
  auto isWritable() const -> bool;

  auto readBytes(std::size_t size) -> ByteArray;
  void writeBytes(ByteArray const& p);

  auto readBytesAbsolute(std::int64_t readPosition, std::size_t size) -> ByteArray;
  void writeBytesAbsolute(std::int64_t writePosition, ByteArray const& p);

protected:
  void setMode(IOMode mode);

  IODevice(IODevice const&);
  auto operator=(IODevice const&) -> IODevice&;

private:
  std::atomic<IOMode> m_mode;
};

inline auto operator|(IOMode a, IOMode b) -> IOMode {
  return (IOMode)((std::uint8_t)a | (std::uint8_t)b);
}

inline auto operator&(IOMode a, IOMode b) -> bool {
  return (std::uint8_t)a & (std::uint8_t)b;
}

inline auto IODevice::mode() const -> IOMode {
  return m_mode;
}

inline auto IODevice::isOpen() const -> bool {
  return m_mode != IOMode::Closed;
}

inline auto IODevice::isReadable() const -> bool {
  return m_mode & IOMode::Read;
}

inline auto IODevice::isWritable() const -> bool {
  return m_mode & IOMode::Write;
}

}// namespace Star
