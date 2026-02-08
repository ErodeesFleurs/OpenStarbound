#include "StarDataStreamDevices.hpp"

#include "StarConfig.hpp"

import std;

namespace Star {

DataStreamFunctions::DataStreamFunctions(std::function<std::size_t(char*, std::size_t)> reader, std::function<std::size_t(char const*, std::size_t)> writer)
    : m_reader(std::move(reader)), m_writer(std::move(writer)) {}

void DataStreamFunctions::readData(char* data, std::size_t len) {
  if (!m_reader)
    throw DataStreamException("DataStreamFunctions no read function given");
  m_reader(data, len);
}

void DataStreamFunctions::writeData(char const* data, std::size_t len) {
  if (!m_writer)
    throw DataStreamException("DataStreamFunctions no write function given");
  m_writer(data, len);
}

DataStreamIODevice::DataStreamIODevice(Ptr<IODevice> device)
    : m_device(std::move(device)) {}

auto DataStreamIODevice::device() const -> Ptr<IODevice> const& {
  return m_device;
}

void DataStreamIODevice::seek(std::size_t pos, IOSeek mode) {
  m_device->seek(pos, mode);
}

auto DataStreamIODevice::atEnd() -> bool {
  return m_device->atEnd();
}

auto DataStreamIODevice::pos() -> std::int64_t {
  return m_device->pos();
}

void DataStreamIODevice::readData(char* data, std::size_t len) {
  return m_device->readFull(data, len);
}

void DataStreamIODevice::writeData(char const* data, std::size_t len) {
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

void DataStreamBuffer::resize(std::size_t size) {
  m_buffer->resize(size);
}

void DataStreamBuffer::reserve(std::size_t size) {
  m_buffer->reserve(size);
}

void DataStreamBuffer::clear() {
  m_buffer->clear();
}

auto DataStreamBuffer::device() const -> Ptr<Buffer> const& {
  return m_buffer;
}

auto DataStreamBuffer::data() -> ByteArray& {
  return m_buffer->data();
}

auto DataStreamBuffer::data() const -> ByteArray const& {
  return m_buffer->data();
}

auto DataStreamBuffer::takeData() -> ByteArray {
  return m_buffer->takeData();
}

auto DataStreamBuffer::ptr() -> char* {
  return m_buffer->ptr();
}

auto DataStreamBuffer::ptr() const -> const char* {
  return m_buffer->ptr();
}

auto DataStreamBuffer::size() const -> std::size_t {
  return m_buffer->dataSize();
}

auto DataStreamBuffer::empty() const -> bool {
  return m_buffer->empty();
}

void DataStreamBuffer::seek(std::size_t pos, IOSeek mode) {
  m_buffer->seek(pos, mode);
}

auto DataStreamBuffer::atEnd() -> bool {
  return m_buffer->atEnd();
}

auto DataStreamBuffer::pos() -> std::size_t {
  return (std::size_t)m_buffer->pos();
}

void DataStreamBuffer::reset(std::size_t newSize) {
  m_buffer->reset(newSize);
}

void DataStreamBuffer::reset(ByteArray b) {
  m_buffer->reset(std::move(b));
}

void DataStreamBuffer::readData(char* data, std::size_t len) {
  m_buffer->readFull(data, len);
}

void DataStreamBuffer::writeData(char const* data, std::size_t len) {
  m_buffer->writeFull(data, len);
}

DataStreamExternalBuffer::DataStreamExternalBuffer() : m_buffer() {}

DataStreamExternalBuffer::DataStreamExternalBuffer(ByteArray const& byteArray) : DataStreamExternalBuffer(byteArray.ptr(), byteArray.size()) {}

DataStreamExternalBuffer::DataStreamExternalBuffer(DataStreamBuffer const& buffer) : DataStreamExternalBuffer(buffer.ptr(), buffer.size()) {}

DataStreamExternalBuffer::DataStreamExternalBuffer(char const* externalData, std::size_t len) : DataStreamExternalBuffer() {
  reset(externalData, len);
}

auto DataStreamExternalBuffer::ptr() const -> char const* {
  return m_buffer.ptr();
}

auto DataStreamExternalBuffer::size() const -> std::size_t {
  return m_buffer.dataSize();
}

auto DataStreamExternalBuffer::empty() const -> bool {
  return m_buffer.empty();
}

void DataStreamExternalBuffer::seek(std::size_t pos, IOSeek mode) {
  m_buffer.seek(pos, mode);
}

auto DataStreamExternalBuffer::atEnd() -> bool {
  return m_buffer.atEnd();
}

auto DataStreamExternalBuffer::pos() -> std::size_t {
  return m_buffer.pos();
}

auto DataStreamExternalBuffer::remaining() -> std::size_t {
  return m_buffer.dataSize() - m_buffer.pos();
}

void DataStreamExternalBuffer::reset(char const* externalData, std::size_t len) {
  m_buffer.reset(externalData, len);
}

void DataStreamExternalBuffer::readData(char* data, std::size_t len) {
  m_buffer.readFull(data, len);
}
void DataStreamExternalBuffer::writeData(char const* data, std::size_t len) {
  m_buffer.writeFull(data, len);
}

}// namespace Star
