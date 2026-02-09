#pragma once

#include "StarBytes.hpp"
#include "StarNetCompatibility.hpp"
#include "StarString.hpp"

import std;

namespace Star {

using DataStreamException = ExceptionDerived<"DataStreamException", IOException>;
extern unsigned const CurrentStreamVersion;

// Writes complex types to bytes in a portable big-endian fashion.
class DataStream {
public:
  DataStream();
  virtual ~DataStream() = default;

  // DataStream defaults to big-endian order for all primitive types
  [[nodiscard]] auto byteOrder() const -> ByteOrder;
  void setByteOrder(ByteOrder byteOrder);

  // DataStream can optionally write strings as null terminated rather than
  // length prefixed
  [[nodiscard]] auto nullTerminatedStrings() const -> bool;
  void setNullTerminatedStrings(bool nullTerminatedStrings);

  // streamCompatibilityVersion defaults to CurrentStreamVersion, but can be
  // changed for compatibility with older versions of DataStream serialization.
  [[nodiscard]] auto streamCompatibilityVersion() const -> unsigned;
  void setStreamCompatibilityVersion(unsigned streamCompatibilityVersion);
  void setStreamCompatibilityVersion(NetCompatibilityRules const& rules);
  // Do direct reads and writes
  virtual void readData(char* data, std::size_t len) = 0;
  virtual void writeData(char const* data, std::size_t len) = 0;
  virtual auto atEnd() -> bool { return false; };

  // These do not read / write sizes, they simply read / write directly.
  auto readBytes(std::size_t len) -> ByteArray;
  void writeBytes(ByteArray const& ba);

  auto operator<<(bool d) -> DataStream&;
  auto operator<<(char c) -> DataStream&;
  auto operator<<(std::int8_t d) -> DataStream&;
  auto operator<<(std::uint8_t d) -> DataStream&;
  auto operator<<(std::int16_t d) -> DataStream&;
  auto operator<<(std::uint16_t d) -> DataStream&;
  auto operator<<(std::int32_t d) -> DataStream&;
  auto operator<<(std::uint32_t d) -> DataStream&;
  auto operator<<(std::int64_t d) -> DataStream&;
  auto operator<<(std::uint64_t d) -> DataStream&;
  auto operator<<(float d) -> DataStream&;
  auto operator<<(double d) -> DataStream&;

  auto operator>>(bool& d) -> DataStream&;
  auto operator>>(char& c) -> DataStream&;
  auto operator>>(std::int8_t& d) -> DataStream&;
  auto operator>>(std::uint8_t& d) -> DataStream&;
  auto operator>>(std::int16_t& d) -> DataStream&;
  auto operator>>(std::uint16_t& d) -> DataStream&;
  auto operator>>(std::int32_t& d) -> DataStream&;
  auto operator>>(std::uint32_t& d) -> DataStream&;
  auto operator>>(std::int64_t& d) -> DataStream&;
  auto operator>>(std::uint64_t& d) -> DataStream&;
  auto operator>>(float& d) -> DataStream&;
  auto operator>>(double& d) -> DataStream&;

  // Writes and reads a VLQ encoded integer.  Can write / read anywhere from 1
  // to 10 bytes of data, with integers of smaller (absolute) value taking up
  // fewer bytes.  std::size_t version can be used to portably write a std::size_t type,
  // and portably and efficiently handles the case of std::numeric_limits<std::size_t>::max().

  auto writeVlqU(std::uint64_t i) -> std::size_t;
  auto writeVlqI(std::int64_t i) -> std::size_t;
  auto writeVlqS(std::size_t i) -> std::size_t;

  auto readVlqU(std::uint64_t& i) -> std::size_t;
  auto readVlqI(std::int64_t& i) -> std::size_t;
  auto readVlqS(std::size_t& i) -> std::size_t;

  auto readVlqU() -> std::uint64_t;
  auto readVlqI() -> std::int64_t;
  auto readVlqS() -> std::size_t;

  // The following functions write / read data with length and then content
  // following, but note that the length is encoded as an unsigned VLQ integer.
  // String objects are encoded in utf8, and can optionally be written as null
  // terminated rather than length then content.

  auto operator<<(const char* s) -> DataStream&;
  auto operator<<(std::string const& d) -> DataStream&;
  auto operator<<(ByteArray const& d) -> DataStream&;
  auto operator<<(String const& s) -> DataStream&;

