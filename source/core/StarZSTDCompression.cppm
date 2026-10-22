module;

import std;
import star.byte_array;
import star.format;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;

namespace Star {

class CompressionStream {
public:
  [[nodiscard]] static std::expected<std::unique_ptr<CompressionStream>, StarException> create();
  ~CompressionStream();
  CompressionStream(CompressionStream const&) = delete;
  CompressionStream& operator=(CompressionStream const&) = delete;
  CompressionStream(CompressionStream&&) = delete;
  CompressionStream& operator=(CompressionStream&&) = delete;

  [[nodiscard]] std::expected<void, StarException> compress(const char* in, std::size_t inLen, ByteArray& out);
  [[nodiscard]] std::expected<void, StarException> compress(ByteArray const& in, ByteArray& out);
  [[nodiscard]] std::expected<ByteArray, StarException> compress(const char* in, std::size_t inLen);
  [[nodiscard]] std::expected<ByteArray, StarException> compress(ByteArray const& in);

private:
  CompressionStream() = default;
  struct StreamDeleter {
    void operator()(ZSTD_CStream* stream) const noexcept;
  };
  std::unique_ptr<ZSTD_CStream, StreamDeleter> m_cStream;
};

class DecompressionStream {
public:
  [[nodiscard]] static std::expected<std::unique_ptr<DecompressionStream>, StarException> create();
  ~DecompressionStream();
  DecompressionStream(DecompressionStream const&) = delete;
  DecompressionStream& operator=(DecompressionStream const&) = delete;
  DecompressionStream(DecompressionStream&&) = delete;
  DecompressionStream& operator=(DecompressionStream&&) = delete;

  [[nodiscard]] std::expected<void, StarException> decompress(const char* in, std::size_t inLen, ByteArray& out);
  [[nodiscard]] std::expected<void, StarException> decompress(ByteArray const& in, ByteArray& out);
  [[nodiscard]] std::expected<ByteArray, StarException> decompress(const char* in, std::size_t inLen);
  [[nodiscard]] std::expected<ByteArray, StarException> decompress(ByteArray const& in);

private:
  DecompressionStream() = default;
  struct StreamDeleter {
    void operator()(ZSTD_DStream* stream) const noexcept;
  };
  std::unique_ptr<ZSTD_DStream, StreamDeleter> m_dStream;
};

class ZstdCompression {
public:
  [[nodiscard]] static std::expected<void, StarException> compress(const char* in, std::size_t inLen, ByteArray& out, int compressionLevel = 0);
  [[nodiscard]] static std::expected<void, StarException> compress(ByteArray const& in, ByteArray& out, int compressionLevel = 0);
  [[nodiscard]] static std::expected<ByteArray, StarException> compress(const char* in, std::size_t inLen, int compressionLevel = 0);
  [[nodiscard]] static std::expected<ByteArray, StarException> compress(ByteArray const& in, int compressionLevel = 0);

  [[nodiscard]] static std::expected<void, StarException> decompress(const char* in, std::size_t inLen, ByteArray& out);
  [[nodiscard]] static std::expected<void, StarException> decompress(ByteArray const& in, ByteArray& out);
  [[nodiscard]] static std::expected<ByteArray, StarException> decompress(const char* in, std::size_t inLen);
  [[nodiscard]] static std::expected<ByteArray, StarException> decompress(ByteArray const& in);
};

}

export module star.zstd_compression;

export namespace Star {
  using ::Star::CompressionStream;
  using ::Star::DecompressionStream;
  using ::Star::ZstdCompression;
}
