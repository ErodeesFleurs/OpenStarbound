#include "StarZSTDCompression.hpp"
#include <zstd.h>

namespace Star {

void ZstdCompressionStreamDeleter::operator()(ZSTD_CStream* stream) const noexcept {
  ZSTD_freeCStream(stream);
}

void ZstdDecompressionStreamDeleter::operator()(ZSTD_DStream* stream) const noexcept {
  ZSTD_freeDStream(stream);
}

namespace {

void checkZstd(size_t result, char const* operation) {
  if (ZSTD_isError(result))
    throw IOException(strf("ZSTD {} error {}", operation, ZSTD_getErrorName(result)));
}

}// namespace

CompressionStream::CompressionStream() : m_cStream(ZSTD_createCStream()) {
  if (!m_cStream)
    throw IOException("Could not create ZSTD compression stream");

  checkZstd(ZSTD_CCtx_setParameter(m_cStream.get(), ZSTD_c_enableLongDistanceMatching, 1), "compression parameter");
  checkZstd(ZSTD_CCtx_setParameter(m_cStream.get(), ZSTD_c_windowLog, 24), "compression parameter");
  checkZstd(ZSTD_initCStream(m_cStream.get(), 2), "compression initialization");
}

CompressionStream::~CompressionStream() = default;

void CompressionStream::compress(const char* in, size_t inLen, ByteArray& out) {
  size_t const cOutSize = ZSTD_CStreamOutSize();
  ZSTD_inBuffer inBuffer = {in, inLen, 0};
  size_t written = out.size();
  out.resize(out.size() + cOutSize);
  bool finished = false;
  do {
    ZSTD_outBuffer outBuffer = {out.ptr() + written, min(cOutSize, out.size() - written), 0};
    size_t ret = ZSTD_compressStream2(m_cStream.get(), &outBuffer, &inBuffer, ZSTD_e_flush);
    if (ZSTD_isError(ret)) {
      throw IOException(strf("ZSTD compression error {}", ZSTD_getErrorName(ret)));
    }

    written += outBuffer.pos;
    if (outBuffer.pos == outBuffer.size) {
      if (written >= out.size())
        out.resize(out.size() * 2);
      continue;
    }

    finished = ret == 0 && inBuffer.pos == inBuffer.size;
  } while (!finished);
  out.resize(written);
}

void CompressionStream::compress(ByteArray const& in, ByteArray& out) {
  return compress(in.ptr(), in.size(), out);
}

[[nodiscard]] ByteArray CompressionStream::compress(const char* in, size_t inLen) {
  ByteArray out;
  compress(in, inLen, out);
  return out;
}

[[nodiscard]] ByteArray CompressionStream::compress(ByteArray const& in) {
  ByteArray out;
  compress(in.ptr(), in.size(), out);
  return out;
}

DecompressionStream::DecompressionStream() : m_dStream(ZSTD_createDStream()) {
  if (!m_dStream)
    throw IOException("Could not create ZSTD decompression stream");

  checkZstd(ZSTD_DCtx_setParameter(m_dStream.get(), ZSTD_d_windowLogMax, 25), "decompression parameter");
  checkZstd(ZSTD_initDStream(m_dStream.get()), "decompression initialization");
}

DecompressionStream::~DecompressionStream() = default;

void DecompressionStream::decompress(const char* in, size_t inLen, ByteArray& out) {
  size_t const dOutSize = ZSTD_DStreamOutSize();
  ZSTD_inBuffer inBuffer = {in, inLen, 0};
  size_t written = out.size();
  out.resize(out.size() + dOutSize);
  bool finished = false;
  do {
    ZSTD_outBuffer outBuffer = {out.ptr() + written, min(dOutSize, out.size() - written), 0};
    size_t ret = ZSTD_decompressStream(m_dStream.get(), &outBuffer, &inBuffer);
    if (ZSTD_isError(ret)) {
      throw IOException(strf("ZSTD decompression error {}", ZSTD_getErrorName(ret)));
    }

    written += outBuffer.pos;
    if (outBuffer.pos == outBuffer.size) {
      if (written >= out.size())
        out.resize(out.size() * 2);
      continue;
    }
    finished = inBuffer.pos == inBuffer.size;
  } while (!finished);
  out.resize(written);
}

void DecompressionStream::decompress(ByteArray const& in, ByteArray& out) {
  return decompress(in.ptr(), in.size(), out);
}

[[nodiscard]] ByteArray DecompressionStream::decompress(const char* in, size_t inLen) {
  ByteArray out;
  decompress(in, inLen, out);
  return out;
}

[[nodiscard]] ByteArray DecompressionStream::decompress(ByteArray const& in) {
  ByteArray out;
  decompress(in.ptr(), in.size(), out);
  return out;
}

}// namespace Star
