module;
#define XXH_INLINE_ALL
#include "xxhash.h"
#include "xxh3.h"

import std;
import star.string;
import star.byte_array;


import star.bytes;
namespace Star {

class XXHash32 {
public:
  XXHash32(std::uint32_t seed = 0);

  void push(char const* data, std::size_t length);
  std::uint32_t digest();

private:
  XXH32_state_s state;
};

class XXHash64 {
public:
  XXHash64(std::uint64_t seed = 0);

  void push(char const* data, std::size_t length);
  std::uint64_t digest();

private:
  XXH64_state_s state;
};

class XXHash3 {
public:
  XXHash3();

  void push(char const* data, std::size_t length);
  std::uint64_t digest();

private:
  XXH3_state_s state;
};


std::uint32_t xxHash32(char const* source, std::size_t length);
std::uint32_t xxHash32(ByteArray const& in);
std::uint32_t xxHash32(String const& in);

std::uint64_t xxHash64(char const* source, std::size_t length);
std::uint64_t xxHash64(ByteArray const& in);
std::uint64_t xxHash64(String const& in);

std::uint64_t xxHash3(char const* source, std::size_t length);
std::uint64_t xxHash3(ByteArray const& in);
std::uint64_t xxHash3(String const& in);

template <typename Representation, typename Hasher, typename Value>
static inline void xxHashPushPrimitive(Hasher& hash, Value const& v) {
  Representation cv = v;
  cv = toLittleEndian(cv);
  hash.push(reinterpret_cast<char const*>(&cv), sizeof(cv));
}

inline void xxHash32Push(XXHash32& hash, bool const& v) {
  xxHashPushPrimitive<bool>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, int const& v) {
  xxHashPushPrimitive<std::int32_t>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, long const& v) {
  xxHashPushPrimitive<std::int64_t>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, long long const& v) {
  xxHashPushPrimitive<std::int64_t>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, unsigned int const& v) {
  xxHashPushPrimitive<std::uint32_t>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, unsigned long const& v) {
  xxHashPushPrimitive<std::uint64_t>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, unsigned long long const& v) {
  xxHashPushPrimitive<std::uint64_t>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, float const& v) {
  xxHashPushPrimitive<float>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, double const& v) {
  xxHashPushPrimitive<double>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, bool const& v) {
  xxHashPushPrimitive<bool>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, int const& v) {
  xxHashPushPrimitive<std::int32_t>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, long const& v) {
  xxHashPushPrimitive<std::int64_t>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, long long const& v) {
  xxHashPushPrimitive<std::int64_t>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, unsigned int const& v) {
  xxHashPushPrimitive<std::uint32_t>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, unsigned long const& v) {
  xxHashPushPrimitive<std::uint64_t>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, unsigned long long const& v) {
  xxHashPushPrimitive<std::uint64_t>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, float const& v) {
  xxHashPushPrimitive<float>(hash, v);
}

inline void xxHash64Push(XXHash64& hash, double const& v) {
  xxHashPushPrimitive<double>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, bool const& v) {
  xxHashPushPrimitive<bool>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, int const& v) {
  xxHashPushPrimitive<std::int32_t>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, long const& v) {
  xxHashPushPrimitive<std::int64_t>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, long long const& v) {
  xxHashPushPrimitive<std::int64_t>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, unsigned int const& v) {
  xxHashPushPrimitive<std::uint32_t>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, unsigned long const& v) {
  xxHashPushPrimitive<std::uint64_t>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, unsigned long long const& v) {
  xxHashPushPrimitive<std::uint64_t>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, float const& v) {
  xxHashPushPrimitive<float>(hash, v);
}

inline void xxHash3Push(XXHash3& hash, double const& v) {
  xxHashPushPrimitive<double>(hash, v);
}

inline void xxHash32Push(XXHash32& hash, char const* str) {
  hash.push(str, std::strlen(str));
}

inline void xxHash32Push(XXHash32& hash, String const& str) {
  // String::size() is a code point count, the bytes hashed are utf8 bytes.
  hash.push(str.utf8Ptr(), str.utf8Size());
}

inline void xxHash64Push(XXHash64& hash, char const* str) {
  hash.push(str, std::strlen(str));
}

inline void xxHash64Push(XXHash64& hash, String const& str) {
  // String::size() is a code point count, the bytes hashed are utf8 bytes.
  hash.push(str.utf8Ptr(), str.utf8Size());
}

inline void xxHash3Push(XXHash3& hash, char const* str) {
  hash.push(str, std::strlen(str));
}

inline void xxHash3Push(XXHash3& hash, String const& str) {
  hash.push(str.utf8Ptr(), str.size());
}

inline XXHash32::XXHash32(std::uint32_t seed) {
  XXH32_reset(&state, seed);
}

inline void XXHash32::push(char const* data, std::size_t length) {
  XXH32_update(&state, data, length);
}

inline std::uint32_t XXHash32::digest() {
  return XXH32_digest(&state);
}

inline XXHash64::XXHash64(std::uint64_t seed) {
  XXH64_reset(&state, seed);
}

inline void XXHash64::push(char const* data, std::size_t length) {
  XXH64_update(&state, data, length);
}

inline std::uint64_t XXHash64::digest() {
  return XXH64_digest(&state);
}

inline XXHash3::XXHash3() {
  XXH3_64bits_reset(&state);
}

inline void XXHash3::push(char const* data, std::size_t length) {
  XXH3_64bits_update(&state, data, length);
}

inline std::uint64_t XXHash3::digest() {
  return XXH3_64bits_digest(&state);
}

inline std::uint32_t xxHash32(char const* source, std::size_t length) {
  return XXH32(source, length, 0);
}

inline std::uint32_t xxHash32(ByteArray const& in) {
  return xxHash32(in.ptr(), in.size());
}

inline std::uint32_t xxHash32(String const& in) {
  return xxHash32(in.utf8Ptr(), in.utf8Size());
}

inline std::uint64_t xxHash64(char const* source, std::size_t length) {
  return XXH64(source, length, 0);
}

inline std::uint64_t xxHash64(ByteArray const& in) {
  return xxHash64(in.ptr(), in.size());
}

inline std::uint64_t xxHash64(String const& in) {
  return xxHash64(in.utf8Ptr(), in.utf8Size());
}

inline std::uint64_t xxHash3(char const* source, std::size_t length) {
  return XXH3_64bits(source, length);
}

inline auto xxHash3_128(char const* source, std::size_t length) {
  return XXH3_128bits(source, length);
}

inline std::uint64_t xxHash3(ByteArray const& in) {
  return xxHash3(in.ptr(), in.size());
}

inline std::uint64_t xxHash3(String const& in) {
  return xxHash3(in.utf8Ptr(), in.utf8Size());
}
}

export module star.xx_hash;

export namespace Star {
  using ::Star::XXHash32;
  using ::Star::XXHash64;
  using ::Star::XXHash3;
  using ::Star::xxHash32;
  using ::Star::xxHash64;
  using ::Star::xxHash3;
  using ::Star::xxHash3_128;
  using ::Star::xxHash32Push;
  using ::Star::xxHash64Push;
  using ::Star::xxHash3Push;
}
