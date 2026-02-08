#pragma once

#include "StarException.hpp"
#include "StarFormat.hpp"
#include "StarHash.hpp"

import std;

namespace Star {
// Class to hold an array of bytes.  Contains an internal buffer that may be
// larger than what is reported by size(), to avoid repeated allocations when a
// ByteArray grows.
class ByteArray {
public:
  using value_type = char;
  using iterator = char*;
  using const_iterator = char const*;

  // Constructs a byte array from a given c string WITHOUT including the
  // trailing '\0'
  static auto fromCString(char const* str) -> ByteArray;
  // Same, but includes the trailing '\0'
  static auto fromCStringWithNull(char const* str) -> ByteArray;
  static auto withReserve(std::size_t capacity) -> ByteArray;

  ByteArray();
  ByteArray(std::size_t dataSize, char c);
  ByteArray(char const* data, std::size_t dataSize);
  ByteArray(ByteArray const& b);
  ByteArray(ByteArray&& b) noexcept;
  ~ByteArray();

  auto operator=(ByteArray const& b) -> ByteArray&;
  auto operator=(ByteArray&& b) noexcept -> ByteArray&;

  [[nodiscard]] auto ptr() const -> char const*;
  auto ptr() -> char*;

  [[nodiscard]] auto size() const -> std::size_t;
  // Maximum size before realloc
  [[nodiscard]] auto capacity() const -> std::size_t;
  // Is zero size
  [[nodiscard]] auto empty() const -> bool;

  // Sets size to 0.
  void clear();
  // Clears and resets buffer to empty.
  void reset();

  void reserve(std::size_t capacity);

  void resize(std::size_t size);
  // resize, filling new space with given byte if it exists.
  void resize(std::size_t size, char f);

  // fill array with byte.
  void fill(char c);
  // fill array and resize to new size.
  void fill(std::size_t size, char c);

  void append(ByteArray const& b);
  void append(char const* data, std::size_t len);
  void appendByte(char b);

  void copyTo(char* data, std::size_t len) const;
  void copyTo(char* data) const;

  // Copy from ByteArray starting at pos, to data, with size len.
  void copyTo(char* data, std::size_t pos, std::size_t len) const;
  // Copy from data pointer to ByteArray at pos with size len.
  // Resizes if needed.
  void writeFrom(char const* data, std::size_t pos, std::size_t len);

  [[nodiscard]] auto sub(std::size_t b, std::size_t s) const -> ByteArray;
  [[nodiscard]] auto left(std::size_t s) const -> ByteArray;
  [[nodiscard]] auto right(std::size_t s) const -> ByteArray;

  void trimLeft(std::size_t s);
  void trimRight(std::size_t s);

  // returns location of first character that is different than the given
  // ByteArray.
  [[nodiscard]] auto diffChar(ByteArray const& b) const -> std::size_t;
  // returns -1 if this < b, 0 if this == b, 1 if this > b
  [[nodiscard]] auto compare(ByteArray const& b) const -> int;

  template <typename Combiner>
  auto combineWith(Combiner&& combine, ByteArray const& rhs, bool extend = false) -> ByteArray;

  auto andWith(ByteArray const& rhs, bool extend = false) -> ByteArray;
  auto orWith(ByteArray const& rhs, bool extend = false) -> ByteArray;
  auto xorWith(ByteArray const& rhs, bool extend = false) -> ByteArray;

  auto begin() -> iterator;
  auto end() -> iterator;

  [[nodiscard]] auto begin() const -> const_iterator;
  [[nodiscard]] auto end() const -> const_iterator;

  void insert(std::size_t pos, char byte);
  auto insert(const_iterator pos, char byte) -> iterator;
  void push_back(char byte);

  auto operator[](std::size_t i) -> char&;
  auto operator[](std::size_t i) const -> char;
  [[nodiscard]] auto at(std::size_t i) const -> char;

