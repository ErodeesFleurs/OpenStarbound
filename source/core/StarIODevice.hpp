#pragma once

#include "StarByteArray.hpp"
#include "StarString.hpp"

namespace Star {

class IODevice;
using IODevicePtr = SharedPtr<IODevice>;

struct EofExceptionTag {
  static constexpr char const* typeName = "EofException";
};
using EofException = TypedException<IOException, EofExceptionTag>;

enum class IOMode : uint8_t {
  Closed = 0x0,
  Read = 0x1,
  Write = 0x2,
  ReadWrite = 0x3,
  Append = 0x4,
  Truncate = 0x8,
};

[[nodiscard]] IOMode operator|(IOMode a, IOMode b);
[[nodiscard]] bool operator&(IOMode a, IOMode b);

// Should match SEEK_SET, SEEK_CUR, AND SEEK_END
enum IOSeek : uint8_t {
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
  [[nodiscard]] virtual size_t read(char* data, size_t len) = 0;
  [[nodiscard]] virtual size_t write(char const* data, size_t len) = 0;

  // std::span convenience overloads (delegate to virtual methods)
  [[nodiscard]] size_t read(std::span<char> data) { return read(data.data(), data.size()); }
  [[nodiscard]] size_t write(std::span<char const> data) { return write(data.data(), data.size()); }

  [[nodiscard]] virtual StreamOffset pos() = 0;
  virtual void seek(StreamOffset pos, IOSeek mode = IOSeek::Absolute) = 0;

  // Default implementation throws unsupported exception.
  virtual void resize(StreamOffset size);

  // Read / write from an absolute offset in the file without modifying the
  // current file position.  Default implementation stores the file position,
  // then seeks and calls read/write partial, then restores the file position,
  // and is not thread safe.
  [[nodiscard]] virtual size_t readAbsolute(StreamOffset readPosition, char* data, size_t len);
  [[nodiscard]] virtual size_t writeAbsolute(StreamOffset writePosition, char const* data, size_t len);

  // Read and write fully, and throw an exception in every other case.  The
  // default implementations here will call the normal read or write, and if
  // the full amount is not read will throw an exception.
  virtual void readFull(char* data, size_t len);
  virtual void writeFull(char const* data, size_t len);
  virtual void readFullAbsolute(StreamOffset readPosition, char* data, size_t len);
  virtual void writeFullAbsolute(StreamOffset writePosition, char const* data, size_t len);

  // Default implementation throws exception if opening in a different mode
  // than the current mode.
  virtual void open(IOMode mode);

  // Default implementation sets mode equal to Closed
  virtual void close();

  // Default implementation is a no-op
  virtual void sync();

  // Returns a clone of this device with the same mode
  [[nodiscard]] virtual IODevicePtr clone() = 0;

  // Default implementation just prints address of generic IODevice
  [[nodiscard]] virtual String deviceName() const;

  // Is the file position at the end of the file and there is no more to read?
  // This is not the same as feof, which returns true after an unsuccessful read
  // past the end, it should return true after successfully reading the final
  // byte.  Default implementation returns pos() >= size();
  [[nodiscard]] virtual bool atEnd();

  // Default is to store position, seek end, then restore position.
  [[nodiscard]] virtual StreamOffset size();

  [[nodiscard]] IOMode mode() const;
  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool isReadable() const;
  [[nodiscard]] bool isWritable() const;

  [[nodiscard]] ByteArray readBytes(size_t size);
  void writeBytes(ByteArray const& p);

  [[nodiscard]] ByteArray readBytesAbsolute(StreamOffset readPosition, size_t size);
  void writeBytesAbsolute(StreamOffset writePosition, ByteArray const& p);

protected:
  void setMode(IOMode mode);

  IODevice(IODevice const&);
  IODevice& operator=(IODevice const&);

private:
  atomic<IOMode> m_mode;
};

[[nodiscard]] inline IOMode operator|(IOMode a, IOMode b) {
  return static_cast<IOMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

[[nodiscard]] inline bool operator&(IOMode a, IOMode b) {
  return static_cast<uint8_t>(a) & static_cast<uint8_t>(b);
}

[[nodiscard]] inline IOMode IODevice::mode() const {
  return m_mode;
}

[[nodiscard]] inline bool IODevice::isOpen() const {
  return m_mode != IOMode::Closed;
}

[[nodiscard]] inline bool IODevice::isReadable() const {
  return m_mode & IOMode::Read;
}

[[nodiscard]] inline bool IODevice::isWritable() const {
  return m_mode & IOMode::Write;
}

}// namespace Star