  auto operator>>(std::string& d) -> DataStream&;
  auto operator>>(ByteArray& d) -> DataStream&;
  auto operator>>(String& s) -> DataStream&;

  // All enum types are automatically serializable

  template <typename EnumType, typename = std::enable_if_t<std::is_enum_v<EnumType>>>
  auto operator<<(EnumType const& e) -> DataStream&;

  template <typename EnumType, typename = std::enable_if_t<std::is_enum_v<EnumType>>>
  auto operator>>(EnumType& e) -> DataStream&;

  // Convenience method to avoid temporary.
  template <typename T>
  auto read() -> T;

  // Convenient argument style reading / writing

  template <typename Data>
  void read(Data& data);

  template <typename Data>
  void write(Data const& data);

  // Argument style reading / writing with casting.

  template <typename ReadType, typename Data>
  void cread(Data& data);

  template <typename WriteType, typename Data>
  void cwrite(Data const& data);

  // Argument style reading / writing of variable length integers.  Arguments
  // are explicitly casted, so things like enums are allowed.

  template <typename IntegralType>
  void vuread(IntegralType& data);

  template <typename IntegralType>
  void viread(IntegralType& data);

  template <typename IntegralType>
  void vsread(IntegralType& data);

  template <typename IntegralType>
  void vuwrite(IntegralType const& data);

  template <typename IntegralType>
  void viwrite(IntegralType const& data);

  template <typename IntegralType>
  void vswrite(IntegralType const& data);

  // Store a fixed point number as a variable length integer

  template <typename FloatType>
  void vfread(FloatType& data, FloatType base);

  template <typename FloatType>
  void vfwrite(FloatType const& data, FloatType base);

  // Read a shared / unique ptr, and store whether the pointer is initialized.

  template <typename PointerType, typename ReadFunction>
  void pread(PointerType& pointer, ReadFunction readFunction);

  template <typename PointerType, typename WriteFunction>
  void pwrite(PointerType const& pointer, WriteFunction writeFunction);

  template <typename PointerType>
  void pread(PointerType& pointer);

  template <typename PointerType>
  void pwrite(PointerType const& pointer);

  // WriteFunction should be void (DataStream& ds, Element const& e)
  template <typename Container, typename WriteFunction>
  void writeContainer(Container const& container, WriteFunction function);

  // ReadFunction should be void (DataStream& ds, Element& e)
  template <typename Container, typename ReadFunction>
  void readContainer(Container& container, ReadFunction function);

  template <typename Container, typename WriteFunction>
  void writeMapContainer(Container& map, WriteFunction function);

  // Specialization of readContainer for map types (whose elements are a pair
  // with the key type marked const)
  template <typename Container, typename ReadFunction>
  void readMapContainer(Container& map, ReadFunction function);

  template <typename Container>
  void writeContainer(Container const& container);

  template <typename Container>
  void readContainer(Container& container);

  template <typename Container>
  void writeMapContainer(Container const& container);

  template <typename Container>
  void readMapContainer(Container& container);

private:
  void writeStringData(char const* data, std::size_t len);

