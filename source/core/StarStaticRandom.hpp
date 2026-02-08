#pragma once

#include "StarString.hpp"

import std;

namespace Star {

constexpr std::uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ull;
constexpr std::uint64_t FNV_PRIME = 0x100000001b3ull;

inline void fnvPush(std::uint64_t& hash, const void* data, std::size_t size) {
  const auto* bytes = static_cast<const std::uint8_t*>(data);
  for (std::size_t i = 0; i < size; ++i) {
    hash ^= bytes[i];
    hash *= FNV_PRIME;
  }
}

template <typename T>
void staticPushValue(std::uint64_t& hash, T const& v) {
  if constexpr (std::is_same_v<T, String>) {
    fnvPush(hash, v.utf8Ptr(), v.utf8Size());
  } else if constexpr (std::is_enum_v<T>) {
    auto val = static_cast<std::underlying_type_t<T>>(v);
    fnvPush(hash, &val, sizeof(val));
  } else {
    fnvPush(hash, &v, sizeof(v));
  }
}

inline void staticRandomIter(std::uint64_t&) {}

template <typename T, typename... TL>
void staticRandomIter(std::uint64_t& hash, T const& v, TL const&... rest) {
  staticPushValue(hash, v);
  staticRandomIter(hash, rest...);
}

template <typename T, typename... TL>
auto staticRandomU64(T const& v, TL const&... rest) -> std::uint64_t {
  std::uint64_t hash = 1997293021376312589ull;
  staticRandomIter(hash, v, rest...);
  return hash;
}

template <typename T, typename... TL>
auto staticRandomU32(T const& v, TL const&... rest) -> std::uint32_t {
  std::uint64_t h = staticRandomU64(v, rest...);
  return (std::uint32_t)(h ^ (h >> 32));
}

template <typename T, typename... TL>
auto staticRandomI32(T const& d, TL const&... rest) -> std::int32_t {
  return (std::int32_t)staticRandomU32(d, rest...);
}

template <typename T, typename... TL>
auto staticRandomI32Range(std::int32_t min, std::int32_t max, T const& d, TL const&... rest) -> std::int32_t {
  std::uint64_t denom = (std::uint64_t)(-1) / ((std::uint64_t)(max - min) + 1);
  return (std::int32_t)(staticRandomU64(d, rest...) / denom + min);
}

template <typename T, typename... TL>
auto staticRandomU32Range(std::uint32_t min, std::uint32_t max, T const& d, TL const&... rest) -> std::uint32_t {
  std::uint64_t denom = (std::uint64_t)(-1) / ((std::uint64_t)(max - min) + 1);
  return staticRandomU64(d, rest...) / denom + min;
}

template <typename T, typename... TL>
auto staticRandomI64(T const& d, TL const&... rest) -> std::int64_t {
  return (std::int64_t)staticRandomU64(d, rest...);
}

// Generates values in the range [0.0, 1.0]
template <typename T, typename... TL>
auto staticRandomFloat(T const& d, TL const&... rest) -> float {
  return (staticRandomU32(d, rest...) & 0x7fffffff) / 2147483648.0;
}

template <typename T, typename... TL>
auto staticRandomFloatRange(float min, float max, T const& d, TL const&... rest) -> float {
  return staticRandomFloat(d, rest...) * (max - min) + min;
}

// Generates values in the range [0.0, 1.0]
template <typename T, typename... TL>
auto staticRandomDouble(T const& d, TL const&... rest) -> double {
  return (staticRandomU64(d, rest...) & 0x7fffffffffffffff) / 9223372036854775808.0;
}

template <typename T, typename... TL>
auto staticRandomDoubleRange(double min, double max, T const& d, TL const&... rest) -> double {
  return staticRandomDouble(d, rest...) * (max - min) + min;
}

template <typename Container, typename T, typename... TL>
auto staticRandomFrom(Container& container, T const& d, TL const&... rest) -> typename Container::value_type& {
  auto i = container.begin();
  std::advance(i, staticRandomI32Range(0, container.size() - 1, d, rest...));
  return *i;
}

template <typename Container, typename T, typename... TL>
auto staticRandomFrom(Container const& container, T const& d, TL const&... rest) -> typename Container::value_type const& {
  auto i = container.begin();
  std::advance(i, staticRandomI32Range(0, container.size() - 1, d, rest...));
  return *i;
}

template <typename Container, typename T, typename... TL>
auto staticRandomValueFrom(Container const& container, T const& d, TL const&... rest) -> typename Container::value_type {
  if (container.empty()) {
    return {};
  } else {
    auto i = container.begin();
    std::advance(i, staticRandomI32Range(0, container.size() - 1, d, rest...));
    return *i;
  }
}

template <typename T>
class URBG {
public:
  using Function = std::function<T()>;

  URBG(Function func) : m_func(func) {};

  using result_type = T;
  static constexpr auto min() -> T { return std::numeric_limits<T>::min(); };
  static constexpr auto max() -> T { return std::numeric_limits<T>::max(); };
  auto operator()() -> T { return m_func(); };

private:
  Function m_func;
};

template <typename Container, typename T, typename... TL>
void staticRandomShuffle(Container& container, T const& d, TL const&... rest) {
  auto begin = container.begin();
  auto end = container.end();
  auto it = begin;
  for (int i = 1, mix = 0; ++it != end; ++i) {
    int off = staticRandomU32Range(0, i, ++mix, d, rest...);
    if (off != i)
      std::swap(*it, *(begin + off));
  }
}

}// namespace Star
