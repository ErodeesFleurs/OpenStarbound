#pragma once

#include "StarString.hpp"
#include "StarByteArray.hpp"

namespace Star {

[[nodiscard]] size_t hexEncode(char const* data, size_t len, char* output, size_t outLen = NPos);
[[nodiscard]] size_t hexDecode(char const* src, size_t len, char* output, size_t outLen = NPos);
[[nodiscard]] size_t nibbleDecode(char const* src, size_t len, char* output, size_t outLen = NPos);

[[nodiscard]] size_t base64Encode(char const* data, size_t len, char* output, size_t outLen = NPos);
[[nodiscard]] size_t base64Decode(char const* src, size_t len, char* output, size_t outLen = NPos);

[[nodiscard]] String hexEncode(char const* data, size_t len);
[[nodiscard]] String base64Encode(char const* data, size_t len);

[[nodiscard]] String hexEncode(ByteArray const& data);
[[nodiscard]] ByteArray hexDecode(String const& encodedData);

[[nodiscard]] String base64Encode(ByteArray const& data);
[[nodiscard]] ByteArray base64Decode(String const& encodedData);

}