  ByteOrder m_byteOrder;
  bool m_nullTerminatedStrings;
  unsigned m_streamCompatibilityVersion;
};

template <typename EnumType, typename>
auto DataStream::operator<<(EnumType const& e) -> DataStream& {
  *this << (std::underlying_type_t<EnumType>)e;
  return *this;
}

template <typename EnumType, typename>
auto DataStream::operator>>(EnumType& e) -> DataStream& {
  std::underlying_type_t<EnumType> i;
  *this >> i;
  e = (EnumType)i;
  return *this;
}

template <typename T>
auto DataStream::read() -> T {
  T t;
  *this >> t;
  return t;
}

template <typename Data>
void DataStream::read(Data& data) {
  *this >> data;
}

template <typename Data>
void DataStream::write(Data const& data) {
  *this << data;
}

template <typename ReadType, typename Data>
void DataStream::cread(Data& data) {
  ReadType v;
  *this >> v;
  data = (Data)v;
}

template <typename WriteType, typename Data>
void DataStream::cwrite(Data const& data) {
  auto v = (WriteType)data;
  *this << v;
}

template <typename IntegralType>
void DataStream::vuread(IntegralType& data) {
  std::uint64_t i = readVlqU();
  data = (IntegralType)i;
}

template <typename IntegralType>
void DataStream::viread(IntegralType& data) {
  std::int64_t i = readVlqI();
  data = (IntegralType)i;
}

template <typename IntegralType>
void DataStream::vsread(IntegralType& data) {
  std::size_t s = readVlqS();
  data = (IntegralType)s;
}

template <typename IntegralType>
void DataStream::vuwrite(IntegralType const& data) {
  writeVlqU((std::uint64_t)data);
}

template <typename IntegralType>
void DataStream::viwrite(IntegralType const& data) {
  writeVlqI((std::int64_t)data);
}

template <typename IntegralType>
void DataStream::vswrite(IntegralType const& data) {
  writeVlqS((std::size_t)data);
}

template <typename FloatType>
void DataStream::vfread(FloatType& data, FloatType base) {
  std::int64_t i = readVlqI();
  data = (FloatType)i * base;
}

template <typename FloatType>
void DataStream::vfwrite(FloatType const& data, FloatType base) {
  writeVlqI((std::int64_t)std::round(data / base));
}

template <typename PointerType, typename ReadFunction>
void DataStream::pread(PointerType& pointer, ReadFunction readFunction) {
  bool initialized = read<bool>();
  if (initialized) {
    auto element = std::make_unique<std::decay_t<typename PointerType::element_type>>();
    readFunction(*this, *element);
    pointer.reset(element.release());
  } else {
    pointer.reset();
  }
}

template <typename PointerType, typename WriteFunction>
void DataStream::pwrite(PointerType const& pointer, WriteFunction writeFunction) {
  if (pointer) {
    write(true);
    writeFunction(*this, *pointer);
  } else {
    write(false);
  }
}

template <typename PointerType>
void DataStream::pread(PointerType& pointer) {
  return pread(pointer, [](DataStream& ds, std::decay_t<typename PointerType::element_type>& value) -> auto {
    ds.read(value);
  });
}

template <typename PointerType>
void DataStream::pwrite(PointerType const& pointer) {
  return pwrite(pointer, [](DataStream& ds, typename std::decay<typename PointerType::element_type>::type const& value) -> auto {
    ds.write(value);
  });
}

template <typename Container, typename WriteFunction>
void DataStream::writeContainer(Container const& container, WriteFunction function) {
  writeVlqU(container.size());
  for (auto const& elem : container)
    function(*this, elem);
}

template <typename Container, typename ReadFunction>
void DataStream::readContainer(Container& container, ReadFunction function) {
  container.clear();
  std::size_t size = readVlqU();
  for (std::size_t i = 0; i < size; ++i) {
    typename Container::value_type elem;
    function(*this, elem);
    container.insert(container.end(), elem);
  }
}

template <typename Container, typename WriteFunction>
void DataStream::writeMapContainer(Container& map, WriteFunction function) {
  writeVlqU(map.size());
  for (auto const& elem : map)
    function(*this, elem.first, elem.second);
}

template <typename Container, typename ReadFunction>
void DataStream::readMapContainer(Container& map, ReadFunction function) {
  map.clear();
  std::size_t size = readVlqU();
  for (std::size_t i = 0; i < size; ++i) {
    typename Container::key_type key;
    typename Container::mapped_type mapped;
    function(*this, key, mapped);
    map.insert(std::make_pair(std::move(key), std::move(mapped)));
  }
}

template <typename Container>
void DataStream::writeContainer(Container const& container) {
  writeContainer(container, [](DataStream& ds, typename Container::value_type const& element) -> auto { ds << element; });
}

template <typename Container>
void DataStream::readContainer(Container& container) {
  readContainer(container, [](DataStream& ds, typename Container::value_type& element) -> auto { ds >> element; });
}

template <typename Container>
void DataStream::writeMapContainer(Container const& container) {
  writeMapContainer(container, [](DataStream& ds, typename Container::key_type const& key, typename Container::mapped_type const& mapped) -> auto {
    ds << key;
    ds << mapped;
  });
}

template <typename Container>
void DataStream::readMapContainer(Container& container) {
  readMapContainer(container, [](DataStream& ds, typename Container::key_type& key, typename Container::mapped_type& mapped) -> auto {
    ds >> key;
    ds >> mapped;
  });
}

}// namespace Star