  auto operator<(ByteArray const& b) const -> bool;
  auto operator==(ByteArray const& b) const -> bool;
  auto operator!=(ByteArray const& b) const -> bool;

private:
  char* m_data;
  std::size_t m_capacity;
  std::size_t m_size;
};

template <>
struct hash<ByteArray> {
  auto operator()(ByteArray const& b) const -> std::size_t;
};

auto operator<<(std::ostream& os, ByteArray const& b) -> std::ostream&;

inline void ByteArray::clear() {
  resize(0);
}

inline void ByteArray::resize(std::size_t size) {
  reserve(size);
  m_size = size;
}

inline void ByteArray::append(ByteArray const& b) {
  append(b.ptr(), b.size());
}

inline void ByteArray::append(const char* data, std::size_t len) {
  resize(m_size + len);
  std::memcpy(m_data + m_size - len, data, len);
}

inline void ByteArray::appendByte(char b) {
  resize(m_size + 1);
  m_data[m_size - 1] = b;
}

inline auto ByteArray::empty() const -> bool {
  return m_size == 0;
}

inline auto ByteArray::ptr() const -> char const* {
  return m_data;
}

inline auto ByteArray::ptr() -> char* {
  return m_data;
}

inline auto ByteArray::size() const -> std::size_t {
  return m_size;
}

inline auto ByteArray::capacity() const -> std::size_t {
  return m_capacity;
}

inline void ByteArray::copyTo(char* data, std::size_t len) const {
  len = std::min(m_size, len);
  std::memcpy(data, m_data, len);
}

inline void ByteArray::copyTo(char* data) const {
  copyTo(data, m_size);
}

inline void ByteArray::copyTo(char* data, std::size_t pos, std::size_t len) const {
  if (len == 0 || pos >= m_size)
    return;

  len = std::min(m_size - pos, len);
  std::memcpy(data, m_data + pos, len);
}

inline void ByteArray::writeFrom(const char* data, std::size_t pos, std::size_t len) {
  if (pos + len > m_size)
    resize(pos + len);

  std::memcpy(m_data + pos, data, len);
}

template <typename Combiner>
auto ByteArray::combineWith(Combiner&& combine, ByteArray const& rhs, bool extend) -> ByteArray {
  ByteArray const* smallerArray = &rhs;
  ByteArray const* largerArray = this;

  if (m_size < rhs.size())
    std::swap(smallerArray, largerArray);

  ByteArray res;
  res.resize(smallerArray->size());

  for (std::size_t i = 0; i < smallerArray->size(); ++i)
    res[i] = combine((*smallerArray)[i], (*largerArray)[i]);

  if (extend) {
    res.resize(largerArray->size());
    for (std::size_t i = smallerArray->size(); i < largerArray->size(); ++i)
      res[i] = (*largerArray)[i];
  }

  return res;
}

inline auto ByteArray::begin() -> ByteArray::iterator {
  return m_data;
}

inline auto ByteArray::end() -> ByteArray::iterator {
  return m_data + m_size;
}

inline auto ByteArray::begin() const -> ByteArray::const_iterator {
  return m_data;
}

inline auto ByteArray::end() const -> ByteArray::const_iterator {
  return m_data + m_size;
}

inline auto ByteArray::operator[](std::size_t i) -> char& {
  return m_data[i];
}

inline auto ByteArray::operator[](std::size_t i) const -> char {
  return m_data[i];
}

inline auto ByteArray::at(std::size_t i) const -> char {
  if (i >= m_size)
    throw OutOfRangeException(strf("Out of range in ByteArray::at({})", i));

  return m_data[i];
}

inline auto hash<ByteArray>::operator()(ByteArray const& b) const -> std::size_t {
  PLHasher hash;
  for (char i : b)
    hash.put(i);
  return hash.hash();
}

}// namespace Star

template <>
struct std::formatter<Star::ByteArray> : Star::ostream_formatter {};
