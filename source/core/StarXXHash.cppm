module;
#define XXH_INLINE_ALL
#include "xxhash.h"
#include "xxh3.h"

import std;
import star.string;
import star.byte_array;
import star.format;


import star.bytes;
namespace Star {

class XXHash32 {
public:
  XXHash32(std::uint32_t seed = 0);

  void push(std::span<char const> data);
  std::uint32_t digest();

private:
  XXH32_state_s state;
};

class XXHash64 {
public:
  XXHash64(std::uint64_t seed = 0);

  void push(std::span<char const> data);
  std::uint64_t digest();

private:
  XXH64_state_s state;
};

class XXHash3 {
public:
  XXHash3();

  void push(std::span<char const> data);
  std::uint64_t digest();

private:
  XXH3_state_s state;
};


std::uint32_t xxHash32(std::span<char const> source);
std::uint32_t xxHash32(ByteArray const& in);
std::uint32_t xxHash32(String const& in);

std::uint64_t xxHash64(std::span<char const> source);
std::uint64_t xxHash64(ByteArray const& in);
std::uint64_t xxHash64(String const& in);

std::uint64_t xxHash3(std::span<char const> source);
std::uint64_t xxHash3(ByteArray const& in);
std::uint64_t xxHash3(String const& in);

template <typename Representation, typename Hasher, typename Value>
static inline void xxHashPushPrimitive(Hasher& hash, Value const& v) {
  Representation cv = v;
  cv = toLittleEndian(cv);
  hash.push({reinterpret_cast<char const*>(&cv), sizeof(cv)});
}

template <typename Value>
  requires std::same_as<Value, bool>
inline void xxHash32Push(XXHash32& hash, Value const& v) {
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

template <typename Value>
  requires std::same_as<Value, bool>
inline void xxHash64Push(XXHash64& hash, Value const& v) {
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

template <typename Value>
  requires std::same_as<Value, bool>
inline void xxHash3Push(XXHash3& hash, Value const& v) {
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

inline void xxHash32Push(XXHash32& hash, std::string_view str) {
  hash.push(std::span<char const>(str));
}

template <std::size_t N>
inline void xxHash32Push(XXHash32& hash, char const (&text)[N]) {
  std::string_view view(text, N);
  xxHash32Push(hash, view.substr(0, view.find('\0')));
}

inline void xxHash32Push(XXHash32& hash, std::string const& text) {
  xxHash32Push(hash, std::string_view(text));
}

inline void xxHash32Push(XXHash32& hash, String const& str) {
  // String::size() is a code point count, the bytes hashed are utf8 bytes.
  hash.push(std::span<char const>(str.utf8()));
}

inline void xxHash64Push(XXHash64& hash, std::string_view str) {
  hash.push(std::span<char const>(str));
}

template <std::size_t N>
inline void xxHash64Push(XXHash64& hash, char const (&text)[N]) {
  std::string_view view(text, N);
  xxHash64Push(hash, view.substr(0, view.find('\0')));
}

inline void xxHash64Push(XXHash64& hash, std::string const& text) {
  xxHash64Push(hash, std::string_view(text));
}

inline void xxHash64Push(XXHash64& hash, String const& str) {
  // String::size() is a code point count, the bytes hashed are utf8 bytes.
  hash.push(std::span<char const>(str.utf8()));
}

inline void xxHash3Push(XXHash3& hash, std::string_view str) {
  hash.push(std::span<char const>(str));
}

template <std::size_t N>
inline void xxHash3Push(XXHash3& hash, char const (&text)[N]) {
  std::string_view view(text, N);
  xxHash3Push(hash, view.substr(0, view.find('\0')));
}

[[nodiscard]] inline std::expected<void, StarException> xxHash3Push(XXHash3& hash, String const& str) {
  auto size = str.size();
  if (!size)
    return std::unexpected(std::move(size.error()));
  hash.push(std::span<char const>(str.utf8()).first(*size));
  return {};
}

inline XXHash32::XXHash32(std::uint32_t seed) {
  XXH32_reset(&state, seed);
}

inline void XXHash32::push(std::span<char const> data) {
  XXH32_update(&state, data.data(), data.size());
}

inline std::uint32_t XXHash32::digest() {
  return XXH32_digest(&state);
}

inline XXHash64::XXHash64(std::uint64_t seed) {
  XXH64_reset(&state, seed);
}

inline void XXHash64::push(std::span<char const> data) {
  XXH64_update(&state, data.data(), data.size());
}

inline std::uint64_t XXHash64::digest() {
  return XXH64_digest(&state);
}

inline XXHash3::XXHash3() {
  XXH3_64bits_reset(&state);
}

inline void XXHash3::push(std::span<char const> data) {
  XXH3_64bits_update(&state, data.data(), data.size());
}

inline std::uint64_t XXHash3::digest() {
  return XXH3_64bits_digest(&state);
}

inline std::uint32_t xxHash32(std::span<char const> source) {
  return XXH32(source.data(), source.size(), 0);
}

inline std::uint32_t xxHash32(ByteArray const& in) {
  return xxHash32(in.span());
}

inline std::uint32_t xxHash32(String const& in) {
  return xxHash32(std::span<char const>(in.utf8()));
}

inline std::uint64_t xxHash64(std::span<char const> source) {
  return XXH64(source.data(), source.size(), 0);
}

inline std::uint64_t xxHash64(ByteArray const& in) {
  return xxHash64(in.span());
}

inline std::uint64_t xxHash64(String const& in) {
  return xxHash64(std::span<char const>(in.utf8()));
}

inline std::uint64_t xxHash3(std::span<char const> source) {
  return XXH3_64bits(source.data(), source.size());
}

inline auto xxHash3_128(std::span<char const> source) {
  return XXH3_128bits(source.data(), source.size());
}

inline std::uint64_t xxHash3(ByteArray const& in) {
  return xxHash3(in.span());
}

inline std::uint64_t xxHash3(String const& in) {
  return xxHash3(std::span<char const>(in.utf8()));
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
