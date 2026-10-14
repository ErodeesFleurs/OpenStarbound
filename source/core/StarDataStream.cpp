#include <string.h>
import std;
import star.memory;
import star.data_stream;
import star.string;
import star.format;
import star.bytes;


import star.vlq;

import star.net_compatibility;
import star.byte_array;
import star.algorithm;
namespace Star {

extern unsigned const CurrentStreamVersion = 18; // update OpenProtocolVersion too!

DataStream::DataStream()
  : m_byteOrder(ByteOrder::BigEndian),
    m_nullTerminatedStrings(false),
    m_streamCompatibilityVersion(CurrentStreamVersion) {}

ByteOrder DataStream::byteOrder() const {
  return m_byteOrder;
}

void DataStream::setByteOrder(ByteOrder byteOrder) {
  m_byteOrder = byteOrder;
}

bool DataStream::nullTerminatedStrings() const {
  return m_nullTerminatedStrings;
}

void DataStream::setNullTerminatedStrings(bool nullTerminatedStrings) {
  m_nullTerminatedStrings = nullTerminatedStrings;
}

unsigned DataStream::streamCompatibilityVersion() const {
  return m_streamCompatibilityVersion;
}

void DataStream::setStreamCompatibilityVersion(unsigned streamCompatibilityVersion) {
  m_streamCompatibilityVersion = streamCompatibilityVersion;
}

void DataStream::setStreamCompatibilityVersion(NetCompatibilityRules const& rules) {
  m_streamCompatibilityVersion = rules.version();
}

ByteArray DataStream::readBytes(std::size_t len) {
  ByteArray ba;
  ba.resize(len);
  readData(ba.ptr(), len);
  return ba;
}

void DataStream::writeBytes(ByteArray const& ba) {
  writeData(ba.ptr(), ba.size());
}

DataStream& DataStream::operator<<(bool d) {
  operator<<((std::uint8_t)d);
  return *this;
}

DataStream& DataStream::operator<<(char c) {
  writeData(&c, 1);
  return *this;
}

DataStream& DataStream::operator<<(std::int8_t d) {
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(std::uint8_t d) {
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(std::int16_t d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(std::uint16_t d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(std::int32_t d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(std::uint32_t d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(std::int64_t d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(std::uint64_t d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(float d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator<<(double d) {
  d = toByteOrder(m_byteOrder, d);
  writeData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator>>(bool& d) {
  std::uint8_t bu;
  readData((char*)&bu, sizeof(bu));
  // Converting an arbitrary byte to bool is UB when it is neither 0 nor 1, and
  // a stream can contain anything (a save file written by another version, for
  // example).  UBSan reports the resulting bool load.
  d = bu != 0;
  return *this;
}

DataStream& DataStream::operator>>(char& c) {
  readData(&c, 1);
  return *this;
}

DataStream& DataStream::operator>>(std::int8_t& d) {
  readData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator>>(std::uint8_t& d) {
  readData((char*)&d, sizeof(d));
  return *this;
}

DataStream& DataStream::operator>>(std::int16_t& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

DataStream& DataStream::operator>>(std::uint16_t& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

DataStream& DataStream::operator>>(std::int32_t& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

DataStream& DataStream::operator>>(std::uint32_t& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

DataStream& DataStream::operator>>(std::int64_t& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

DataStream& DataStream::operator>>(std::uint64_t& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

DataStream& DataStream::operator>>(float& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

DataStream& DataStream::operator>>(double& d) {
  readData((char*)&d, sizeof(d));
  d = fromByteOrder(m_byteOrder, d);
  return *this;
}

std::size_t DataStream::writeVlqU(std::uint64_t i) {
  return Star::writeVlqU(i, makeFunctionOutputIterator([this](std::uint8_t b) { *this << b; }));
}

std::size_t DataStream::writeVlqI(std::int64_t i) {
  return Star::writeVlqI(i, makeFunctionOutputIterator([this](std::uint8_t b) { *this << b; }));
}

std::size_t DataStream::writeVlqS(std::size_t i) {
  std::uint64_t i64;
  if (i == NPos)
    i64 = 0;
  else
    i64 = i + 1;
  return writeVlqU(i64);
}

std::size_t DataStream::readVlqU(std::uint64_t& i) {
  std::size_t bytesRead = Star::readVlqU(i, makeFunctionInputIterator([this]() { return this->read<std::uint8_t>(); }));

  if (bytesRead == NPos)
    throw DataStreamException("Error reading VLQ encoded integer!");

  return bytesRead;
}

std::size_t DataStream::readVlqI(std::int64_t& i) {
  std::size_t bytesRead = Star::readVlqI(i, makeFunctionInputIterator([this]() { return this->read<std::uint8_t>(); }));

  if (bytesRead == NPos)
    throw DataStreamException("Error reading VLQ encoded integer!");

  return bytesRead;
}

std::size_t DataStream::readVlqS(std::size_t& i) {
  std::uint64_t i64;
  std::size_t res = readVlqU(i64);
  if (i64 == 0)
    i = NPos;
  else
    i = (std::size_t)(i64 - 1);
  return res;
}

std::uint64_t DataStream::readVlqU() {
  std::uint64_t i;
  readVlqU(i);
  return i;
}

std::int64_t DataStream::readVlqI() {
  std::int64_t i;
  readVlqI(i);
  return i;
}

std::size_t DataStream::readVlqS() {
  std::size_t i;
  readVlqS(i);
  return i;
}

DataStream& DataStream::operator<<(char const* s) {
  writeStringData(s, std::strlen(s));
  return *this;
}

DataStream& DataStream::operator<<(std::string const& d) {
  writeStringData(d.c_str(), d.size());
  return *this;
}

DataStream& DataStream::operator<<(const ByteArray& d) {
  writeVlqU(d.size());
  writeData(d.ptr(), d.size());
  return *this;
}

DataStream& DataStream::operator<<(const String& s) {
  writeStringData(s.utf8Ptr(), s.utf8Size());
  return *this;
}

DataStream& DataStream::operator>>(std::string& d) {
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

DataStream& DataStream::operator>>(ByteArray& d) {
  d.resize((std::size_t)readVlqU());
  readData(d.ptr(), d.size());
  return *this;
}

DataStream& DataStream::operator>>(String& s) {
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

}
