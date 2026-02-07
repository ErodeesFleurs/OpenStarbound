#pragma once
#include "StarByteArray.hpp"

using ZSTD_CCtx = struct ZSTD_CCtx_s;
using ZSTD_DCtx = struct ZSTD_DCtx_s;
using ZSTD_DStream = ZSTD_DCtx;
using ZSTD_CStream = ZSTD_CCtx;

import std;

namespace Star {

class CompressionStream {
public:
  CompressionStream();
  ~CompressionStream();

  void compress(const char* in, std::size_t inLen, ByteArray& out);
  void compress(ByteArray const& in, ByteArray& out);
  auto compress(const char* in, std::size_t inLen) -> ByteArray;
  auto compress(ByteArray const& in) -> ByteArray;

private:
  ZSTD_CStream* m_cStream;
};

class DecompressionStream {
public:
  DecompressionStream();
  ~DecompressionStream();

  void decompress(const char* in, std::size_t inLen, ByteArray& out);
  void decompress(ByteArray const& in, ByteArray& out);
  auto decompress(const char* in, std::size_t inLen) -> ByteArray;
  auto decompress(ByteArray const& in) -> ByteArray;

private:
  ZSTD_DStream* m_dStream;
};

}// namespace Star
