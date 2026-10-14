module;
import std;

import star.byte_array;
import star.string;
import star.format;

namespace Star {
using StreamOffset = std::int64_t;


class IODevice;
using IODevicePtr = std::shared_ptr<IODevice>;
using IODeviceConstPtr = std::shared_ptr<const IODevice>;
using IODeviceWeakPtr = std::weak_ptr<IODevice>;
using IODeviceConstWeakPtr = std::weak_ptr<const IODevice>;
using IODeviceUPtr = std::unique_ptr<IODevice>;
using IODeviceConstUPtr = std::unique_ptr<const IODevice>;

struct EofExceptionTag {
  static constexpr char const* name() { return "EofException"; }
};
using EofException = StarError<EofExceptionTag, IOException>;

enum class IOMode : std::uint8_t {
  Closed = 0x0,
  Read = 0x1,
  Write = 0x2,
  ReadWrite = 0x3,
  Append = 0x4,
  Truncate = 0x8,
};

IOMode operator|(IOMode a, IOMode b);
bool operator&(IOMode a, IOMode b);

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
  virtual std::size_t read(char* data, std::size_t len) = 0;
  virtual std::size_t write(char const* data, std::size_t len) = 0;

  virtual StreamOffset pos() = 0;
  virtual void seek(StreamOffset pos, IOSeek mode = IOSeek::Absolute) = 0;

  // Default implementation throws unsupported exception.
  virtual void resize(StreamOffset size);

  // Read / write from an absolute offset in the file without modifying the
  // current file position.  Default implementation stores the file position,
  // then seeks and calls read/write partial, then restores the file position,
  // and is not thread safe.
  virtual std::size_t readAbsolute(StreamOffset readPosition, char* data, std::size_t len);
  virtual std::size_t writeAbsolute(StreamOffset writePosition, char const* data, std::size_t len);

  // Read and write fully, and throw an exception in every other case.  The
  // default implementations here will call the normal read or write, and if
  // the full amount is not read will throw an exception.
  virtual void readFull(char* data, std::size_t len);
  virtual void writeFull(char const* data, std::size_t len);
  virtual void readFullAbsolute(StreamOffset readPosition, char* data, std::size_t len);
  virtual void writeFullAbsolute(StreamOffset writePosition, char const* data, std::size_t len);

  // Default implementation throws exception if opening in a different mode
  // than the current mode.
  virtual void open(IOMode mode);

  // Default implementation sets mode equal to Closed
  virtual void close();

  // Default implementation is a no-op
  virtual void sync();

  // Returns a clone of this device with the same mode
  virtual IODevicePtr clone() = 0;

  // Default implementation just prints address of generic IODevice
  virtual String deviceName() const;

  // Is the file position at the end of the file and there is no more to read?
  // This is not the same as feof, which returns true after an unsuccesful read
  // past the end, it should return true after succesfully reading the final
  // byte.  Default implementation returns pos() >= size();
  virtual bool atEnd();

  // Default is to store position, seek end, then restore position.
  virtual StreamOffset size();

  IOMode mode() const;
  bool isOpen() const;
  bool isReadable() const;
  bool isWritable() const;

  ByteArray readBytes(std::size_t size);
  void writeBytes(ByteArray const& p);

  ByteArray readBytesAbsolute(StreamOffset readPosition, std::size_t size);
  void writeBytesAbsolute(StreamOffset writePosition, ByteArray const& p);

protected:
  void setMode(IOMode mode);

  IODevice(IODevice const&);
  IODevice& operator=(IODevice const&);

private:
  std::atomic<IOMode> m_mode;
};

inline IOMode operator|(IOMode a, IOMode b) {
  return (IOMode)((std::uint8_t)a | (std::uint8_t)b);
}

inline bool operator&(IOMode a, IOMode b) {
  return (std::uint8_t)a & (std::uint8_t)b;
}

inline IOMode IODevice::mode() const {
  return m_mode;
}

inline bool IODevice::isOpen() const {
  return m_mode != IOMode::Closed;
}

inline bool IODevice::isReadable() const {
  return m_mode & IOMode::Read;
}

inline bool IODevice::isWritable() const {
  return m_mode & IOMode::Write;
}

}

export module star.io_device;

export namespace Star {
  using ::Star::StreamOffset;
  using ::Star::EofExceptionTag;
  using ::Star::EofException;
  using ::Star::IOMode;
  using ::Star::IOSeek;
  using ::Star::Absolute;
  using ::Star::Relative;
  using ::Star::End;
  using ::Star::IODevice;
  using ::Star::IODevicePtr;
  using ::Star::IODeviceConstPtr;
  using ::Star::IODeviceWeakPtr;
  using ::Star::IODeviceConstWeakPtr;
  using ::Star::IODeviceUPtr;
  using ::Star::IODeviceConstUPtr;
  using ::Star::operator|;
  using ::Star::operator&;
}
