#pragma once

#include "StarString.hpp"
#include "StarByteArray.hpp"

#define XXH_STATIC_LINKING_ONLY
#define XXH_INLINE_ALL
#include "xxhash.h"
#include "xxh3.h"

#include <type_traits>

namespace Star {

class XXHash32 {
public:
  XXHash32(uint32_t seed = 0);

  void push(char const* data, size_t length);
  uint32_t digest();

private:
  XXH32_state_s state;
};

class XXHash64 {
public:
  XXHash64(uint64_t seed = 0);

  void push(char const* data, size_t length);
  uint64_t digest();

private:
  XXH64_state_s state;
};

class XXHash3 {
public:
  XXHash3();

  void push(char const* data, size_t length);
  uint64_t digest();

private:
  XXH3_state_s state;
};


uint32_t xxHash32(char const* source, size_t length);
uint32_t xxHash32(ByteArray const& in);
uint32_t xxHash32(String const& in);

uint64_t xxHash64(char const* source, size_t length);
uint64_t xxHash64(ByteArray const& in);
uint64_t xxHash64(String const& in);

uint64_t xxHash3(char const* source, size_t length);
uint64_t xxHash3(ByteArray const& in);
uint64_t xxHash3(String const& in);

namespace Detail {
  template <typename T>
  struct XXHashPrimitiveType;

  template <> struct XXHashPrimitiveType<bool> { using Type = bool; };
  template <> struct XXHashPrimitiveType<int> { using Type = int32_t; };
  template <> struct XXHashPrimitiveType<long> { using Type = int64_t; };
  template <> struct XXHashPrimitiveType<long long> { using Type = int64_t; };
  template <> struct XXHashPrimitiveType<unsigned int> { using Type = uint32_t; };
  template <> struct XXHashPrimitiveType<unsigned long> { using Type = uint64_t; };
  template <> struct XXHashPrimitiveType<unsigned long long> { using Type = uint64_t; };
  template <> struct XXHashPrimitiveType<unsigned short> { using Type = uint16_t; };
  template <> struct XXHashPrimitiveType<float> { using Type = float; };
  template <> struct XXHashPrimitiveType<double> { using Type = double; };

  template <typename T>
  concept XXHashPrimitive = requires {
    typename XXHashPrimitiveType<std::remove_cvref_t<T>>::Type;
  };

  template <typename Hash, XXHashPrimitive T>
  void xxHashPushPrimitive(Hash& hash, T const& v) {
    using CastType = typename XXHashPrimitiveType<std::remove_cvref_t<T>>::Type;
    auto cv = static_cast<CastType>(v);
    cv = toLittleEndian(cv);
    hash.push(reinterpret_cast<char const*>(&cv), sizeof(cv));
  }
}

template <Detail::XXHashPrimitive T>
inline void xxHash32Push(XXHash32& hash, T const& v) {
  Detail::xxHashPushPrimitive(hash, v);
}

template <Detail::XXHashPrimitive T>
inline void xxHash64Push(XXHash64& hash, T const& v) {
  Detail::xxHashPushPrimitive(hash, v);
}

template <Detail::XXHashPrimitive T>
inline void xxHash3Push(XXHash3& hash, T const& v) {
  Detail::xxHashPushPrimitive(hash, v);
}

inline void xxHash32Push(XXHash32& hash, char const* str) {
  hash.push(str, strlen(str));
}

inline void xxHash32Push(XXHash32& hash, String const& str) {
  hash.push(str.utf8Ptr(), str.size());
}

inline void xxHash64Push(XXHash64& hash, char const* str) {
  hash.push(str, strlen(str));
}

inline void xxHash64Push(XXHash64& hash, String const& str) {
  hash.push(str.utf8Ptr(), str.size());
}

inline void xxHash3Push(XXHash3& hash, char const* str) {
  hash.push(str, strlen(str));
}

inline void xxHash3Push(XXHash3& hash, String const& str) {
  hash.push(str.utf8Ptr(), str.size());
}

inline XXHash32::XXHash32(uint32_t seed) {
  XXH32_reset(&state, seed);
}

inline void XXHash32::push(char const* data, size_t length) {
  XXH32_update(&state, data, length);
}

inline uint32_t XXHash32::digest() {
  return XXH32_digest(&state);
}

inline XXHash64::XXHash64(uint64_t seed) {
  XXH64_reset(&state, seed);
}

inline void XXHash64::push(char const* data, size_t length) {
  XXH64_update(&state, data, length);
}

inline uint64_t XXHash64::digest() {
  return XXH64_digest(&state);
}

inline XXHash3::XXHash3() {
  XXH3_64bits_reset(&state);
}

inline void XXHash3::push(char const* data, size_t length) {
  XXH3_64bits_update(&state, data, length);
}

inline uint64_t XXHash3::digest() {
  return XXH3_64bits_digest(&state);
}

inline uint32_t xxHash32(char const* source, size_t length) {
  return XXH32(source, length, 0);
}

inline uint32_t xxHash32(ByteArray const& in) {
  return xxHash32(in.ptr(), in.size());
}

inline uint32_t xxHash32(String const& in) {
  return xxHash32(in.utf8Ptr(), in.utf8Size());
}

inline uint64_t xxHash64(char const* source, size_t length) {
  return XXH64(source, length, 0);
}

inline uint64_t xxHash64(ByteArray const& in) {
  return xxHash64(in.ptr(), in.size());
}

inline uint64_t xxHash64(String const& in) {
  return xxHash64(in.utf8Ptr(), in.utf8Size());
}

inline uint64_t xxHash3(char const* source, size_t length) {
  return XXH3_64bits(source, length);
}

inline uint64_t xxHash3(ByteArray const& in) {
  return xxHash3(in.ptr(), in.size());
}

inline uint64_t xxHash3(String const& in) {
  return xxHash3(in.utf8Ptr(), in.utf8Size());
}
}
