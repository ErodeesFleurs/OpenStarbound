module;

import std;
import star.byte_array;
import star.data_stream_devices;

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;

namespace Star {

class CompressionStream {
public:
  CompressionStream();
  ~CompressionStream();
  CompressionStream(CompressionStream const&) = delete;
  CompressionStream& operator=(CompressionStream const&) = delete;
  CompressionStream(CompressionStream&&) = delete;
  CompressionStream& operator=(CompressionStream&&) = delete;

  void compress(const char* in, std::size_t inLen, ByteArray& out);
  void compress(ByteArray const& in, ByteArray& out);
  ByteArray compress(const char* in, std::size_t inLen);
  ByteArray compress(ByteArray const& in);

private:
  struct StreamDeleter {
    void operator()(ZSTD_CStream* stream) const noexcept;
  };
  std::unique_ptr<ZSTD_CStream, StreamDeleter> m_cStream;
};

class DecompressionStream {
public:
  DecompressionStream();
  ~DecompressionStream();
  DecompressionStream(DecompressionStream const&) = delete;
  DecompressionStream& operator=(DecompressionStream const&) = delete;
  DecompressionStream(DecompressionStream&&) = delete;
  DecompressionStream& operator=(DecompressionStream&&) = delete;

  void decompress(const char* in, std::size_t inLen, ByteArray& out);
  void decompress(ByteArray const& in, ByteArray& out);
  ByteArray decompress(const char* in, std::size_t inLen);
  ByteArray decompress(ByteArray const& in);

private:
  struct StreamDeleter {
    void operator()(ZSTD_DStream* stream) const noexcept;
  };
  std::unique_ptr<ZSTD_DStream, StreamDeleter> m_dStream;
};

class ZstdCompression {
public:
  static void compress(const char* in, std::size_t inLen, ByteArray& out, int compressionLevel = 0);
  static void compress(ByteArray const& in, ByteArray& out, int compressionLevel = 0);
  static ByteArray compress(const char* in, std::size_t inLen, int compressionLevel = 0);
  static ByteArray compress(ByteArray const& in, int compressionLevel = 0);

  static void decompress(const char* in, std::size_t inLen, ByteArray& out);
  static void decompress(ByteArray const& in, ByteArray& out);
  static ByteArray decompress(const char* in, std::size_t inLen);
  static ByteArray decompress(ByteArray const& in);
};

}

export module star.zstd_compression;

export namespace Star {
  using ::Star::CompressionStream;
  using ::Star::DecompressionStream;
  using ::Star::ZstdCompression;
}
