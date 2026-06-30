#pragma once
#include "StarByteArray.hpp"
#include "StarDataStreamDevices.hpp"

#include <memory>

struct ZSTD_CCtx_s;
struct ZSTD_DCtx_s;
using ZSTD_CCtx = ZSTD_CCtx_s;
using ZSTD_DCtx = ZSTD_DCtx_s;
using ZSTD_DStream = ZSTD_DCtx;
using ZSTD_CStream = ZSTD_CCtx;

namespace Star {

struct ZstdCompressionStreamDeleter {
  void operator()(ZSTD_CStream* stream) const noexcept;
};

struct ZstdDecompressionStreamDeleter {
  void operator()(ZSTD_DStream* stream) const noexcept;
};

class CompressionStream {
public:
  CompressionStream();
  ~CompressionStream();

  CompressionStream(CompressionStream const&) = delete;
  CompressionStream& operator=(CompressionStream const&) = delete;
  CompressionStream(CompressionStream&&) = delete;
  CompressionStream& operator=(CompressionStream&&) = delete;

  void compress(const char* in, size_t inLen, ByteArray& out);
  void compress(ByteArray const& in, ByteArray& out);
  [[nodiscard]] ByteArray compress(const char* in, size_t inLen);
  [[nodiscard]] ByteArray compress(ByteArray const& in);

private:
  std::unique_ptr<ZSTD_CStream, ZstdCompressionStreamDeleter> m_cStream;
};

class DecompressionStream {
public:
  DecompressionStream();
  ~DecompressionStream();

  DecompressionStream(DecompressionStream const&) = delete;
  DecompressionStream& operator=(DecompressionStream const&) = delete;
  DecompressionStream(DecompressionStream&&) = delete;
  DecompressionStream& operator=(DecompressionStream&&) = delete;

  void decompress(const char* in, size_t inLen, ByteArray& out);
  void decompress(ByteArray const& in, ByteArray& out);
  [[nodiscard]] ByteArray decompress(const char* in, size_t inLen);
  [[nodiscard]] ByteArray decompress(ByteArray const& in);

private:
  std::unique_ptr<ZSTD_DStream, ZstdDecompressionStreamDeleter> m_dStream;
};

}// namespace Star
