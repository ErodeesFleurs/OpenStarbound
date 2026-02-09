#pragma once

#include "StarConfig.hpp"
#include "StarIODevice.hpp"
#include "StarString.hpp"

import std;

namespace Star {

// Zlib compression level, ranges from 0 to 9
using CompressionLevel = std::int32_t;

CompressionLevel const LowCompression = 2;
CompressionLevel const MediumCompression = 5;
CompressionLevel const HighCompression = 9;

void compressData(ByteArray const& in, ByteArray& out, CompressionLevel compression = MediumCompression);
auto compressData(ByteArray const& in, CompressionLevel compression = MediumCompression) -> ByteArray;

void uncompressData(const char* in, std::size_t inLen, ByteArray& out, std::size_t limit = 0);
auto uncompressData(const char* in, std::size_t inLen, std::size_t limit = 0) -> ByteArray;
void uncompressData(ByteArray const& in, ByteArray& out, std::size_t limit = 0);
auto uncompressData(ByteArray const& in, std::size_t limit = 0) -> ByteArray;

// Random access to a (potentially) compressed file.
class CompressedFile : public IODevice {
public:
  static auto open(String const& filename, IOMode mode, CompressionLevel comp = MediumCompression) -> Ptr<CompressedFile>;

  CompressedFile();
  CompressedFile(String filename);
  ~CompressedFile() override;

  void setFilename(String filename);
  void setCompression(CompressionLevel compression);

  auto pos() -> std::int64_t override;
  // Only seek forward is supported on writes.  Seek is emulated *slowly* on
  // reads.
  void seek(std::int64_t pos, IOSeek seek = IOSeek::Absolute) override;
  auto atEnd() -> bool override;
  auto read(char* data, std::size_t len) -> std::size_t override;
  auto write(char const* data, std::size_t len) -> std::size_t override;

  void open(IOMode mode) override;
  // Compression is ignored on read.  Always truncates on write
  void open(IOMode mode, CompressionLevel compression);

  void sync() override;
  void close() override;

  auto clone() -> Ptr<IODevice> override;

private:
  String m_filename;
  void* m_file;
  CompressionLevel m_compression;
};

}// namespace Star
