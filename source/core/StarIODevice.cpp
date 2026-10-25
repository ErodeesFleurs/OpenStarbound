import std;
import star.io_device;
import star.string;
import star.format;
import star.byte_array;

namespace Star {

IODevice::IODevice(IOMode mode) : m_mode(mode) {}
IODevice::~IODevice() = default;

std::expected<void, StarException> IODevice::resize(StreamOffset) {
  return std::unexpected(IOException("resize not supported"));
}

std::expected<void, StarException> IODevice::readFull(std::span<char> data) {
  auto count = read(data);
  if (!count) return std::unexpected(std::move(count.error()));
  if (*count != data.size()) {
    auto end = atEnd();
    if (!end) return std::unexpected(std::move(end.error()));
    if (*end) return std::unexpected(EofException("Failed to read full buffer in readFull, eof reached."));
    return std::unexpected(IOException("Failed to read full buffer in readFull"));
  }
  return {};
}

std::expected<void, StarException> IODevice::writeFull(std::span<char const> data) {
  auto count = write(data);
  if (!count) return std::unexpected(std::move(count.error()));
  if (*count != data.size()) {
    auto end = atEnd();
    if (!end) return std::unexpected(std::move(end.error()));
    if (*end) return std::unexpected(EofException("Failed to write full buffer in writeFull, eof reached."));
    return std::unexpected(IOException("Failed to write full buffer in writeFull"));
  }
  return {};
}

std::expected<void, StarException> IODevice::open(IOMode mode) {
  if (mode != m_mode) {
    auto name = deviceName();
    if (!name) return std::unexpected(std::move(name.error()));
    return std::unexpected(IOException::format("Cannot reopen device '{}'", *name));
  }
  return {};
}

std::expected<void, StarException> IODevice::close() {
  m_mode = IOMode::Closed;
  return {};
}

std::expected<void, StarException> IODevice::sync() { return {}; }

std::expected<String, StarException> IODevice::deviceName() const {
  return strf("IODevice <{}>", (void*)this);
}

std::expected<bool, StarException> IODevice::atEnd() {
  auto position = pos();
  if (!position) return std::unexpected(std::move(position.error()));
  auto length = size();
  if (!length) return std::unexpected(std::move(length.error()));
  return *position >= *length;
}

std::expected<StreamOffset, StarException> IODevice::size() {
  auto stored = pos();
  if (!stored) return std::unexpected(IOException("Cannot call size() on IODevice", stored.error()));
  auto moved = seek(0, IOSeek::End);
  if (!moved) return std::unexpected(IOException("Cannot call size() on IODevice", moved.error()));
  auto length = pos();
  auto restored = seek(*stored);
  if (!length) return std::unexpected(IOException("Cannot call size() on IODevice", length.error()));
  if (!restored) return std::unexpected(IOException("Cannot restore position after size() on IODevice", restored.error()));
  return *length;
}

std::expected<std::size_t, StarException> IODevice::readAbsolute(StreamOffset readPosition, std::span<char> data) {
  auto stored = pos();
  if (!stored) return std::unexpected(std::move(stored.error()));
  auto moved = seek(readPosition);
  if (!moved) return std::unexpected(std::move(moved.error()));
  auto count = read(data);
  auto restored = seek(*stored);
  if (!count) return std::unexpected(std::move(count.error()));
  if (!restored) return std::unexpected(std::move(restored.error()));
  return *count;
}

std::expected<std::size_t, StarException> IODevice::writeAbsolute(StreamOffset writePosition, std::span<char const> data) {
  auto stored = pos();
  if (!stored) return std::unexpected(std::move(stored.error()));
  auto moved = seek(writePosition);
  if (!moved) return std::unexpected(std::move(moved.error()));
  auto count = write(data);
  auto restored = seek(*stored);
  if (!count) return std::unexpected(std::move(count.error()));
  if (!restored) return std::unexpected(std::move(restored.error()));
  return *count;
}

std::expected<void, StarException> IODevice::readFullAbsolute(StreamOffset position, std::span<char> data) {
  while (!data.empty()) {
    auto count = readAbsolute(position, data);
    if (!count) return std::unexpected(std::move(count.error()));
    if (*count == 0 || *count > data.size())
      return std::unexpected(IOException("Failed to read full buffer in readFullAbsolute"));
    position += *count;
    data = data.subspan(*count);
  }
  return {};
}

std::expected<void, StarException> IODevice::writeFullAbsolute(StreamOffset position, std::span<char const> data) {
  while (!data.empty()) {
    auto count = writeAbsolute(position, data);
    if (!count) return std::unexpected(std::move(count.error()));
    if (*count == 0 || *count > data.size())
      return std::unexpected(IOException("Failed to write full buffer in writeFullAbsolute"));
    position += *count;
    data = data.subspan(*count);
  }
  return {};
}

std::expected<ByteArray, StarException> IODevice::readBytes(std::size_t size) {
  ByteArray bytes;
  bytes.resize(size);
  if (size) {
    auto result = readFull(bytes);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return bytes;
}

std::expected<void, StarException> IODevice::writeBytes(ByteArray const& bytes) {
  return writeFull(bytes);
}

std::expected<ByteArray, StarException> IODevice::readBytesAbsolute(StreamOffset position, std::size_t size) {
  ByteArray bytes;
  bytes.resize(size);
  if (size) {
    auto result = readFullAbsolute(position, bytes);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return bytes;
}

std::expected<void, StarException> IODevice::writeBytesAbsolute(StreamOffset position, ByteArray const& bytes) {
  return writeFullAbsolute(position, bytes);
}

void IODevice::setMode(IOMode mode) { m_mode = mode; }
IODevice::IODevice(IODevice const& rhs) : m_mode(rhs.mode()) {}
IODevice& IODevice::operator=(IODevice const& rhs) {
  m_mode = rhs.mode();
  return *this;
}

}
