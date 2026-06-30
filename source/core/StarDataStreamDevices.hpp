#pragma once

#include "StarBuffer.hpp"
#include "StarDataStream.hpp"

namespace Star {

// Implements DataStream using function objects as implementations of read/write.
class DataStreamFunctions : public DataStream {
public:
  // Either reader or writer can be unset, if unset then the readData/writeData
  // implementations will throw DataStreamException as unimplemented.
  DataStreamFunctions(function<size_t(char*, size_t)> reader, function<size_t(char const*, size_t)> writer);

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  function<size_t(char*, size_t)> m_reader;
  function<size_t(char const*, size_t)> m_writer;
};

class DataStreamIODevice : public DataStream {
public:
  DataStreamIODevice(IODevicePtr device);

  [[nodiscard]] IODevicePtr const& device() const;

  void seek(size_t pos, IOSeek seek = IOSeek::Absolute);
  [[nodiscard]] bool atEnd() override;
  [[nodiscard]] StreamOffset pos();

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  IODevicePtr m_device;
};

class DataStreamBuffer : public DataStream {
public:
  // Convenience methods to serialize to / from ByteArray directly without
  // having to construct a temporary DataStreamBuffer to do it

  template <typename T>
  [[nodiscard]] static ByteArray serialize(T const& t);

  template <typename T>
  [[nodiscard]] static ByteArray serializeContainer(T const& t);

  template <typename T, typename WriteFunction>
  [[nodiscard]] static ByteArray serializeContainer(T const& t, WriteFunction writeFunction);

  template <typename T>
  [[nodiscard]] static ByteArray serializeMapContainer(T const& t);

  template <typename T, typename WriteFunction>
  [[nodiscard]] static ByteArray serializeMapContainer(T const& t, WriteFunction writeFunction);

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
  [[nodiscard]] static T deserialize(ByteArray data);

  template <typename T>
  [[nodiscard]] static T deserializeContainer(ByteArray data);

  template <typename T, typename ReadFunction>
  [[nodiscard]] static T deserializeContainer(ByteArray data, ReadFunction readFunction);

  template <typename T>
  [[nodiscard]] static T deserializeMapContainer(ByteArray data);

  template <typename T, typename ReadFunction>
  [[nodiscard]] static T deserializeMapContainer(ByteArray data, ReadFunction readFunction);

  DataStreamBuffer();
  DataStreamBuffer(size_t initialSize);
  DataStreamBuffer(ByteArray b);

  // Resize existing buffer to new size.
  void resize(size_t size);
  void reserve(size_t size);
  void clear();

  [[nodiscard]] ByteArray& data();
  [[nodiscard]] ByteArray const& data() const;
  [[nodiscard]] ByteArray takeData();

  [[nodiscard]] char* ptr();
  [[nodiscard]] char const* ptr() const;

  [[nodiscard]] BufferPtr const& device() const;

  [[nodiscard]] size_t size() const;
  [[nodiscard]] bool empty() const;

  void seek(size_t pos, IOSeek seek = IOSeek::Absolute);
  [[nodiscard]] bool atEnd() override;
  [[nodiscard]] size_t pos();

  // Set new buffer.
  void reset(size_t newSize);
  void reset(ByteArray b);

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  BufferPtr m_buffer;
};

class DataStreamExternalBuffer : public DataStream {
public:
  DataStreamExternalBuffer() = default;
  DataStreamExternalBuffer(ByteArray const& byteArray);
  DataStreamExternalBuffer(DataStreamBuffer const& buffer);

  DataStreamExternalBuffer(DataStreamExternalBuffer const& buffer) noexcept = default;
  DataStreamExternalBuffer(char const* externalData, size_t len);

  [[nodiscard]] char const* ptr() const;

  [[nodiscard]] size_t size() const;
  [[nodiscard]] bool empty() const;

  void seek(size_t pos, IOSeek mode = IOSeek::Absolute);
  [[nodiscard]] bool atEnd() override;
  [[nodiscard]] size_t pos();
  [[nodiscard]] size_t remaining();

  void reset(char const* externalData, size_t len);

  void readData(char* data, size_t len) override;
  void writeData(char const* data, size_t len) override;

private:
  ExternalBuffer m_buffer;
};

template <typename T>
[[nodiscard]] ByteArray DataStreamBuffer::serialize(T const& t) {
  DataStreamBuffer ds;
  ds.write(t);
  return ds.takeData();
}

template <typename T>
[[nodiscard]] ByteArray DataStreamBuffer::serializeContainer(T const& t) {
  DataStreamBuffer ds;
  ds.writeContainer(t);
  return ds.takeData();
}

template <typename T, typename WriteFunction>
[[nodiscard]] ByteArray DataStreamBuffer::serializeContainer(T const& t, WriteFunction writeFunction) {
  DataStreamBuffer ds;
  ds.writeContainer(t, writeFunction);
  return ds.takeData();
}

template <typename T>
[[nodiscard]] ByteArray DataStreamBuffer::serializeMapContainer(T const& t) {
  DataStreamBuffer ds;
  ds.writeMapContainer(t);
  return ds.takeData();
}

template <typename T, typename WriteFunction>
[[nodiscard]] ByteArray DataStreamBuffer::serializeMapContainer(T const& t, WriteFunction writeFunction) {
  DataStreamBuffer ds;
  ds.writeMapContainer(t, writeFunction);
  return ds.takeData();
}

template <typename T>
void DataStreamBuffer::deserialize(T& t, ByteArray data) {
  [[nodiscard]] DataStreamBuffer ds(std::move(data));
  ds.read(t);
}

template <typename T>
void DataStreamBuffer::deserializeContainer(T& t, ByteArray data) {
  [[nodiscard]] DataStreamBuffer ds(std::move(data));
  ds.readContainer(t);
}

template <typename T, typename ReadFunction>
void DataStreamBuffer::deserializeContainer(T& t, ByteArray data, ReadFunction readFunction) {
  [[nodiscard]] DataStreamBuffer ds(std::move(data));
  ds.readContainer(t, readFunction);
}

template <typename T>
void DataStreamBuffer::deserializeMapContainer(T& t, ByteArray data) {
  [[nodiscard]] DataStreamBuffer ds(std::move(data));
  ds.readMapContainer(t);
}

template <typename T, typename ReadFunction>
void DataStreamBuffer::deserializeMapContainer(T& t, ByteArray data, ReadFunction readFunction) {
  [[nodiscard]] DataStreamBuffer ds(std::move(data));
  ds.readMapContainer(t, readFunction);
}

template <typename T>
[[nodiscard]] T DataStreamBuffer::deserialize(ByteArray data) {
  T t;
  deserialize(t, std::move(data));
  return t;
}

template <typename T>
[[nodiscard]] T DataStreamBuffer::deserializeContainer(ByteArray data) {
  T t;
  deserializeContainer(t, std::move(data));
  return t;
}

template <typename T, typename ReadFunction>
[[nodiscard]] T DataStreamBuffer::deserializeContainer(ByteArray data, ReadFunction readFunction) {
  T t;
  deserializeContainer(t, std::move(data), readFunction);
  return t;
}

template <typename T>
[[nodiscard]] T DataStreamBuffer::deserializeMapContainer(ByteArray data) {
  T t;
  deserializeMapContainer(t, std::move(data));
  return t;
}

template <typename T, typename ReadFunction>
[[nodiscard]] T DataStreamBuffer::deserializeMapContainer(ByteArray data, ReadFunction readFunction) {
  T t;
  deserializeMapContainer(t, std::move(data), readFunction);
  return t;
}

}
