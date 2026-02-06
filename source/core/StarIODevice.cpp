#include "StarIODevice.hpp"
#include "StarMathCommon.hpp"
#include "StarFormat.hpp"

namespace Star {

IODevice::IODevice(IOMode mode) {
  m_mode = mode;
}

IODevice::~IODevice() {
  close();
}

void IODevice::resize(std::int64_t) {
  throw IOException("resize not supported");
}

void IODevice::readFull(char* data, size_t len) {
  size_t r = read(data, len);
  if (r < len) {
    if (atEnd())
      throw EofException("Failed to read full buffer in readFull, eof reached.");
    else
      throw IOException("Failed to read full buffer in readFull");
  }
  data += r;
  len -= r;
}

void IODevice::writeFull(char const* data, size_t len) {
  size_t r = write(data, len);
  if (r < len) {
    if (atEnd())
      throw EofException("Failed to write full buffer in writeFull, eof reached.");
    else
      throw IOException("Failed to write full buffer in writeFull");
  }
  data += r;
  len -= r;
}

void IODevice::open(IOMode mode) {
  if (mode != m_mode)
    throw IOException::format("Cannot reopen device '{}", deviceName());
}

void IODevice::close() {
  m_mode = IOMode::Closed;
}

void IODevice::sync() {}

String IODevice::deviceName() const {
  return strf("IODevice <{}>", (void*)this);
}

bool IODevice::atEnd() {
  return pos() >= size();
}

std::int64_t IODevice::size() {
  try {
    std::int64_t storedPos = pos();
    seek(0, IOSeek::End);
    std::int64_t size = pos();
    seek(storedPos);
    return size;
  } catch (IOException const& e) {
    throw IOException("Cannot call size() on IODevice", e);
  }
}

size_t IODevice::readAbsolute(std::int64_t readPosition, char* data, size_t len) {
  std::int64_t storedPos = pos();
  seek(readPosition);
  size_t ret = read(data, len);
  seek(storedPos);
  return ret;
}

size_t IODevice::writeAbsolute(std::int64_t writePosition, char const* data, size_t len) {
  std::int64_t storedPos = pos();
  seek(writePosition);
  size_t ret = write(data, len);
  seek(storedPos);
  return ret;
}

void IODevice::readFullAbsolute(std::int64_t readPosition, char* data, size_t len) {
  while (len > 0) {
    size_t r = readAbsolute(readPosition, data, len);
    if (r == 0)
      throw IOException("Failed to read full buffer in readFullAbsolute");
    readPosition += r;
    data += r;
    len -= r;
  }
}

void IODevice::writeFullAbsolute(std::int64_t writePosition, char const* data, size_t len) {
  while (len > 0) {
    size_t r = writeAbsolute(writePosition, data, len);
    if (r == 0)
      throw IOException("Failed to write full buffer in writeFullAbsolute");
    writePosition += r;
    data += r;
    len -= r;
  }
}

ByteArray IODevice::readBytes(size_t size) {
  if (!size)
    return {};

  ByteArray p;
  p.resize(size);
  readFull(p.ptr(), size);
  return p;
}

void IODevice::writeBytes(ByteArray const& p) {
  writeFull(p.ptr(), p.size());
}

ByteArray IODevice::readBytesAbsolute(std::int64_t readPosition, size_t size) {
  if (!size)
    return {};

  ByteArray p;
  p.resize(size);
  readFullAbsolute(readPosition, p.ptr(), size);
  return p;
}

void IODevice::writeBytesAbsolute(std::int64_t writePosition, ByteArray const& p) {
  writeFullAbsolute(writePosition, p.ptr(), p.size());
}

void IODevice::setMode(IOMode m) {
  m_mode = m;
}

IODevice::IODevice(IODevice const& rhs) {
  m_mode = rhs.mode();
}

IODevice& IODevice::operator=(IODevice const& rhs) {
  m_mode = rhs.mode();
  return *this;
}

}
