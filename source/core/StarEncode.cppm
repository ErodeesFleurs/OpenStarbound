module;
import std;
import star.memory;
import star.string;
import star.byte_array;

namespace Star {

std::size_t hexEncode(char const* data, std::size_t len, char* output, std::size_t outLen = NPos);
std::size_t hexDecode(char const* src, std::size_t len, char* output, std::size_t outLen = NPos);
std::size_t nibbleDecode(char const* src, std::size_t len, char* output, std::size_t outLen = NPos);

std::size_t base64Encode(char const* data, std::size_t len, char* output, std::size_t outLen = NPos);
std::size_t base64Decode(char const* src, std::size_t len, char* output, std::size_t outLen = NPos);

String hexEncode(char const* data, std::size_t len);
String base64Encode(char const* data, std::size_t len);

String hexEncode(ByteArray const& data);
ByteArray hexDecode(String const& encodedData);

String base64Encode(ByteArray const& data);
ByteArray base64Decode(String const& encodedData);

}

export module star.encode;

export namespace Star {
  using ::Star::hexEncode;
  using ::Star::hexDecode;
  using ::Star::nibbleDecode;
  using ::Star::base64Encode;
  using ::Star::base64Decode;
}
