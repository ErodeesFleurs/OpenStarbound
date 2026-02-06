#include "StarBuffer.hpp"
#include "StarIODevice.hpp"
#include "StarFormat.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"

import std;

namespace Star {

Buffer::Buffer()
  : m_pos(0) {
  setMode(IOMode::ReadWrite);
}

Buffer::Buffer(std::size_t initialSize)
  : Buffer() {
  reset(initialSize);
}

Buffer::Buffer(ByteArray b)
  : Buffer() {
  reset(std::move(b));
}

Buffer::Buffer(Buffer const& buffer)
  : Buffer() {
  operator=(buffer);
}

Buffer::Buffer(Buffer&& buffer)
  : Buffer() {
  operator=(std::move(buffer));
}

auto Buffer::pos() -> std::int64_t {
  return m_pos;
}

void Buffer::seek(std::int64_t pos, IOSeek mode) {
  std::int64_t newPos = m_pos;
  if (mode == IOSeek::Absolute)
    newPos = pos;
  else if (mode == IOSeek::Relative)
    newPos += pos;
  else if (mode == IOSeek::End)
    newPos = m_bytes.size() - pos;
  m_pos = newPos;
}

void Buffer::resize(std::int64_t size) {
  data().resize((std::size_t)size);
}

auto Buffer::atEnd() -> bool {
  return m_pos >= m_bytes.size();
}

auto Buffer::read(char* data, std::size_t len) -> std::size_t {
  std::size_t l = doRead(m_pos, data, len);
  m_pos += l;
  return l;
}

auto Buffer::write(char const* data, std::size_t len) -> std::size_t {
  std::size_t l = doWrite(m_pos, data, len);
  m_pos += l;
  return l;
}

auto Buffer::readAbsolute(std::int64_t readPosition, char* data, std::size_t len) -> std::size_t {
  std::size_t rpos = readPosition;
  if ((std::int64_t)rpos != readPosition)
    throw IOException("Error, readPosition out of range");

  return doRead(rpos, data, len);
}

auto Buffer::writeAbsolute(std::int64_t writePosition, char const* data, std::size_t len) -> std::size_t {
  std::size_t wpos = writePosition;
  if ((std::int64_t)wpos != writePosition)
    throw IOException("Error, writePosition out of range");

  return doWrite(wpos, data, len);
}

void Buffer::open(IOMode mode) {
  setMode(mode);
  if (mode & IOMode::Write && mode & IOMode::Truncate)
    resize(0);
  if (mode & IOMode::Append)
    seek(0, IOSeek::End);
}

auto Buffer::deviceName() const -> String {
  return strf("Buffer <{}>", (void*)this);
}

auto Buffer::size() -> std::int64_t {
  return m_bytes.size();
}

auto Buffer::data() -> ByteArray& {
  return m_bytes;
}

auto Buffer::data() const -> ByteArray const& {
  return m_bytes;
}

auto Buffer::takeData() -> ByteArray {
  ByteArray ret = std::move(m_bytes);
  reset(0);
  return ret;
}

auto Buffer::ptr() -> char* {
  return data().ptr();
}

auto Buffer::ptr() const -> char const* {
  return m_bytes.ptr();
}

auto Buffer::dataSize() const -> std::size_t {
  return m_bytes.size();
}

void Buffer::reserve(std::size_t size) {
  data().reserve(size);
}

void Buffer::clear() {
  m_pos = 0;
  m_bytes.clear();
}

auto Buffer::empty() const -> bool {
  return m_bytes.empty();
}

void Buffer::reset(std::size_t newSize) {
  m_pos = 0;
  m_bytes.fill(newSize, 0);
}

void Buffer::reset(ByteArray b) {
  m_pos = 0;
  m_bytes = std::move(b);
}

auto Buffer::operator=(Buffer const& buffer) -> Buffer& = default;

auto Buffer::operator=(Buffer&& buffer) -> Buffer& {
  IODevice::operator=(buffer);
  m_pos = buffer.m_pos;
  m_bytes = std::move(buffer.m_bytes);

  buffer.m_pos = 0;
  buffer.m_bytes = ByteArray();

  return *this;
}

auto Buffer::doRead(std::size_t pos, char* data, std::size_t len) -> std::size_t {
  if (len == 0)
    return 0;

  if (!isReadable())
    throw IOException("Error, read called on non-readable Buffer");

  if (pos >= m_bytes.size())
    return 0;

  std::size_t l = std::min(m_bytes.size() - pos, len);
  std::memcpy(data, m_bytes.ptr() + pos, l);
  return l;
}

auto Buffer::doWrite(std::size_t pos, char const* data, std::size_t len) -> std::size_t {
  if (len == 0)
    return 0;

  if (!isWritable())
    throw EofException("Error, write called on non-writable Buffer");

  if (pos + len > m_bytes.size())
    m_bytes.resize(pos + len);

  std::memcpy(m_bytes.ptr() + pos, data, len);
  return len;
}

ExternalBuffer::ExternalBuffer()
  : m_pos(0), m_bytes(nullptr), m_size(0) {
  setMode(IOMode::Read);
}

ExternalBuffer::ExternalBuffer(char const* externalData, std::size_t len) : ExternalBuffer() {
  reset(externalData, len);
}

auto ExternalBuffer::pos() -> std::int64_t {
  return m_pos;
}

void ExternalBuffer::seek(std::int64_t pos, IOSeek mode) {
  std::int64_t newPos = m_pos;
  if (mode == IOSeek::Absolute)
    newPos = pos;
  else if (mode == IOSeek::Relative)
    newPos += pos;
  else if (mode == IOSeek::End)
    newPos = m_size - pos;
  m_pos = newPos;
}

auto ExternalBuffer::atEnd() -> bool {
  return m_pos >= m_size;
}

auto ExternalBuffer::read(char* data, std::size_t len) -> std::size_t {
  std::size_t l = doRead(m_pos, data, len);
  m_pos += l;
  return l;
}

auto ExternalBuffer::write(char const*, std::size_t) -> std::size_t {
  throw IOException("Error, ExternalBuffer is not writable");
}

auto ExternalBuffer::readAbsolute(std::int64_t readPosition, char* data, std::size_t len) -> std::size_t {
  std::size_t rpos = readPosition;
  if ((std::int64_t)rpos != readPosition)
    throw IOException("Error, readPosition out of range");

  return doRead(rpos, data, len);
}

auto ExternalBuffer::writeAbsolute(std::int64_t, char const*, std::size_t) -> std::size_t {
  throw IOException("Error, ExternalBuffer is not writable");
}

auto ExternalBuffer::deviceName() const -> String {
  return strf("ExternalBuffer <{}>", (void*)this);
}

auto ExternalBuffer::size() -> std::int64_t {
  return m_size;
}

auto ExternalBuffer::ptr() const -> char const* {
  return m_bytes;
}

auto ExternalBuffer::dataSize() const -> std::size_t {
  return m_size;
}

auto ExternalBuffer::empty() const -> bool {
  return m_size == 0;
}

ExternalBuffer::operator bool() const {
  return m_size == 0;
}

void ExternalBuffer::reset(char const* externalData, std::size_t len) {
  m_pos = 0;
  m_bytes = externalData;
  m_size = len;
}

auto Buffer::clone() -> Ptr<IODevice> {
  auto cloned = std::make_shared<Buffer>(*this);
  // Reset position to 0 while preserving mode and data
  cloned->seek(0);
  return cloned;
}

auto ExternalBuffer::clone() -> Ptr<IODevice> {
  Logger::info("Cloning ExternalBuffer from position {}", m_pos);
  return std::make_shared<ExternalBuffer>(*this);
}

auto ExternalBuffer::doRead(std::size_t pos, char* data, std::size_t len) -> std::size_t {
  if (len == 0)
    return 0;

  if (!isReadable())
    throw IOException("Error, read called on non-readable Buffer");

  if (pos >= m_size)
    return 0;

  std::size_t l = std::min(m_size - pos, len);
  std::memcpy(data, m_bytes + pos, l);
  return l;
}

}
