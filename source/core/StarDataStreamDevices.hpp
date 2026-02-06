#pragma once

#include "StarBuffer.hpp"
#include "StarDataStream.hpp"

import std;

namespace Star {

// Implements DataStream using function objects as implementations of read/write.
class DataStreamFunctions : public DataStream {
public:
  // Either reader or writer can be unset, if unset then the readData/writeData
  // implementations will throw DataStreamException as unimplemented.
  DataStreamFunctions(std::function<std::size_t(char*, std::size_t)> reader, std::function<std::size_t(char const*, std::size_t)> writer);

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  std::function<size_t(char*, size_t)> m_reader;
  std::function<size_t(char const*, size_t)> m_writer;
};

class DataStreamIODevice : public DataStream {
public:
  DataStreamIODevice(Ptr<IODevice> device);

  [[nodiscard]] auto device() const -> Ptr<IODevice> const&;

  void seek(size_t pos, IOSeek seek = IOSeek::Absolute);
  auto atEnd() -> bool override;
  auto pos() -> std::int64_t;

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  Ptr<IODevice> m_device;
};

class DataStreamBuffer : public DataStream {
public:
  // Convenience methods to serialize to / from ByteArray directly without
  // having to construct a temporary DataStreamBuffer to do it

  template <typename T>
  static auto serialize(T const& t) -> ByteArray;

  template <typename T>
  static auto serializeContainer(T const& t) -> ByteArray;

  template <typename T, typename WriteFunction>
  static auto serializeContainer(T const& t, WriteFunction writeFunction) -> ByteArray;

  template <typename T>
  static auto serializeMapContainer(T const& t) -> ByteArray;

  template <typename T, typename WriteFunction>
  static auto serializeMapContainer(T const& t, WriteFunction writeFunction) -> ByteArray;

  template <typename T>
  static void deserialize(T& t, ByteArray data);

  template <typename T>
  static void deserializeContainer(T& t, ByteArray data);

  template <typename T, typename ReadFunction>
  static void deserializeContainer(T& t, ByteArray data, ReadFunction readFunction);

  template <typename T>
  static void deserializeMapContainer(T& t, ByteArray data);

  template <typename T, typename ReadFunction>
  static void deserializeMapContainer(T& t, ByteArray data, ReadFunction readFunction);

  template <typename T>
  static auto deserialize(ByteArray data) -> T;

  template <typename T>
  static auto deserializeContainer(ByteArray data) -> T;

  template <typename T, typename ReadFunction>
  static auto deserializeContainer(ByteArray data, ReadFunction readFunction) -> T;

  template <typename T>
  static auto deserializeMapContainer(ByteArray data) -> T;

  template <typename T, typename ReadFunction>
  static auto deserializeMapContainer(ByteArray data, ReadFunction readFunction) -> T;

  DataStreamBuffer();
  DataStreamBuffer(size_t initialSize);
  DataStreamBuffer(ByteArray b);

  // Resize existing buffer to new size.
  void resize(size_t size);
  void reserve(size_t size);
  void clear();

  auto data() -> ByteArray&;
  [[nodiscard]] auto data() const -> ByteArray const&;
  auto takeData() -> ByteArray;

  auto ptr() -> char*;
  [[nodiscard]] auto ptr() const -> char const*;

  [[nodiscard]] auto device() const -> Ptr<Buffer> const&;

  [[nodiscard]] auto size() const -> size_t;
  [[nodiscard]] auto empty() const -> bool;

  void seek(size_t pos, IOSeek seek = IOSeek::Absolute);
  auto atEnd() -> bool override;
  auto pos() -> size_t;

  // Set new buffer.
  void reset(size_t newSize);
  void reset(ByteArray b);

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  Ptr<Buffer> m_buffer;
};

class DataStreamExternalBuffer : public DataStream {
public:
  DataStreamExternalBuffer();
  DataStreamExternalBuffer(ByteArray const& byteArray);
  DataStreamExternalBuffer(DataStreamBuffer const& buffer);

  DataStreamExternalBuffer(DataStreamExternalBuffer const& buffer) = default;
  DataStreamExternalBuffer(char const* externalData, size_t len);

  auto ptr() const -> char const*;

  auto size() const -> size_t;
  auto empty() const -> bool;

  void seek(size_t pos, IOSeek mode = IOSeek::Absolute);
  auto atEnd() -> bool override;
  auto pos() -> size_t;
  auto remaining() -> size_t;

  void reset(char const* externalData, size_t len);

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  ExternalBuffer m_buffer;
};

template <typename T>
auto DataStreamBuffer::serialize(T const& t) -> ByteArray {
  DataStreamBuffer ds;
  ds.write(t);
  return ds.takeData();
}

template <typename T>
auto DataStreamBuffer::serializeContainer(T const& t) -> ByteArray {
  DataStreamBuffer ds;
  ds.writeContainer(t);
  return ds.takeData();
}

template <typename T, typename WriteFunction>
auto DataStreamBuffer::serializeContainer(T const& t, WriteFunction writeFunction) -> ByteArray {
  DataStreamBuffer ds;
  ds.writeContainer(t, writeFunction);
  return ds.takeData();
}

template <typename T>
auto DataStreamBuffer::serializeMapContainer(T const& t) -> ByteArray {
  DataStreamBuffer ds;
  ds.writeMapContainer(t);
  return ds.takeData();
}

template <typename T, typename WriteFunction>
auto DataStreamBuffer::serializeMapContainer(T const& t, WriteFunction writeFunction) -> ByteArray {
  DataStreamBuffer ds;
  ds.writeMapContainer(t, writeFunction);
  return ds.takeData();
}

template <typename T>
void DataStreamBuffer::deserialize(T& t, ByteArray data) {
  DataStreamBuffer ds(std::move(data));
  ds.read(t);
}

template <typename T>
void DataStreamBuffer::deserializeContainer(T& t, ByteArray data) {
  DataStreamBuffer ds(std::move(data));
  ds.readContainer(t);
}

template <typename T, typename ReadFunction>
void DataStreamBuffer::deserializeContainer(T& t, ByteArray data, ReadFunction readFunction) {
  DataStreamBuffer ds(std::move(data));
  ds.readContainer(t, readFunction);
}

template <typename T>
void DataStreamBuffer::deserializeMapContainer(T& t, ByteArray data) {
  DataStreamBuffer ds(std::move(data));
  ds.readMapContainer(t);
}

template <typename T, typename ReadFunction>
void DataStreamBuffer::deserializeMapContainer(T& t, ByteArray data, ReadFunction readFunction) {
  DataStreamBuffer ds(std::move(data));
  ds.readMapContainer(t, readFunction);
}

template <typename T>
auto DataStreamBuffer::deserialize(ByteArray data) -> T {
  T t;
  deserialize(t, std::move(data));
  return t;
}

template <typename T>
auto DataStreamBuffer::deserializeContainer(ByteArray data) -> T {
  T t;
  deserializeContainer(t, std::move(data));
  return t;
}

template <typename T, typename ReadFunction>
auto DataStreamBuffer::deserializeContainer(ByteArray data, ReadFunction readFunction) -> T {
  T t;
  deserializeContainer(t, std::move(data), readFunction);
  return t;
}

template <typename T>
auto DataStreamBuffer::deserializeMapContainer(ByteArray data) -> T {
  T t;
  deserializeMapContainer(t, std::move(data));
  return t;
}

template <typename T, typename ReadFunction>
auto DataStreamBuffer::deserializeMapContainer(ByteArray data, ReadFunction readFunction) -> T {
  T t;
  deserializeMapContainer(t, std::move(data), readFunction);
  return t;
}

}
