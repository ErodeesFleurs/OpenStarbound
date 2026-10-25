module;
import std;
import star.string;
import star.format;
import star.byte_array;

namespace Star {

std::size_t hexEncode(std::span<char const> data, std::span<char> output);
[[nodiscard]] std::expected<std::size_t, StarException> hexDecode(std::span<char const> data, std::span<char> output);
[[nodiscard]] std::expected<std::size_t, StarException> nibbleDecode(std::span<char const> data, std::span<char> output);

std::size_t base64Encode(std::span<char const> data, std::span<char> output);
[[nodiscard]] std::expected<std::size_t, StarException> base64Decode(std::span<char const> data, std::span<char> output);

String hexEncode(std::span<char const> data);
String base64Encode(std::span<char const> data);

String hexEncode(ByteArray const& data);
[[nodiscard]] std::expected<ByteArray, StarException> hexDecode(String const& encodedData);

String base64Encode(ByteArray const& data);
[[nodiscard]] std::expected<ByteArray, StarException> base64Decode(String const& encodedData);

}

export module star.encode;

export namespace Star {
  using ::Star::hexEncode;
  using ::Star::hexDecode;
  using ::Star::nibbleDecode;
  using ::Star::base64Encode;
  using ::Star::base64Decode;
}
