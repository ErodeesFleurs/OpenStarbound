import star.data_stream_devices;
import std;
import star.io_device;
import star.data_stream;
import star.buffer;

import star.byte_array;
import star.format;
namespace Star {

DataStreamFunctions::DataStreamFunctions(std::function<std::expected<std::size_t, StarException>(char*, std::size_t)> reader,
    std::function<std::expected<std::size_t, StarException>(char const*, std::size_t)> writer)
  : m_reader(std::move(reader)), m_writer(std::move(writer)) {}

[[nodiscard]] std::expected<void, StarException> DataStreamFunctions::readData(char* data, std::size_t len) {
  if (!m_reader)
    return std::unexpected(DataStreamException("DataStreamFunctions no read function given"));
  auto result = m_reader(data, len);
  if (!result)
    return std::unexpected(std::move(result.error()));
  if (*result != len)
    return std::unexpected(EofException("DataStreamFunctions did not read full length"));
  return {};
}

[[nodiscard]] std::expected<void, StarException> DataStreamFunctions::writeData(char const* data, std::size_t len) {
  if (!m_writer)
    return std::unexpected(DataStreamException("DataStreamFunctions no write function given"));
  auto result = m_writer(data, len);
  if (!result)
    return std::unexpected(std::move(result.error()));
  if (*result != len)
    return std::unexpected(EofException("DataStreamFunctions did not write full length"));
  return {};
}

DataStreamIODevice::DataStreamIODevice(std::shared_ptr<IODevice> device)
  : m_device(std::move(device)) {}

std::shared_ptr<IODevice> const& DataStreamIODevice::device() const {
  return m_device;
}

[[nodiscard]] std::expected<void, StarException> DataStreamIODevice::seek(std::size_t pos, IOSeek mode) {
  if (!std::in_range<StreamOffset>(pos))
    return std::unexpected(IOException("Error, seek position out of range"));
  return m_device->seek(static_cast<StreamOffset>(pos), mode);
}

[[nodiscard]] std::expected<bool, StarException> DataStreamIODevice::atEnd() {
  return m_device->atEnd();
}

[[nodiscard]] std::expected<StreamOffset, StarException> DataStreamIODevice::pos() {
  return m_device->pos();
}

[[nodiscard]] std::expected<void, StarException> DataStreamIODevice::readData(char* data, std::size_t len) {
  return m_device->readFull(data, len);
}

[[nodiscard]] std::expected<void, StarException> DataStreamIODevice::writeData(char const* data, std::size_t len) {
  return m_device->writeFull(data, len);
}

DataStreamBuffer::DataStreamBuffer() {
  m_buffer = std::make_shared<Buffer>();
}

DataStreamBuffer::DataStreamBuffer(std::size_t s)
  : DataStreamBuffer() {
  reset(s);
}

DataStreamBuffer::DataStreamBuffer(ByteArray b)
  : DataStreamBuffer() {
  reset(std::move(b));
}

[[nodiscard]] std::expected<void, StarException> DataStreamBuffer::resize(std::size_t size) {
  if (!std::in_range<StreamOffset>(size))
    return std::unexpected(IOException("Error, buffer size out of range"));
  return m_buffer->resize(static_cast<StreamOffset>(size));
}

void DataStreamBuffer::reserve(std::size_t size) {
  m_buffer->reserve(size);
}

void DataStreamBuffer::clear() {
  m_buffer->clear();
}

std::shared_ptr<Buffer> const& DataStreamBuffer::device() const {
  return m_buffer;
}

ByteArray& DataStreamBuffer::data() {
  return m_buffer->data();
}

ByteArray const& DataStreamBuffer::data() const {
  return m_buffer->data();
}

ByteArray DataStreamBuffer::takeData() {
  return m_buffer->takeData();
}

char* DataStreamBuffer::ptr() {
  return m_buffer->ptr();
}

const char* DataStreamBuffer::ptr() const {
  return m_buffer->ptr();
}

std::size_t DataStreamBuffer::size() const {
  return m_buffer->dataSize();
}

bool DataStreamBuffer::empty() const {
  return m_buffer->empty();
}

[[nodiscard]] std::expected<void, StarException> DataStreamBuffer::seek(std::size_t pos, IOSeek mode) {
  if (!std::in_range<StreamOffset>(pos))
    return std::unexpected(IOException("Error, seek position out of range"));
  return m_buffer->seek(static_cast<StreamOffset>(pos), mode);
}

[[nodiscard]] std::expected<bool, StarException> DataStreamBuffer::atEnd() {
  return m_buffer->atEnd();
}

[[nodiscard]] std::expected<std::size_t, StarException> DataStreamBuffer::pos() {
  auto result = m_buffer->pos();
  if (!result)
    return std::unexpected(std::move(result.error()));
  return static_cast<std::size_t>(*result);
}

void DataStreamBuffer::reset(std::size_t newSize) {
  m_buffer->reset(newSize);
}

void DataStreamBuffer::reset(ByteArray b) {
  m_buffer->reset(std::move(b));
}

[[nodiscard]] std::expected<void, StarException> DataStreamBuffer::readData(char* data, std::size_t len) {
  return m_buffer->readFull(data, len);
}

[[nodiscard]] std::expected<void, StarException> DataStreamBuffer::writeData(char const* data, std::size_t len) {
  return m_buffer->writeFull(data, len);
}

DataStreamExternalBuffer::DataStreamExternalBuffer() : m_buffer() {}

DataStreamExternalBuffer::DataStreamExternalBuffer(std::span<char const> externalData) : DataStreamExternalBuffer() {
  reset(externalData);
}

char const* DataStreamExternalBuffer::ptr() const {
  return m_buffer.ptr();
}

std::size_t DataStreamExternalBuffer::size() const {
  return m_buffer.dataSize();
}

bool DataStreamExternalBuffer::empty() const {
  return m_buffer.empty();
}

[[nodiscard]] std::expected<void, StarException> DataStreamExternalBuffer::seek(std::size_t pos, IOSeek mode) {
  if (!std::in_range<StreamOffset>(pos))
    return std::unexpected(IOException("Error, seek position out of range"));
  return m_buffer.seek(static_cast<StreamOffset>(pos), mode);
}

[[nodiscard]] std::expected<bool, StarException> DataStreamExternalBuffer::atEnd() {
  return m_buffer.atEnd();
}

[[nodiscard]] std::expected<std::size_t, StarException> DataStreamExternalBuffer::pos() {
  auto result = m_buffer.pos();
  if (!result)
    return std::unexpected(std::move(result.error()));
  return static_cast<std::size_t>(*result);
}

[[nodiscard]] std::expected<std::size_t, StarException> DataStreamExternalBuffer::remaining() {
  auto result = pos();
  if (!result)
    return std::unexpected(std::move(result.error()));
  if (*result > m_buffer.dataSize())
    return std::unexpected(EofException("Error, buffer position is past end"));
  return m_buffer.dataSize() - *result;
}

void DataStreamExternalBuffer::reset(std::span<char const> externalData) {
  m_buffer.reset(externalData);
}

[[nodiscard]] std::expected<void, StarException> DataStreamExternalBuffer::readData(char* data, std::size_t len) {
  return m_buffer.readFull(data, len);
}
[[nodiscard]] std::expected<void, StarException> DataStreamExternalBuffer::writeData(char const* data, std::size_t len) {
  return m_buffer.writeFull(data, len);
}

}
