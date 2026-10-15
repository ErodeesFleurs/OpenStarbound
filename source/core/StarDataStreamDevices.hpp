#pragma once
import std;
import star.io_device;
import star.string;
import star.buffer;
import star.data_stream;

import star.either;
import star.byte_array;
namespace Star {

// Implements DataStream using function objects as implementations of read/write.
class DataStreamFunctions : public DataStream {
public:
  // Either reader or writer can be unset, if unset then the readData/writeData
  // implementations will throw DataStreamException as unimplemented.
  DataStreamFunctions(std::function<std::size_t(char*, std::size_t)> reader, std::function<std::size_t(char const*, std::size_t)> writer);

  void readData(char* data, std::size_t len) override;
  void writeData(char const* data, std::size_t len) override;

private:
  std::function<std::size_t(char*, std::size_t)> m_reader;
  std::function<std::size_t(char const*, std::size_t)> m_writer;
};

class DataStreamIODevice : public DataStream {
public:
  DataStreamIODevice(std::shared_ptr<IODevice> device);

  std::shared_ptr<IODevice> const& device() const;

  void seek(std::size_t pos, IOSeek seek = IOSeek::Absolute);
  bool atEnd() override;
  StreamOffset pos();

  void readData(char* data, std::size_t len) override;
  void writeData(char const* data, std::size_t len) override;

private:
  std::shared_ptr<IODevice> m_device;
};

class DataStreamBuffer : public DataStream {
public:
  // Convenience methods to serialize to / from ByteArray directly without
  // having to construct a temporary DataStreamBuffer to do it

  template <typename T>
  static ByteArray serialize(T const& t);

  template <typename T>
  static ByteArray serializeContainer(T const& t);

  template <typename T, typename WriteFunction>
  static ByteArray serializeContainer(T const& t, WriteFunction writeFunction);

  template <typename T>
  static ByteArray serializeMapContainer(T const& t);

  template <typename T, typename WriteFunction>
  static ByteArray serializeMapContainer(T const& t, WriteFunction writeFunction);

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
  static T deserialize(ByteArray data);

  template <typename T>
  static T deserializeContainer(ByteArray data);

  template <typename T, typename ReadFunction>
  static T deserializeContainer(ByteArray data, ReadFunction readFunction);

  template <typename T>
  static T deserializeMapContainer(ByteArray data);

  template <typename T, typename ReadFunction>
  static T deserializeMapContainer(ByteArray data, ReadFunction readFunction);

  DataStreamBuffer();
  DataStreamBuffer(std::size_t initialSize);
  DataStreamBuffer(ByteArray b);

  // Resize existing buffer to new size.
  void resize(std::size_t size);
  void reserve(std::size_t size);
  void clear();

  ByteArray& data();
  ByteArray const& data() const;
  ByteArray takeData();

  char* ptr();
  char const* ptr() const;

  std::shared_ptr<Buffer> const& device() const;

  std::size_t size() const;
  bool empty() const;

  void seek(std::size_t pos, IOSeek seek = IOSeek::Absolute);
  bool atEnd() override;
  std::size_t pos();

  // Set new buffer.
  void reset(std::size_t newSize);
  void reset(ByteArray b);

  void readData(char* data, std::size_t len) override;
  void writeData(char const* data, std::size_t len) override;

private:
  std::shared_ptr<Buffer> m_buffer;
};

class DataStreamExternalBuffer : public DataStream {
public:
  DataStreamExternalBuffer();
  DataStreamExternalBuffer(ByteArray const& byteArray);
  DataStreamExternalBuffer(DataStreamBuffer const& buffer);

  DataStreamExternalBuffer(DataStreamExternalBuffer const& buffer) = default;
  DataStreamExternalBuffer(char const* externalData, std::size_t len);

  char const* ptr() const;

  std::size_t size() const;
  bool empty() const;

  void seek(std::size_t pos, IOSeek mode = IOSeek::Absolute);
  bool atEnd() override;
  std::size_t pos();
  std::size_t remaining();

  void reset(char const* externalData, std::size_t len);

  void readData(char* data, std::size_t len) override;
  void writeData(char const* data, std::size_t len) override;

private:
  ExternalBuffer m_buffer;
};

template <typename T>
ByteArray DataStreamBuffer::serialize(T const& t) {
  DataStreamBuffer ds;
  ds.write(t);
  return ds.takeData();
}

template <typename T>
ByteArray DataStreamBuffer::serializeContainer(T const& t) {
  DataStreamBuffer ds;
  ds.writeContainer(t);
  return ds.takeData();
}

template <typename T, typename WriteFunction>
ByteArray DataStreamBuffer::serializeContainer(T const& t, WriteFunction writeFunction) {
  DataStreamBuffer ds;
  ds.writeContainer(t, writeFunction);
  return ds.takeData();
}

template <typename T>
ByteArray DataStreamBuffer::serializeMapContainer(T const& t) {
  DataStreamBuffer ds;
  ds.writeMapContainer(t);
  return ds.takeData();
}

template <typename T, typename WriteFunction>
ByteArray DataStreamBuffer::serializeMapContainer(T const& t, WriteFunction writeFunction) {
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
T DataStreamBuffer::deserialize(ByteArray data) {
  T t;
  deserialize(t, std::move(data));
  return t;
}

template <typename T>
T DataStreamBuffer::deserializeContainer(ByteArray data) {
  T t;
  deserializeContainer(t, std::move(data));
  return t;
}

template <typename T, typename ReadFunction>
T DataStreamBuffer::deserializeContainer(ByteArray data, ReadFunction readFunction) {
  T t;
  deserializeContainer(t, std::move(data), readFunction);
  return t;
}

template <typename T>
T DataStreamBuffer::deserializeMapContainer(ByteArray data) {
  T t;
  deserializeMapContainer(t, std::move(data));
  return t;
}

template <typename T, typename ReadFunction>
T DataStreamBuffer::deserializeMapContainer(ByteArray data, ReadFunction readFunction) {
  T t;
  deserializeMapContainer(t, std::move(data), readFunction);
  return t;
}

}
