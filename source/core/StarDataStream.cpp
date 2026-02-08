#include "StarDataStream.hpp"
#include "StarBytes.hpp"
#include "StarVlqEncoding.hpp"

import std;

namespace Star {

unsigned const CurrentStreamVersion = 14;// update OpenProtocolVersion too!

DataStream::DataStream()
    : m_byteOrder(ByteOrder::BigEndian),
      m_nullTerminatedStrings(false),
      m_streamCompatibilityVersion(CurrentStreamVersion) {}

auto DataStream::byteOrder() const -> ByteOrder {
  return m_byteOrder;
}

void DataStream::setByteOrder(ByteOrder byteOrder) {
  m_byteOrder = byteOrder;
}

auto DataStream::nullTerminatedStrings() const -> bool {
  return m_nullTerminatedStrings;
}

void DataStream::setNullTerminatedStrings(bool nullTerminatedStrings) {
  m_nullTerminatedStrings = nullTerminatedStrings;
}

auto DataStream::streamCompatibilityVersion() const -> unsigned {
  return m_streamCompatibilityVersion;
}

void DataStream::setStreamCompatibilityVersion(unsigned streamCompatibilityVersion) {
  m_streamCompatibilityVersion = streamCompatibilityVersion;
}

void DataStream::setStreamCompatibilityVersion(NetCompatibilityRules const& rules) {
  m_streamCompatibilityVersion = rules.version();
}

auto DataStream::readBytes(std::size_t len) -> ByteArray {
  ByteArray ba;
  ba.resize(len);
  readData(ba.ptr(), len);
  return ba;
}

void DataStream::writeBytes(ByteArray const& ba) {
  writeData(ba.ptr(), ba.size());
}

auto DataStream::operator<<(bool d) -> DataStream& {
  operator<<((std::uint8_t)d);
  return *this;
}

auto DataStream::operator<<(char c) -> DataStream& {
  writeData(&c, 1);
  return *this;
}

auto DataStream::operator<<(std::int8_t d) -> DataStream& {
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(std::uint8_t d) -> DataStream& {
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(std::int16_t d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(std::uint16_t d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(std::int32_t d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(std::uint32_t d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(std::int64_t d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(std::uint64_t d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(float d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator<<(double d) -> DataStream& {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator>>(bool& d) -> DataStream& {
  std::uint8_t bu;
  readData((char*)&bu, sizeof(bu));
  d = (bool)bu;
  return *this;
}

auto DataStream::operator>>(char& c) -> DataStream& {
  readData(&c, 1);
  return *this;
}

auto DataStream::operator>>(std::int8_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator>>(std::uint8_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  return *this;
}

auto DataStream::operator>>(std::int16_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::operator>>(std::uint16_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::operator>>(std::int32_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::operator>>(std::uint32_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::operator>>(std::int64_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::operator>>(std::uint64_t& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::operator>>(float& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::operator>>(double& d) -> DataStream& {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

auto DataStream::writeVlqU(std::uint64_t i) -> std::size_t {
  return Star::writeVlqU(i, makeFunctionOutputIterator([this](std::uint8_t b) -> void { *this << b; }));
}

auto DataStream::writeVlqI(std::int64_t i) -> std::size_t {
  return Star::writeVlqI(i, makeFunctionOutputIterator([this](std::uint8_t b) -> void { *this << b; }));
}

auto DataStream::writeVlqS(std::size_t i) -> std::size_t {
  std::uint64_t i64;
  if (i == std::numeric_limits<std::size_t>::max())
    i64 = 0;
  else
    i64 = i + 1;
  return writeVlqU(i64);
}

auto DataStream::readVlqU(std::uint64_t& i) -> std::size_t {
  std::size_t bytesRead = Star::readVlqU(i, makeFunctionInputIterator([this]() -> unsigned char { return this->read<std::uint8_t>(); }));

  if (bytesRead == std::numeric_limits<std::size_t>::max())
    throw DataStreamException("Error reading VLQ encoded integer!");

  return bytesRead;
}

auto DataStream::readVlqI(std::int64_t& i) -> std::size_t {
  std::size_t bytesRead = Star::readVlqI(i, makeFunctionInputIterator([this]() -> unsigned char { return this->read<std::uint8_t>(); }));

  if (bytesRead == std::numeric_limits<std::size_t>::max())
    throw DataStreamException("Error reading VLQ encoded integer!");

  return bytesRead;
}

auto DataStream::readVlqS(std::size_t& i) -> std::size_t {
  std::uint64_t i64;
  std::size_t res = readVlqU(i64);
  if (i64 == 0)
    i = std::numeric_limits<std::size_t>::max();
  else
    i = (std::size_t)(i64 - 1);
  return res;
}

auto DataStream::readVlqU() -> std::uint64_t {
  std::uint64_t i;
  readVlqU(i);
  return i;
}

auto DataStream::readVlqI() -> std::int64_t {
  std::int64_t i;
  readVlqI(i);
  return i;
}

auto DataStream::readVlqS() -> std::size_t {
  std::size_t i;
  readVlqS(i);
  return i;
}

auto DataStream::operator<<(char const* s) -> DataStream& {
  writeStringData(s, std::strlen(s));
  return *this;
}

auto DataStream::operator<<(std::string const& d) -> DataStream& {
  writeStringData(d.c_str(), d.size());
  return *this;
}

auto DataStream::operator<<(const ByteArray& d) -> DataStream& {
  writeVlqU(d.size());
  writeData(d.ptr(), d.size());
  return *this;
}

auto DataStream::operator<<(const String& s) -> DataStream& {
  writeStringData(s.utf8Ptr(), s.utf8Size());
  return *this;
}

auto DataStream::operator>>(std::string& d) -> DataStream& {
  if (m_nullTerminatedStrings) {
    d.clear();
    char c;
    while (true) {
      readData((char*)&c, sizeof(c));
      if (c == '\0')
        break;
      d.push_back(c);
    }
  } else {
    d.resize((std::size_t)readVlqU());
    readData(&d[0], d.size());
  }
  return *this;
}

auto DataStream::operator>>(ByteArray& d) -> DataStream& {
  d.resize((std::size_t)readVlqU());
  readData(d.ptr(), d.size());
  return *this;
}

auto DataStream::operator>>(String& s) -> DataStream& {
  std::string string;
  operator>>(string);
  s = std::move(string);
  return *this;
}

void DataStream::writeStringData(char const* data, std::size_t len) {
  if (m_nullTerminatedStrings) {
    writeData(data, len);
    operator<<((std::uint8_t)0x00);
  } else {
    writeVlqU(len);
    writeData(data, len);
  }
}

}// namespace Star
