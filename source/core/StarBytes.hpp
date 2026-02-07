#pragma once

import std;

namespace Star {

enum class ByteOrder {
  BigEndian,
  LittleEndian,
  NoConversion
};

auto platformByteOrder() -> ByteOrder;

void swapByteOrder(void* ptr, std::size_t len);
void swapByteOrder(void* dest, void const* src, std::size_t len);

void toByteOrder(ByteOrder order, void* ptr, std::size_t len);
void toByteOrder(ByteOrder order, void* dest, void const* src, std::size_t len);
void fromByteOrder(ByteOrder order, void* ptr, std::size_t len);
void fromByteOrder(ByteOrder order, void* dest, void const* src, std::size_t len);

template <typename T>
auto toByteOrder(ByteOrder order, T const& t) -> T {
  T ret;
  toByteOrder(order, &ret, &t, sizeof(t));
  return ret;
}

template <typename T>
auto fromByteOrder(ByteOrder order, T const& t) -> T {
  T ret;
  fromByteOrder(order, &ret, &t, sizeof(t));
  return ret;
}

template <typename T>
auto toBigEndian(T const& t) -> T {
  return toByteOrder(ByteOrder::BigEndian, t);
}

template <typename T>
auto fromBigEndian(T const& t) -> T {
  return fromByteOrder(ByteOrder::BigEndian, t);
}

template <typename T>
auto toLittleEndian(T const& t) -> T {
  return toByteOrder(ByteOrder::LittleEndian, t);
}

template <typename T>
auto fromLittleEndian(T const& t) -> T {
  return fromByteOrder(ByteOrder::LittleEndian, t);
}

inline auto platformByteOrder() -> ByteOrder {
#if STAR_LITTLE_ENDIAN
  return ByteOrder::LittleEndian;
#else
  return ByteOrder::BigEndian;
#endif
}

inline void swapByteOrder(void* ptr, std::size_t len) {
  auto* data = static_cast<std::uint8_t*>(ptr);
  std::uint8_t spare;
  for (std::size_t i = 0; i < len / 2; ++i) {
    spare = data[len - 1 - i];
    data[len - 1 - i] = data[i];
    data[i] = spare;
  }
}

inline void swapByteOrder(void* dest, const void* src, std::size_t len) {
  const auto* srcdata = reinterpret_cast<const std::uint8_t*>(src);
  auto* destdata = reinterpret_cast<std::uint8_t*>(dest);
  for (std::size_t i = 0; i < len; ++i)
    destdata[len - 1 - i] = srcdata[i];
}

inline void toByteOrder(ByteOrder order, void* ptr, std::size_t len) {
  if (order != ByteOrder::NoConversion && platformByteOrder() != order)
    swapByteOrder(ptr, len);
}

inline void toByteOrder(ByteOrder order, void* dest, void const* src, std::size_t len) {
  if (order != ByteOrder::NoConversion && platformByteOrder() != order)
    swapByteOrder(dest, src, len);
  else
    std::memcpy(dest, src, len);
}

inline void fromByteOrder(ByteOrder order, void* ptr, std::size_t len) {
  if (order != ByteOrder::NoConversion && platformByteOrder() != order)
    swapByteOrder(ptr, len);
}

inline void fromByteOrder(ByteOrder order, void* dest, void const* src, std::size_t len) {
  if (order != ByteOrder::NoConversion && platformByteOrder() != order)
    swapByteOrder(dest, src, len);
  else
    std::memcpy(dest, src, len);
}

}// namespace Star
